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

#define BG_TAG "RiveBgBinding"
#define BG_LOGI(...) __android_log_print(ANDROID_LOG_INFO, BG_TAG, __VA_ARGS__)
#define BG_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, BG_TAG, __VA_ARGS__)

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
        // runFirstFrameSync=false: do NOT call runOneFrame() synchronously in
        // the ThreadedScene constructor. On some EGL environments (emulator
        // gfxstream, unusual GLES drivers) the synchronous first-frame call
        // crashes (SIGSEGV in StateMachineInstance::advanceAndApply — fault
        // addr 0x39 — before any GL call). With runFirstFrameSync=false the
        // constructor returns immediately and the background thread handles the
        // first frame; if it faults there the process still dies, but the
        // Dart-side gets a chance to check riveThreadedHasFatalError() before
        // the crash propagates to the UI thread. The first visible frame is
        // delayed by one background-thread tick (~16 ms on 60 Hz devices).
        config.runFirstFrameSync = false;

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

                // Impeller-safe state save. The bg worker shares the
                // singleton EGL context with Flutter's render thread (which
                // hosts Impeller's GL backend); whatever GL state we leave
                // here becomes Impeller's starting state on its next
                // eglMakeCurrent. AndroidRenderTexture::endFrame already
                // calls plsGL->unbindGLInternalResources() but only resets
                // Rive's own bindings — viewport, scissor, program, etc.
                // are still whatever Rive's draws left. Snapshot here
                // (after beginFrame has put the context current and Rive
                // has set up its render target) and restore after flush so
                // each bg cycle is state-neutral from Impeller's view.
                GLint savedViewport[4] = {0, 0, 0, 0};
                GLint savedScissor[4] = {0, 0, 0, 0};
                GLint savedActiveTexture = GL_TEXTURE0;
                GLint savedProgram = 0;
                GLint savedFramebuffer = 0;
                GLint savedTextureBinding2D = 0;
                GLboolean savedScissorEnabled = GL_FALSE;
                GLboolean savedBlendEnabled = GL_FALSE;
                GLboolean savedDepthTestEnabled = GL_FALSE;
                glGetIntegerv(GL_VIEWPORT, savedViewport);
                glGetIntegerv(GL_SCISSOR_BOX, savedScissor);
                glGetIntegerv(GL_ACTIVE_TEXTURE, &savedActiveTexture);
                glGetIntegerv(GL_CURRENT_PROGRAM, &savedProgram);
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &savedFramebuffer);
                glGetIntegerv(GL_TEXTURE_BINDING_2D,
                              &savedTextureBinding2D);
                savedScissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
                savedBlendEnabled = glIsEnabled(GL_BLEND);
                savedDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

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
                bool flushOk = flush(renderTexturePtr, dpr);

                // Restore Impeller-relevant state (unconditional — even if
                // flush failed, we want to leave a known state for any
                // future consumer of the context).
                glViewport(savedViewport[0],
                           savedViewport[1],
                           savedViewport[2],
                           savedViewport[3]);
                glScissor(savedScissor[0],
                          savedScissor[1],
                          savedScissor[2],
                          savedScissor[3]);
                glActiveTexture(static_cast<GLenum>(savedActiveTexture));
                glUseProgram(static_cast<GLuint>(savedProgram));
                glBindFramebuffer(GL_FRAMEBUFFER,
                                  static_cast<GLuint>(savedFramebuffer));
                glBindTexture(GL_TEXTURE_2D,
                              static_cast<GLuint>(savedTextureBinding2D));
                if (savedScissorEnabled)
                    glEnable(GL_SCISSOR_TEST);
                else
                    glDisable(GL_SCISSOR_TEST);
                if (savedBlendEnabled)
                    glEnable(GL_BLEND);
                else
                    glDisable(GL_BLEND);
                if (savedDepthTestEnabled)
                    glEnable(GL_DEPTH_TEST);
                else
                    glDisable(GL_DEPTH_TEST);

                if (!flushOk)
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

    // True if EITHER the Android binding marked a fatal EGL/GL failure (via
    // a false return from clear/makeRenderer/flush) OR the underlying
    // ThreadedScene caught an exception thrown out of the render callback.
    // Both indicate the bg worker is permanently stopped; either should
    // trip the Dart-side sync remount.
    bool hasFatalError() const
    {
        if (m_fatalError.load(std::memory_order_acquire))
            return true;
        if (m_scene && m_scene->hasFatalError())
            return true;
        return false;
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

/// Combined snapshot + event drain in a single call. The native side acquires
/// the cached-image mutex once and reads both, so the returned snapshot/events
/// pair always corresponds to the same bg cycle.
///
/// Returns the snapshot property count; the event count is written through
/// outEventCount. Snapshot and event arrays follow the same encoding as
/// riveThreadedAcquireSnapshot and riveThreadedPollEvents respectively.
EXPORT int riveThreadedAcquireFrame(
    void* bindingPtr,
    const char** outPropNames,
    const char** outPropValues,
    int* outPropTypes,
    int maxProperties,
    const char** outEventNames,
    float* outEventDelays,
    int maxEvents,
    int* outEventCount)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (!binding || !binding->scene())
    {
        if (outEventCount)
            *outEventCount = 0;
        return 0;
    }

    thread_local rive::ViewModelSnapshot snapshot;
    thread_local std::vector<rive::ThreadedOutputEvent> events;
    thread_local std::vector<std::string> propValues;
    snapshot.clear();
    events.clear();
    propValues.clear();
    binding->scene()->acquireFrame(snapshot, events);

    int propCount = 0;
    for (const auto& [name, value] : snapshot)
    {
        if (propCount >= maxProperties)
            break;

        outPropNames[propCount] = name.c_str();

        if (std::holds_alternative<std::monostate>(value))
        {
            outPropTypes[propCount] = 0;
            propValues.emplace_back("");
        }
        else if (std::holds_alternative<bool>(value))
        {
            outPropTypes[propCount] = 1;
            propValues.emplace_back(std::get<bool>(value) ? "true" : "false");
        }
        else if (std::holds_alternative<float>(value))
        {
            outPropTypes[propCount] = 2;
            propValues.emplace_back(std::to_string(std::get<float>(value)));
        }
        else if (std::holds_alternative<std::string>(value))
        {
            outPropTypes[propCount] = 3;
            propValues.emplace_back(std::get<std::string>(value));
        }

        outPropValues[propCount] = propValues.back().c_str();
        propCount++;
    }

    int eventCount = static_cast<int>(
        std::min(events.size(), static_cast<size_t>(maxEvents)));
    for (int i = 0; i < eventCount; i++)
    {
        outEventNames[i] = events[i].eventName.c_str();
        outEventDelays[i] = events[i].secondsDelay;
    }
    if (outEventCount)
        *outEventCount = eventCount;
    return propCount;
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

