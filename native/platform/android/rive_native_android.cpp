#include "rive_native/external.hpp"
#include "rive_native/rive_binding.hpp"
#include "rive/renderer/rive_renderer.hpp"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "rive/renderer/gl/render_context_gl_impl.hpp"
#include "rive/renderer/gl/render_target_gl.hpp"

#include <android/log.h>
#include <android/native_window_jni.h>
#include <unordered_map>

#include <jni.h>

#define LOG_TAG "RiveNative"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

std::recursive_mutex flutterMutex;

// ----- JVM bridge for the RiveRenderTexture.scheduleFrame() wrapper -----
// Set in JNI_OnLoad. Used by AndroidRenderTexture::endFrame to wake Flutter's
// compositor after a successful eglSwapBuffers. We can NOT call
// SurfaceProducer.scheduleFrame() directly from the bg worker — it is
// annotated @UiThread and throws RuntimeException off the main thread. The
// Kotlin RiveRenderTexture.scheduleFrame() method posts a Runnable to the
// main-Looper Handler that invokes producer.scheduleFrame on the UI thread,
// with a coalescing flag so a 60 Hz bg worker only enqueues one pending
// post at a time.
//
// Without this wake the Flutter compositor stays idle even though the
// BufferQueue holds a new frame, leaving `vsync_p95` at 50-140ms on Tier-1
// Android.
static JavaVM* g_javaVM = nullptr;
static jclass g_riveRenderTextureClass = nullptr;
static jmethodID g_scheduleFrameMid = nullptr;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    g_javaVM = vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        return JNI_VERSION_1_6;
    }
    jclass localCls =
        env->FindClass("app/rive/rive_native/RiveRenderTexture");
    if (localCls != nullptr)
    {
        g_riveRenderTextureClass =
            reinterpret_cast<jclass>(env->NewGlobalRef(localCls));
        env->DeleteLocalRef(localCls);
        g_scheduleFrameMid =
            env->GetMethodID(g_riveRenderTextureClass,
                             "scheduleFrame",
                             "()V");
        if (g_scheduleFrameMid == nullptr)
        {
            env->ExceptionClear();
            LOGW("RiveRenderTexture.scheduleFrame() not found — bg-thread "
                 "compositor wake will be unavailable.");
        }
    }
    else
    {
        env->ExceptionClear();
        LOGW("RiveRenderTexture class not found — bg-thread compositor wake "
             "will be unavailable.");
    }
    return JNI_VERSION_1_6;
}

// Per-thread cached JNIEnv* for the Rive bg worker. Attached as a daemon so
// the thread does not need to detach before exit. Returns nullptr if the JVM
// pointer is null (JNI_OnLoad never ran — should be impossible in practice).
static JNIEnv* getBgThreadJniEnv()
{
    if (g_javaVM == nullptr) return nullptr;
    thread_local JNIEnv* env = nullptr;
    if (env != nullptr) return env;
    JavaVMAttachArgs args = {JNI_VERSION_1_6, "RiveBgWorker", nullptr};
    if (g_javaVM->AttachCurrentThreadAsDaemon(&env, &args) != JNI_OK)
    {
        env = nullptr;
    }
    return env;
}

#define EGL_ERR_CHECK() _check_egl_error(__FILE__, __LINE__)

void _check_egl_error(const char* file, int line)
{
    EGLenum err(eglGetError());

    while (true)
    {
        std::string error;

        switch (err)
        {
            case EGL_SUCCESS:
                return;
            case EGL_NOT_INITIALIZED:
                error = "EGL_NOT_INITIALIZED";
                break;
            case EGL_BAD_ACCESS:
                error = "EGL_BAD_ACCESS";
                break;
            case EGL_BAD_ALLOC:
                error = "EGL_BAD_ALLOC";
                break;
            case EGL_BAD_ATTRIBUTE:
                error = "EGL_BAD_ATTRIBUTE";
                break;
            case EGL_BAD_CONTEXT:
                error = "EGL_BAD_CONTEXT";
                break;
            case EGL_BAD_CONFIG:
                error = "EGL_BAD_CONFIG";
                break;
            case EGL_BAD_CURRENT_SURFACE:
                error = "EGL_BAD_CURRENT_SURFACE";
                break;
            case EGL_BAD_DISPLAY:
                error = "EGL_BAD_DISPLAY";
                break;
            case EGL_BAD_SURFACE:
                error = "EGL_BAD_SURFACE";
                break;
            case EGL_BAD_MATCH:
                error = "EGL_BAD_MATCH";
                break;
            case EGL_BAD_PARAMETER:
                error = "EGL_BAD_PARAMETER";
                break;
            case EGL_BAD_NATIVE_PIXMAP:
                error = "EGL_BAD_NATIVE_PIXMAP";
                break;
            case EGL_BAD_NATIVE_WINDOW:
                error = "EGL_BAD_NATIVE_WINDOW";
                break;
            case EGL_CONTEXT_LOST:
                error = "EGL_CONTEXT_LOST";
                break;
            default:
                LOGE("(%d) %s - %s:%d", err, "Unknown", file, line);
                return;
        }
        LOGE("(%d) %s - %s:%d", err, error.c_str(), file, line);
        err = eglGetError();
    }
}

static bool config_has_attribute(EGLDisplay display,
                                 EGLConfig config,
                                 EGLint attribute,
                                 EGLint value)
{
    EGLint outValue = 0;
    EGLBoolean result =
        eglGetConfigAttrib(display, config, attribute, &outValue);
    EGL_ERR_CHECK();
    return result && (outValue == value);
}

class EGLThreadState
{
public:
    EGLThreadState()
    {
        m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (m_display == EGL_NO_DISPLAY)
        {
            EGL_ERR_CHECK();
            LOGE("eglGetDisplay() failed.");
            return;
        }
        if (!eglInitialize(m_display, 0, 0))
        {
            EGL_ERR_CHECK();
            LOGE("eglInitialize() failed.");
            return;
        }

        const EGLint configAttributes[] = {EGL_RENDERABLE_TYPE,
                                           EGL_OPENGL_ES2_BIT,
                                           EGL_BLUE_SIZE,
                                           8,
                                           EGL_GREEN_SIZE,
                                           8,
                                           EGL_RED_SIZE,
                                           8,
                                           EGL_DEPTH_SIZE,
                                           0,
                                           EGL_STENCIL_SIZE,
                                           8,
                                           EGL_ALPHA_SIZE,
                                           8,
                                           EGL_NONE};
        EGLint num_configs = 0;
        if (!eglChooseConfig(m_display,
                             configAttributes,
                             nullptr,
                             0,
                             &num_configs))
        {
            EGL_ERR_CHECK();
            LOGE("eglChooseConfig() didn't find any (%d)", num_configs);
            return;
        }

        std::vector<EGLConfig> supportedConfigs(
            static_cast<size_t>(num_configs));
        eglChooseConfig(m_display,
                        configAttributes,
                        supportedConfigs.data(),
                        num_configs,
                        &num_configs);
        EGL_ERR_CHECK();

        // Choose a config, either a match if possible or the first config
        // otherwise
        const auto configMatches = [&](EGLConfig config) {
            if (!config_has_attribute(m_display, config, EGL_RED_SIZE, 8))
                return false;
            if (!config_has_attribute(m_display, config, EGL_GREEN_SIZE, 8))
                return false;
            if (!config_has_attribute(m_display, config, EGL_BLUE_SIZE, 8))
                return false;
            if (!config_has_attribute(m_display, config, EGL_STENCIL_SIZE, 8))
                return false;
            return config_has_attribute(m_display, config, EGL_DEPTH_SIZE, 0);
        };

        const auto configIter = std::find_if(supportedConfigs.cbegin(),
                                             supportedConfigs.cend(),
                                             configMatches);

        m_config = (configIter != supportedConfigs.cend())
                       ? *configIter
                       : supportedConfigs[0];

        const EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION,
                                            2,
                                            EGL_NONE};

        m_context =
            eglCreateContext(m_display, m_config, nullptr, contextAttributes);
        if (m_context == EGL_NO_CONTEXT)
        {
            LOGE("eglCreateContext() failed.");
            EGL_ERR_CHECK();
        }

        // PLS
        // Create a 1x1 Pbuffer surface that we can use to guarantee m_context
        // is
        // always current on this thread.
        const EGLint PbufferAttrs[] = {
            EGL_WIDTH,
            1,
            EGL_HEIGHT,
            1,
            EGL_NONE,
        };
        m_backgroundSurface =
            eglCreatePbufferSurface(m_display, m_config, PbufferAttrs);
        EGL_ERR_CHECK();
        if (m_backgroundSurface == EGL_NO_SURFACE)
        {
            LOGE("Failed to create a 1x1 background Pbuffer surface for PLS");
        }

        eglMakeCurrent(m_display,
                       m_backgroundSurface,
                       m_backgroundSurface,
                       m_context);
        m_currentSurface = m_backgroundSurface;

        m_renderContext = rive::gpu::RenderContextGLImpl::MakeContext();
    }

    ~EGLThreadState()
    {
        LOGD("EGLThreadState getting destroyed! 🧨");

        // Destruction order matters. Members get destructed in reverse
        // declaration order AFTER this body returns, so `m_renderContext`
        // (`rive::gpu::RenderContextGLImpl::~RenderContextGLImpl`) would
        // otherwise run after the `eglDestroyContext` / `eglTerminate`
        // calls below — calling `glDeleteTextures` (and the rest of Rive's
        // GL teardown) against a destroyed GL context. The GLES driver
        // null-dereferences in that path during `__cxa_thread_finalize`,
        // crashing the bg worker thread with SIGSEGV inside
        // `__cxa_thread_finalize`.
        //
        // Make our pbuffer surface current so the Rive render-context
        // destructor sees a valid GL context, then tear it down first.
        // After that the EGL surface/context/display can be safely
        // destroyed in reverse-creation order.
        if (m_context != EGL_NO_CONTEXT &&
            m_display != EGL_NO_DISPLAY &&
            m_backgroundSurface != EGL_NO_SURFACE)
        {
            eglMakeCurrent(m_display,
                           m_backgroundSurface,
                           m_backgroundSurface,
                           m_context);
            EGL_ERR_CHECK();
        }
        m_renderContext.reset();

        if (m_context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(m_display, m_context);
            EGL_ERR_CHECK();
        }

        eglReleaseThread();
        EGL_ERR_CHECK();

        if (m_display != EGL_NO_DISPLAY)
        {
            eglTerminate(m_display);
            EGL_ERR_CHECK();
        }
    }

    EGLSurface createEGLSurface(ANativeWindow* window)
    {
        if (!window)
        {
            return EGL_NO_SURFACE;
        }

        auto res = eglCreateWindowSurface(m_display, m_config, window, nullptr);
        EGL_ERR_CHECK();
        return res;
    }

    void destroySurface(EGLSurface eglSurface)
    {
        if (eglSurface == EGL_NO_SURFACE)
        {
            return;
        }

        assert(eglSurface != m_backgroundSurface);
        if (m_currentSurface == eglSurface)
        {
            // Make sure m_context always stays current.
            makeCurrent(m_backgroundSurface);
        }

        eglDestroySurface(m_display, eglSurface);
        EGL_ERR_CHECK();
    }

    bool makeCurrent(EGLSurface eglSurface)
    {
        if (eglSurface == m_currentSurface)
        {
            // return;
        }

        if (eglSurface == EGL_NO_SURFACE)
        {
            LOGE("Cannot make EGL_NO_SURFACE current");
            return false;
        }

        if (!eglMakeCurrent(m_display, eglSurface, eglSurface, m_context))
        {
            LOGE("eglMakeCurrent failed");
            EGL_ERR_CHECK();
            return false;
        }

        m_currentSurface = eglSurface;
        return true;
    }

    void swapBuffers()
    {
        eglSwapBuffers(m_display, m_currentSurface);
        EGL_ERR_CHECK();
    }

    rive::gpu::RenderContext* renderContext() const
    {
        return m_renderContext.get();
    }

protected:
    EGLSurface m_currentSurface = EGL_NO_SURFACE;
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLContext m_context = EGL_NO_CONTEXT;
    EGLConfig m_config = static_cast<EGLConfig>(0);

    std::unique_ptr<rive::gpu::RenderContext> m_renderContext;

    // 1x1 Pbuffer surface that allows us to make the GL context current without
    // a window surface.
    EGLSurface m_backgroundSurface;
};

class AndroidRenderTexture
{
public:
    AndroidRenderTexture(ANativeWindow* surfaceWindow,
                         uint32_t width,
                         uint32_t height) :
        m_surfaceWindow(surfaceWindow), m_width(width), m_height(height)
    {
        ANativeWindow_acquire(surfaceWindow);
    }

    ~AndroidRenderTexture()
    {
        // Clean up GPU resources first, in reverse order of creation
        if (threadState && m_eglSurface != EGL_NO_SURFACE)
        {
            // threadState->makeCurrent(m_eglSurface);
            threadState->destroySurface(m_eglSurface);
            m_eglSurface = EGL_NO_SURFACE;
        }

        if (m_surfaceWindow)
        {
            ANativeWindow_release(m_surfaceWindow);
            m_surfaceWindow = nullptr;
        }

        // TODO: Rive Android calls `releaseResources` when there are no Rive
        // views if (threadState && threadState->renderContext())
        // {
        //     threadState->renderContext()->releaseResources();
        // }
    }

    bool beginFrame(bool clear, uint32_t color)
    {
        if (m_scheduledDestruction)
        {
            LOGW("Rive AndroidRenderTexture beginFrame called on destroyed "
                 "texture");
            return false;
        }
        if (!threadState)
        {
            LOGD("Rive AndroidRenderTexture creating EGLThreadState");
            threadState = std::make_unique<EGLThreadState>();
        }

        if (!m_renderTarget)
        {
            m_eglSurface = threadState->createEGLSurface(m_surfaceWindow);
            if (m_eglSurface == EGL_NO_SURFACE)
            {
                LOGE("AndroidRenderTexture failed to create EGL surface");
                return false;
            }

            if (!threadState->makeCurrent(m_eglSurface))
            {
                return false;
            }
            auto renderContext = threadState->renderContext();
            if (renderContext == nullptr)
            {
                LOGE("Rive AndroidRenderTexture: Renderer (PLS) NOT "
                     "supported on this device (surface=%dx%d). The "
                     "threaded path will silently no-op every render "
                     "cycle from now on — m_plsRenderer stays null, "
                     "makeRenderer() returns null, the bg callback hits "
                     "its 'makeRenderer returned null' branch and marks "
                     "fatal. Only viable mitigation is to disable "
                     "threaded rendering for this device (ThreadedRive"
                     "BenchMode.forceSyncRendering, or the throttle "
                     "config disableThreadedRiveAdvance flag).",
                     m_width,
                     m_height);
                return true; // PLS was not supported.
            }
            int width = ANativeWindow_getWidth(m_surfaceWindow);
            int height = ANativeWindow_getHeight(m_surfaceWindow);
            assert(width == m_width);
            assert(height == m_height);

            GLint sampleCount;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glGetIntegerv(GL_SAMPLES, &sampleCount);
            m_renderTarget =
                rive::make_rcp<rive::gpu::FramebufferRenderTargetGL>(
                    width,
                    height,
                    0,
                    sampleCount);
            m_plsRenderer = std::make_unique<rive::RiveRenderer>(renderContext);
        }

        threadState->renderContext()->beginFrame({
            .renderTargetWidth = m_width,
            .renderTargetHeight = m_height,
            .loadAction = clear ? rive::gpu::LoadAction::clear
                                : rive::gpu::LoadAction::preserveRenderTarget,
            .clearColor = color,
        });
        return true;
    }

    bool endFrame(float devicePixelRatio)
    {
        if (m_scheduledDestruction)
        {
            LOGW("Rive AndroidRenderTexture endFrame called on destroyed "
                 "texture");
            return false;
        }

        if (!threadState || !m_renderTarget || m_eglSurface == EGL_NO_SURFACE)
        {
            LOGE("Rive AndroidRenderTexture endFrame called with invalid "
                 "state");
            return false;
        }

        auto renderContext = threadState->renderContext();
        if (!renderContext)
        {
            LOGE("Rive AndroidRenderTexture renderContext is null");
            return false;
        }

        auto plsGL =
            renderContext->static_impl_cast<rive::gpu::RenderContextGLImpl>();
        plsGL->invalidateGLState();

        if (!threadState->makeCurrent(m_eglSurface))
        {
            return false;
        }

        renderContext->flush({.renderTarget = m_renderTarget.get()});
        threadState->swapBuffers();

        plsGL->unbindGLInternalResources();

        // Wake Flutter's compositor. eglSwapBuffers alone is insufficient on
        // some Impeller GLES builds — the SurfaceProducer's underlying
        // BufferQueue signals the engine, but the engine doesn't always
        // follow up with a frame request without an explicit scheduleFrame.
        // Calling it from the bg thread is safe; SurfaceProducer.scheduleFrame
        // posts to the Flutter platform thread internally.
        if (m_surfaceProducer != nullptr && g_scheduleFrameMid != nullptr)
        {
            if (JNIEnv* env = getBgThreadJniEnv())
            {
                env->CallVoidMethod(m_surfaceProducer, g_scheduleFrameMid);
                if (env->ExceptionCheck())
                {
                    env->ExceptionDescribe();
                    env->ExceptionClear();
                }
            }
        }

        return true;
    }

    void scheduleDestruction() { m_scheduledDestruction = true; }

    // Stores a Java global ref to the SurfaceProducer driving this texture.
    // Called once from the JNI createRiveRenderer entry point. The endFrame
    // callback invokes SurfaceProducer.scheduleFrame() through this ref to
    // wake Flutter's compositor after each successful swap.
    void setSurfaceProducer(JNIEnv* env, jobject producer)
    {
        if (m_surfaceProducer != nullptr)
        {
            env->DeleteGlobalRef(m_surfaceProducer);
            m_surfaceProducer = nullptr;
        }
        if (producer != nullptr)
        {
            m_surfaceProducer = env->NewGlobalRef(producer);
        }
    }

private:
    ANativeWindow* m_surfaceWindow;
    uint32_t m_width;
    uint32_t m_height;

    EGLSurface m_eglSurface = nullptr;
    rive::rcp<rive::gpu::RenderTargetGL> m_renderTarget;
    std::unique_ptr<rive::RiveRenderer> m_plsRenderer;
    bool m_scheduledDestruction = false;

    // Java global ref to the SurfaceProducer this texture writes to.
    // Allocated in setSurfaceProducer, released in releaseSurfaceProducer.
    // Read from endFrame to invoke SurfaceProducer.scheduleFrame().
    jobject m_surfaceProducer = nullptr;

public:
    static thread_local std::unique_ptr<EGLThreadState> threadState;

    rive::Renderer* renderer() { return m_plsRenderer.get(); }

    void releaseSurfaceProducer(JNIEnv* env)
    {
        if (m_surfaceProducer != nullptr && env != nullptr)
        {
            env->DeleteGlobalRef(m_surfaceProducer);
        }
        m_surfaceProducer = nullptr;
    }
};

thread_local std::unique_ptr<EGLThreadState> AndroidRenderTexture::threadState;

EXPORT rive::Factory* riveFactory()
{
    std::unique_lock<std::recursive_mutex> lock(flutterMutex);
    if (!AndroidRenderTexture::threadState)
    {
        AndroidRenderTexture::threadState = std::make_unique<EGLThreadState>();
    }
    assert(AndroidRenderTexture::threadState != nullptr);
    assert(AndroidRenderTexture::threadState->renderContext() != nullptr);
    return AndroidRenderTexture::threadState->renderContext();
}

EXPORT jlong Java_app_rive_rive_1native_RiveNativePluginKt_createRiveRenderer(
    JNIEnv* env,
    jclass clazz,
    jobject surface,
    jint width,
    jint height)
{
    ANativeWindow* surfaceWindow = ANativeWindow_fromSurface(env, surface);

    AndroidRenderTexture* renderTexture =
        new AndroidRenderTexture(surfaceWindow,
                                 (uint32_t)width,
                                 (uint32_t)height);
    return reinterpret_cast<jlong>(renderTexture);
}

EXPORT void Java_app_rive_rive_1native_RiveNativePluginKt_destroyRiveRenderer(
    JNIEnv* env,
    jclass clazz,
    jlong renderer)
{
    std::unique_lock<std::recursive_mutex> lock(flutterMutex);
    if (renderer != 0)
    {
        AndroidRenderTexture* renderTexture =
            reinterpret_cast<AndroidRenderTexture*>(renderer);
        // Free the SurfaceProducer global ref while we have a valid JNIEnv*;
        // the destructor doesn't get one and can't clean it up itself.
        renderTexture->releaseSurfaceProducer(env);
        delete renderTexture;
    }
    else
    {
        LOGW("JNI: Rive destroyRiveRenderer called with null pointer");
    }
}

// JNI entry point so the Kotlin RiveRenderTexture can hand its
// TextureRegistry.SurfaceProducer to the bg-thread render path. After the
// renderer is created, Kotlin calls this with the same SurfaceProducer that
// owns the Surface; subsequent endFrame calls invoke SurfaceProducer
// .scheduleFrame() on it. Passing null clears any previously-set producer.
EXPORT void
Java_app_rive_rive_1native_RiveNativePluginKt_setRiveRendererSurfaceProducer(
    JNIEnv* env,
    jclass clazz,
    jlong renderer,
    jobject surfaceProducer)
{
    std::unique_lock<std::recursive_mutex> lock(flutterMutex);
    if (renderer == 0)
    {
        LOGW("JNI: setRiveRendererSurfaceProducer called with null renderer");
        return;
    }
    AndroidRenderTexture* renderTexture =
        reinterpret_cast<AndroidRenderTexture*>(renderer);
    renderTexture->setSurfaceProducer(env, surfaceProducer);
}

EXPORT void
Java_app_rive_rive_1native_RiveNativePluginKt_markDestroyedRiveRenderer(
    JNIEnv* env,
    jclass clazz,
    jlong renderer)
{
    std::unique_lock<std::recursive_mutex> lock(flutterMutex);
    if (renderer != 0)
    {
        AndroidRenderTexture* renderTexture =
            reinterpret_cast<AndroidRenderTexture*>(renderer);
        renderTexture->scheduleDestruction();
    }
    else
    {
        LOGW("JNI: Rive markDestroyedRiveRenderer called with null pointer");
    }
}

EXPORT bool clear(AndroidRenderTexture* renderTexture,
                  bool clear,
                  uint32_t color)
{
    if (renderTexture == nullptr)
    {
        return false;
    }
    std::unique_lock<std::recursive_mutex> lock(flutterMutex);
    return renderTexture->beginFrame(clear, color);
}

EXPORT bool flush(AndroidRenderTexture* renderTexture, float devicePixelRatio)
{
    if (renderTexture == nullptr)
    {
        return false;
    }

    std::unique_lock<std::recursive_mutex> lock(flutterMutex);
    return renderTexture->endFrame(devicePixelRatio);
}

EXPORT rive::Renderer* makeRenderer(AndroidRenderTexture* renderTexture)
{
    if (renderTexture == nullptr)
    {
        return nullptr;
    }
    std::unique_lock<std::recursive_mutex> lock(flutterMutex);
    return renderTexture->renderer();
}
