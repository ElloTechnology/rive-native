// ThreadedScene binding for Android GLES — COR-3538 Phase 2 spike.
// Android counterpart of threaded_scene_binding.mm; renders through
// AndroidRenderTexture (rive_native_android.cpp).

#include "rive_native/external.hpp"
#include "rive/threaded_scene.hpp"
#include "rive/artboard.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/renderer/rive_renderer.hpp"
#include "rive/math/mat2d.hpp"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/log.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#define PROBE_TAG "CorRive3538Probe"
#define PROBE_LOGI(...) __android_log_print(ANDROID_LOG_INFO, PROBE_TAG, __VA_ARGS__)
#define PROBE_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, PROBE_TAG, __VA_ARGS__)

#define BG_TAG "RiveBgBinding"
#define BG_LOGI(...) __android_log_print(ANDROID_LOG_INFO, BG_TAG, __VA_ARGS__)
#define BG_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, BG_TAG, __VA_ARGS__)

// Implemented in rive_native_android.cpp.
extern "C" {
EGLDisplay riveAndroidGetMainEGLDisplay();
EGLContext riveAndroidGetMainEGLContext();
EGLConfig riveAndroidGetMainEGLConfig();
}

class AndroidRenderTexture;
EXPORT bool clear(AndroidRenderTexture* renderTexture,
                  bool clear,
                  uint32_t color);
EXPORT bool flush(AndroidRenderTexture* renderTexture, float devicePixelRatio);
EXPORT rive::Renderer* makeRenderer(AndroidRenderTexture* renderTexture);

namespace rive_flutter
{

class ThreadedSceneBinding
{
public:
    static std::unique_ptr<ThreadedSceneBinding> create(
        AndroidRenderTexture* renderTexture,
        std::unique_ptr<rive::ArtboardInstance> artboard,
        std::unique_ptr<rive::StateMachineInstance> stateMachine,
        rive::rcp<rive::ViewModelInstanceRuntime> viewModelInstance,
        int width,
        int height,
        float devicePixelRatio)
    {
        if (!renderTexture || !artboard || !stateMachine)
        {
            return nullptr;
        }

        auto binding = std::unique_ptr<ThreadedSceneBinding>(
            new ThreadedSceneBinding());
        binding->m_renderTexture = renderTexture;
        binding->m_devicePixelRatio = devicePixelRatio;

        float abWidth = artboard->width();
        float abHeight = artboard->height();

        rive::ThreadedScene::Config config;
        config.width = width;
        config.height = height;
        config.runFirstFrameSync = true;

        // The render callback runs on the background thread.
        // It drives AndroidRenderTexture::beginFrame/endFrame which writes
        // into the GL texture backed by the supplied ANativeWindow. Flutter
        // reads the latest via TextureLayer — no pixel copy needed.
        //
        // Fatal-error contract: `clear` and `flush` return false on any
        // EGL/GL failure (including EGL_CONTEXT_LOST, which the underlying
        // EGL_ERR_CHECK in rive_native_android.cpp consumes before the
        // return value reaches us). On any false return the callback marks
        // a binding-level fatal flag, stops drawing, and no-ops on
        // subsequent frames until Dart calls riveThreadedDestroy. The
        // application-side observer (riveThreadedHasFatalError + a
        // poll site in CharacterRig._onBackgroundFrame, COR-3538 Phase 6)
        // tears down the bg controller and falls back to the synchronous
        // path on next session.
        auto* renderTexturePtr = renderTexture;
        float dpr = devicePixelRatio;
        std::atomic<bool>* fatalFlag = &binding->m_fatalError;
        std::atomic<bool>* pausedFlag = &binding->m_paused;

        binding->m_scene = std::make_unique<rive::ThreadedScene>(
            std::move(artboard),
            std::move(stateMachine),
            config,
            [renderTexturePtr, abWidth, abHeight, dpr, fatalFlag, pausedFlag](
                rive::ArtboardInstance* ab,
                int w,
                int h) -> rive::rcp<rive::RenderImage> {
                if (fatalFlag->load(std::memory_order_acquire))
                {
                    return nullptr;
                }
                if (pausedFlag->load(std::memory_order_acquire))
                {
                    return nullptr;
                }
                if (!clear(renderTexturePtr, true, 0x00000000))
                {
                    BG_LOGE("clear() returned false; marking fatal "
                            "(likely EGL/GL failure, possibly "
                            "EGL_CONTEXT_LOST — see RiveNative logcat)");
                    fatalFlag->store(true, std::memory_order_release);
                    return nullptr;
                }
                auto* riveRenderer = makeRenderer(renderTexturePtr);
                if (!riveRenderer)
                {
                    BG_LOGE("makeRenderer() returned null; marking fatal");
                    fatalFlag->store(true, std::memory_order_release);
                    return nullptr;
                }
                riveRenderer->save();
                riveRenderer->transform(rive::Mat2D::fromScale(
                    static_cast<float>(w) / abWidth * dpr,
                    static_cast<float>(h) / abHeight * dpr));
                ab->draw(riveRenderer);
                riveRenderer->restore();
                if (!flush(renderTexturePtr, dpr))
                {
                    BG_LOGE("flush() returned false; marking fatal "
                            "(likely EGL/GL failure, possibly "
                            "EGL_CONTEXT_LOST — see RiveNative logcat)");
                    fatalFlag->store(true, std::memory_order_release);
                }
                return nullptr; // GPU path — texture IS the output
            },
            std::move(viewModelInstance));

        return binding;
    }

    ~ThreadedSceneBinding()
    {
        if (m_scene)
        {
            m_scene->stop();
        }
    }

    rive::ThreadedScene* scene() { return m_scene.get(); }

    bool hasFatalError() const
    {
        return m_fatalError.load(std::memory_order_acquire);
    }

    void setPaused(bool paused)
    {
        m_paused.store(paused, std::memory_order_release);
    }
    bool isPaused() const
    {
        return m_paused.load(std::memory_order_acquire);
    }

private:
    ThreadedSceneBinding() = default;

    AndroidRenderTexture* m_renderTexture = nullptr;
    std::unique_ptr<rive::ThreadedScene> m_scene;
    float m_devicePixelRatio = 1.0f;
    std::atomic<bool> m_fatalError{false};
    std::atomic<bool> m_paused{false};
};

} // namespace rive_flutter

// ==========================================================================
// FFI exports — called from Dart via dart:ffi.
// Symbol names match threaded_scene_binding.mm so Dart can link to either
// platform without conditional FFI symbol lookups.
// ==========================================================================

using namespace rive_flutter;

// --- Lifecycle ---

EXPORT void* riveThreadedCreate(
    void* androidRenderTexturePtr,
    void* artboardPtr,
    void* stateMachinePtr,
    void* viewModelInstancePtr,
    int width,
    int height,
    float devicePixelRatio)
{
    auto* renderTexture =
        static_cast<AndroidRenderTexture*>(androidRenderTexturePtr);

    // Take ownership: wrap raw pointers in unique_ptr.
    // The caller (Dart) must NOT use these pointers after this call.
    auto artboard = std::unique_ptr<rive::ArtboardInstance>(
        static_cast<rive::ArtboardInstance*>(artboardPtr));
    auto stateMachine = std::unique_ptr<rive::StateMachineInstance>(
        static_cast<rive::StateMachineInstance*>(stateMachinePtr));

    rive::rcp<rive::ViewModelInstanceRuntime> vmi;
    if (viewModelInstancePtr)
    {
        // ViewModelInstanceRuntime is ref-counted — add a ref since we're
        // taking shared ownership (the Dart side may still hold a ref).
        auto* raw = static_cast<rive::ViewModelInstanceRuntime*>(
            viewModelInstancePtr);
        vmi = rive::rcp<rive::ViewModelInstanceRuntime>(rive::ref_rcp(raw));
    }

    auto binding = ThreadedSceneBinding::create(
        renderTexture,
        std::move(artboard),
        std::move(stateMachine),
        std::move(vmi),
        width,
        height,
        devicePixelRatio);

    return binding ? binding.release() : nullptr;
}

EXPORT void riveThreadedDestroy(void* bindingPtr)
{
    delete static_cast<ThreadedSceneBinding*>(bindingPtr);
}

// --- Per-frame (called from Dart Ticker) ---

EXPORT void riveThreadedPostTime(void* bindingPtr, float dt)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene() && !binding->isPaused() &&
        !binding->hasFatalError())
    {
        binding->scene()->postElapsedTime(dt);
    }
}

// Pause / resume the bg worker. While paused, both `riveThreadedPostTime`
// no-ops AND the render callback no-ops — so the state machine stops
// advancing AND the worker stops drawing. The ThreadedScene's bg thread
// continues spinning on its condition variable; we don't tear down the
// thread, so resume is cheap (no re-init of artboard / SM / EGL).
//
// Wire from Dart's `WidgetsBindingObserver.didChangeAppLifecycleState`:
//   AppLifecycleState.paused / inactive / hidden  → setPaused(true)
//   AppLifecycleState.resumed                     → setPaused(false)
//
// Idempotent; safe to call from any thread.
EXPORT void riveThreadedSetPaused(void* bindingPtr, bool paused)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding)
    {
        binding->setPaused(paused);
    }
}

EXPORT int riveThreadedPollEvents(
    void* bindingPtr,
    const char** outNames,
    float* outDelays,
    int maxEvents)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (!binding || !binding->scene())
    {
        return 0;
    }
    thread_local std::vector<rive::ThreadedOutputEvent> events;
    events.clear();
    binding->scene()->pollReportedEvents(events);

    // Phase 5 (plan.md) bumps Dart-side maxEvents to 128 and wraps the poll
    // in a draining loop so any C++-side queue depth surfaces in full. The
    // truncation at std::min stays here as a hard upper bound per call;
    // Dart drains in a loop until batch.length < maxEvents.
    int count = static_cast<int>(
        std::min(events.size(), static_cast<size_t>(maxEvents)));
    for (int i = 0; i < count; i++)
    {
        outNames[i] = events[i].eventName.c_str();
        outDelays[i] = events[i].secondsDelay;
    }
    return count;
}

// --- ViewModel inputs ---

EXPORT void riveThreadedSetVmEnum(
    void* bindingPtr, const char* name, const char* value)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->setViewModelEnum(name, value);
    }
}

EXPORT void riveThreadedSetVmNumber(
    void* bindingPtr, const char* name, float value)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->setViewModelNumber(name, value);
    }
}

EXPORT void riveThreadedSetVmBool(
    void* bindingPtr, const char* name, bool value)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->setViewModelBool(name, value);
    }
}

EXPORT void riveThreadedSetVmString(
    void* bindingPtr, const char* name, const char* value)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->setViewModelString(name, value);
    }
}

EXPORT void riveThreadedFireVmTrigger(void* bindingPtr, const char* name)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->fireViewModelTrigger(name);
    }
}

// --- ViewModel snapshot ---

EXPORT void riveThreadedWatchProperty(void* bindingPtr, const char* name)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->watchViewModelProperty(name);
    }
}

EXPORT void riveThreadedUnwatchProperty(void* bindingPtr, const char* name)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->unwatchViewModelProperty(name);
    }
}

/// Acquire the latest ViewModel snapshot.
/// Writes property names and string-encoded values into pre-allocated arrays.
/// Returns the number of properties written.
///
/// Value encoding: enum/string → the string value, number → string of float,
/// bool → "true"/"false", monostate → "".
EXPORT int riveThreadedAcquireSnapshot(
    void* bindingPtr,
    const char** outNames,
    const char** outValues,
    int* outTypes, // 0=monostate, 1=bool, 2=float, 3=string
    int maxProperties)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (!binding || !binding->scene())
    {
        return 0;
    }

    // Keep the snapshot alive until next call via thread_local storage.
    thread_local rive::ViewModelSnapshot snapshot;
    thread_local std::vector<std::string> stringValues;
    snapshot = binding->scene()->acquireViewModelSnapshot();
    stringValues.clear();

    int count = 0;
    for (const auto& [name, value] : snapshot)
    {
        if (count >= maxProperties)
            break;

        outNames[count] = name.c_str();

        if (std::holds_alternative<std::monostate>(value))
        {
            outTypes[count] = 0;
            stringValues.emplace_back("");
        }
        else if (std::holds_alternative<bool>(value))
        {
            outTypes[count] = 1;
            stringValues.emplace_back(
                std::get<bool>(value) ? "true" : "false");
        }
        else if (std::holds_alternative<float>(value))
        {
            outTypes[count] = 2;
            stringValues.emplace_back(
                std::to_string(std::get<float>(value)));
        }
        else if (std::holds_alternative<std::string>(value))
        {
            outTypes[count] = 3;
            stringValues.emplace_back(std::get<std::string>(value));
        }

        outValues[count] = stringValues.back().c_str();
        count++;
    }
    return count;
}

// --- Pointer events ---

EXPORT void riveThreadedPointerDown(
    void* bindingPtr, float x, float y, int pointerId)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->pointerDown({x, y}, pointerId);
    }
}

EXPORT void riveThreadedPointerMove(
    void* bindingPtr, float x, float y, int pointerId)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->pointerMove({x, y}, pointerId);
    }
}

EXPORT void riveThreadedPointerUp(
    void* bindingPtr, float x, float y, int pointerId)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->pointerUp({x, y}, pointerId);
    }
}

EXPORT void riveThreadedPointerExit(
    void* bindingPtr, float x, float y, int pointerId)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding && binding->scene())
    {
        binding->scene()->pointerExit({x, y}, pointerId);
    }
}

// --- Queries ---

EXPORT bool riveThreadedIsRunning(void* bindingPtr)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    return binding && binding->scene() && binding->scene()->isRunning();
}

// Returns true if the bg worker hit a fatal native error (EGL/GL failure,
// surfaced as a `false` return from clear/makeRenderer/flush). Dart-side
// polls this each frame (COR-3538 Phase 6 wiring) and tears down the bg
// controller on detect, falling back to the synchronous path on the next
// session. The flag is one-way — once set, the binding stops drawing and
// the next riveThreadedDestroy clears the underlying ThreadedScene.
EXPORT bool riveThreadedHasFatalError(void* bindingPtr)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    return binding && binding->hasFatalError();
}

// Shared-context probe: verifies eglCreateContext(share_context=main) lets
// a worker thread see GL resources created by the main thread (and vice
// versa). Returns 0 on success, negative on the step that failed; native
// logs go to tag CorRive3538Probe. Spike-only.
EXPORT int riveSpikeProbeSharedContext()
{
    PROBE_LOGI("--- begin ---");

    // Step 1: read main thread's EGL handles.
    EGLDisplay mainDisplay = riveAndroidGetMainEGLDisplay();
    EGLContext mainContext = riveAndroidGetMainEGLContext();
    EGLConfig mainConfig = riveAndroidGetMainEGLConfig();
    PROBE_LOGI("step1: main display=%p context=%p config=%p",
               mainDisplay, mainContext, mainConfig);
    if (mainDisplay == EGL_NO_DISPLAY || mainContext == EGL_NO_CONTEXT)
    {
        PROBE_LOGE("step1: main EGLThreadState not initialized — "
                   "ensure Rive has rendered at least one frame "
                   "before probing");
        return -1;
    }

    // Step 2: create worker context with share_context = main.
    const EGLint contextAttrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
    };
    EGLContext workerContext =
        eglCreateContext(mainDisplay, mainConfig, mainContext, contextAttrs);
    if (workerContext == EGL_NO_CONTEXT)
    {
        EGLint err = eglGetError();
        PROBE_LOGE("step2: eglCreateContext(share=%p) failed: 0x%x",
                   mainContext, err);
        return -2;
    }
    PROBE_LOGI("step2: worker context=%p (shares with main=%p)",
               workerContext, mainContext);

    // Step 3: 1x1 PBuffer so workerContext can be made current without a
    // window surface.
    const EGLint pbAttrs[] = {
        EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE,
    };
    EGLSurface workerSurface =
        eglCreatePbufferSurface(mainDisplay, mainConfig, pbAttrs);
    if (workerSurface == EGL_NO_SURFACE)
    {
        EGLint err = eglGetError();
        PROBE_LOGE("step3: eglCreatePbufferSurface failed: 0x%x", err);
        eglDestroyContext(mainDisplay, workerContext);
        return -3;
    }
    PROBE_LOGI("step3: worker pbuffer surface=%p", workerSurface);

    // Step 4-5: spawn worker thread, create a texture, capture its name.
    std::atomic<GLuint> workerTextureName{0};
    std::atomic<int> workerResult{0};
    std::thread worker([&]() {
        if (!eglMakeCurrent(mainDisplay,
                            workerSurface,
                            workerSurface,
                            workerContext))
        {
            EGLint err = eglGetError();
            PROBE_LOGE("step4: worker eglMakeCurrent failed: 0x%x", err);
            workerResult = -4;
            return;
        }
        EGLContext currentOnWorker = eglGetCurrentContext();
        PROBE_LOGI("step4: worker thread eglGetCurrentContext=%p "
                   "(expected %p)",
                   currentOnWorker, workerContext);
        if (currentOnWorker != workerContext)
        {
            PROBE_LOGE("step4: worker current context mismatch");
            workerResult = -4;
            return;
        }

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     4,
                     4,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     nullptr);
        GLenum glerr = glGetError();
        if (glerr != GL_NO_ERROR)
        {
            PROBE_LOGE("step5: worker glTexImage2D failed: 0x%x", glerr);
            workerResult = -5;
            return;
        }
        glFinish();
        workerTextureName = tex;
        PROBE_LOGI("step5: worker created texture name=%u (glFinish'd)", tex);

        // Release the context from this thread so the probe thread can use
        // it (or use main).
        eglMakeCurrent(mainDisplay,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);
    });
    worker.join();

    if (workerResult.load() < 0)
    {
        PROBE_LOGE("worker reported failure (result=%d) — tearing down",
                   workerResult.load());
        eglDestroySurface(mainDisplay, workerSurface);
        eglDestroyContext(mainDisplay, workerContext);
        return workerResult.load();
    }

    // Step 6: from the probe thread, verify the worker's texture is visible.
    // We need a context current on this thread to call glIsTexture. Try
    // making main current (the cleanest cross-thread sharing test).
    if (!eglMakeCurrent(mainDisplay,
                        workerSurface,
                        workerSurface,
                        mainContext))
    {
        EGLint err = eglGetError();
        PROBE_LOGE("step6: probe-thread eglMakeCurrent(main) failed: 0x%x — "
                   "main context may be in use on another thread (Flutter's "
                   "render thread). Falling back to worker context.",
                   err);
        // Fallback: use worker context from probe thread. Still tests
        // sharing because the texture was created on a DIFFERENT thread.
        if (!eglMakeCurrent(mainDisplay,
                            workerSurface,
                            workerSurface,
                            workerContext))
        {
            EGLint err2 = eglGetError();
            PROBE_LOGE("step6: probe-thread eglMakeCurrent(worker) also "
                       "failed: 0x%x",
                       err2);
            eglDestroySurface(mainDisplay, workerSurface);
            eglDestroyContext(mainDisplay, workerContext);
            return -6;
        }
        PROBE_LOGI("step6: using worker context from probe thread "
                   "(cross-thread same-context test, not cross-context "
                   "sharing test)");
    }
    else
    {
        PROBE_LOGI("step6: probe thread now has main context current "
                   "— this is the real cross-context sharing test");
    }

    GLuint tname = workerTextureName.load();
    GLboolean isShared = glIsTexture(tname);
    GLenum afterErr = glGetError();
    PROBE_LOGI("step6: glIsTexture(%u)=%s (post-call glGetError=0x%x)",
               tname,
               isShared ? "TRUE" : "FALSE",
               afterErr);

    // Cleanup.
    eglMakeCurrent(mainDisplay,
                   EGL_NO_SURFACE,
                   EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
    eglDestroySurface(mainDisplay, workerSurface);
    eglDestroyContext(mainDisplay, workerContext);

    if (!isShared)
    {
        PROBE_LOGE("--- end: FAIL — worker texture not visible from probe "
                   "thread; share_context did NOT establish resource "
                   "sharing on this device");
        return -7;
    }

    PROBE_LOGI("--- end: SUCCESS — shared EGL context works on this "
               "device; worker-created texture visible cross-thread");
    return 0;
}
