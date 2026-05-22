package app.rive.rive_native

import android.os.Handler
import android.os.Looper
import android.view.Surface
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import io.flutter.view.TextureRegistry
import android.util.Log

external fun createRiveRenderer(
    surface: Surface,
    width: Int,
    height: Int,
): Long

external fun destroyRiveRenderer(renderer: Long)
external fun markDestroyedRiveRenderer(renderer: Long)

// Hands a RiveRenderTexture-like peer to native so the bg-thread render path
// can wake Flutter's compositor after each successful eglSwapBuffers. The
// peer's `scheduleFrame()` posts to the main thread Handler before calling
// `SurfaceProducer.scheduleFrame()`, which is @UiThread and would throw if
// called directly from the bg worker. Without this wake-up the Flutter
// compositor stays idle even though the BufferQueue holds a new frame,
// leaving `vsync_p95` at 50-140 ms on Tier-1 Android.
external fun setRiveRendererSurfaceProducer(
    renderer: Long,
    peer: RiveRenderTexture?,
)

class RiveNativePlugin :
    FlutterPlugin,
    MethodCallHandler {
    companion object {
        init {
            System.loadLibrary("rive_native")
        }
    }

    private lateinit var channel: MethodChannel
    private lateinit var textureRegistry: TextureRegistry
    private val renderTextures = mutableMapOf<Long, RiveRenderTexture>()

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "rive_native")
        channel.setMethodCallHandler(this)
        textureRegistry = flutterPluginBinding.textureRegistry
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        // Clean up all remaining textures
        renderTextures.values.forEach { it.release() }
        renderTextures.clear()
        channel.setMethodCallHandler(null)
    }

    override fun onMethodCall(
        call: MethodCall,
        result: Result,
    ) {
        when (call.method) {
            "createTexture" -> {
                val width = call.argument<Int>("width")
                val height = call.argument<Int>("height")

                if (width == null || height == null) {
                    result.error(
                        "CreateTexture Error",
                        "Width and height are required",
                        null,
                    )
                    return
                }

                val surfaceProducer = textureRegistry.createSurfaceProducer()
                val riveTexture = RiveRenderTexture(surfaceProducer, width, height)
                renderTextures[surfaceProducer.id()] = riveTexture


                result.success(
                    mapOf(
                        "textureId" to surfaceProducer.id(),
                        "renderer" to riveTexture.riveRenderer.toString(16),
                    ),
                )
            }

            "getRenderContext" -> {
                result.success(
                    mapOf(
                        "rendererContext" to "android",
                    ),
                )
            }

            "removeTexture" -> {
                val textureId = call.argument<Int>("id")?.toLong()
                if (textureId == null) {
                    result.error(
                        "removeTexture Error",
                        "Texture ID is required",
                        null,
                    )
                    return
                }

                renderTextures[textureId]?.let { texture ->
                    texture.release()
                    renderTextures.remove(textureId)
                    result.success(null)
                } ?: run {
                    Log.e("RiveNativePlugin", "removeTexture: texture $textureId not found")
                    result.error(
                        "removeTexture Error",
                        "Texture not found",
                        null,
                    )
                }
            }

            else -> result.notImplemented()
        }
    }
}

class RiveRenderTexture(
    surfaceProducer: TextureRegistry.SurfaceProducer,
    width: Int,
    height: Int,
) : TextureRegistry.SurfaceProducer.Callback {
    private val producer: TextureRegistry.SurfaceProducer = surfaceProducer
    private var surface: Surface
    var riveRenderer: Long = 0

    // Handler bound to the main looper so the @UiThread-annotated
    // SurfaceProducer.scheduleFrame can be invoked from a bg worker via JNI:
    // we never call producer.scheduleFrame directly from native, only via
    // `scheduleFrame()` below which posts to this handler.
    private val mainHandler = Handler(Looper.getMainLooper())

    // Cheap "frame already scheduled" guard so a 60 Hz bg worker doesn't
    // pile up 60 main-thread runnables per second — at most one pending
    // post is queued; subsequent bg-thread calls coalesce until the post
    // actually runs and clears the flag.
    @Volatile
    private var schedulePending: Boolean = false

    /**
     * Called from the C++ background render-success path (via JNI). Posts a
     * Runnable to the main thread Handler that invokes
     * SurfaceProducer.scheduleFrame(). Coalesces multiple in-flight requests
     * to a single pending post via [schedulePending].
     */
    @Suppress("unused")
    fun scheduleFrame() {
        if (schedulePending) return
        schedulePending = true
        mainHandler.post {
            schedulePending = false
            try {
                producer.scheduleFrame()
            } catch (e: Throwable) {
                Log.w(
                    "RiveNativePlugin",
                    "scheduleFrame failed (producer may be released): $e",
                )
            }
        }
    }

    init {
        producer.setSize(width, height)
        producer.setCallback(
            this,
        )

        surface = producer.surface
        riveRenderer =
            createRiveRenderer(
                surface,
                width,
                height,
            )
        // Hand this peer to native so the bg-thread render path can call
        // `RiveRenderTexture.scheduleFrame()` after each successful
        // eglSwapBuffers — that wrapper posts to the main thread Handler
        // before invoking `SurfaceProducer.scheduleFrame()` (which is
        // @UiThread and would throw otherwise).
        if (riveRenderer != 0L) {
            setRiveRendererSurfaceProducer(riveRenderer, this)
        }
    }

    // Called when coming back from backgrounding.
    override fun onSurfaceAvailable() {
        // This should only happen when returning from backgrounding.
        markRendererDestroyed()
        // Make a new surface for Flutter to use, but we're going to build a new
        // texture shortly...
        surface = producer.getSurface()
    }

    override fun onSurfaceCleanup() {
        // Do surface cleanup here, and stop drawing frames.
        markRendererDestroyed()
    }

    private fun markRendererDestroyed() {
        synchronized(this) {
            if (riveRenderer != 0L) {
                markDestroyedRiveRenderer(riveRenderer)
            }
        }
    }

    fun release() {
        synchronized(this) {
            if (riveRenderer != 0L) {
                // Drop the native-side SurfaceProducer ref before deleting
                // the renderer so the global ref is freed via a valid env.
                setRiveRendererSurfaceProducer(riveRenderer, null)
                destroyRiveRenderer(riveRenderer)
                riveRenderer = 0
            }
            try {
                surface.release()
                producer.release()
            } catch (e: Exception) {
                Log.w("RiveNativePlugin", "release: error releasing surface: $e")
                // Surface may already be released, ignore
            }
        }
    }
}
