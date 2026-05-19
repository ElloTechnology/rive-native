// ThreadedScene binding for Android GLES — COR-3538 Phase 2 spike.
// Android counterpart of threaded_scene_binding.mm; renders through
// AndroidRenderTexture (rive_native_android.cpp).

#include "rive_native/external.hpp"
#include "rive/threaded_scene.hpp"
#include "rive/artboard.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/layout.hpp"
#include "rive/math/aabb.hpp"
#include "rive/math/mat2d.hpp"
#include "rive/renderer.hpp"
#include "rive/renderer/rive_renderer.hpp"

#include "dart/dart_api_dl.h"

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
extern std::recursive_mutex flutterMutex;
EXPORT bool clear(AndroidRenderTexture* renderTexture,
                  bool clear,
                  uint32_t color);
EXPORT bool flush(AndroidRenderTexture* renderTexture, float devicePixelRatio);
EXPORT rive::Renderer* makeRenderer(AndroidRenderTexture* renderTexture);
EXPORT void* riveThreadedTakeArtboard(void* wrappedArtboardPtr);
EXPORT void* riveThreadedTakeStateMachine(void* wrappedMachinePtr);
EXPORT void* riveThreadedRefViewModelInstance(void* wrappedVMIPtr);
EXPORT void riveThreadedReleaseArtboardWrapper(void* wrappedArtboardPtr);
EXPORT void riveThreadedReleaseStateMachineWrapper(void* wrappedMachinePtr);

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
        float devicePixelRatio,
        rive::Fit fit,
        rive::Alignment alignment)
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
        config.logWarning = [](const std::string& msg) {
            BG_LOGE("%s", msg.c_str());
        };
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
        std::atomic<int64_t>* pendingPort = binding->pendingPortPtr();
        std::atomic<bool>* pendingPosted = binding->pendingPostedPtr();

        binding->m_scene = std::make_unique<rive::ThreadedScene>(
            std::move(artboard),
            std::move(stateMachine),
            config,
            [renderTexturePtr,
             abWidth,
             abHeight,
             dpr,
             fit,
             alignment,
             fatalFlag,
             pausedFlag,
             pendingPort,
             pendingPosted,
             gpuRenderCount = &binding->m_gpuRenderCount,
             consecFailures = &binding->m_consecutiveRenderFailures](
                rive::ArtboardInstance* ab,
                int w,
                int h) -> rive::rcp<rive::RenderImage> {
                if (fatalFlag->load(std::memory_order_acquire))
                {
                    consecFailures->fetch_add(
                        1, std::memory_order_relaxed);
                    return nullptr;
                }
                if (pausedFlag->load(std::memory_order_acquire))
                {
                    // Paused is a healthy state, not a failure — don't bump
                    // consecFailures or renderStalled() would trip every
                    // time the worker is intentionally idle.
                    return nullptr;
                }

                std::unique_lock<std::recursive_mutex> renderLock(
                    flutterMutex);
                if (!clear(renderTexturePtr, true, 0x00000000))
                {
                    BG_LOGE("clear() returned false; marking fatal "
                            "(likely EGL/GL failure, possibly "
                            "EGL_CONTEXT_LOST — see RiveNative logcat)");
                    fatalFlag->store(true, std::memory_order_release);
                    consecFailures->fetch_add(
                        1, std::memory_order_relaxed);
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
                // This captures the state Impeller has proven sensitive to.
                // If corruption persists, audit blend func/equation, write
                // masks, cull/stencil, vertex-array, and buffer bindings.

                auto* riveRenderer = makeRenderer(renderTexturePtr);
                if (!riveRenderer)
                {
                    BG_LOGE(
                        "makeRenderer() returned null; marking fatal "
                        "(AndroidRenderTexture::m_plsRenderer is "
                        "uninitialized — most likely PLS unavailable on "
                        "this GL driver; look for 'Rive Renderer (PLS) "
                        "NOT supported' in earlier logcat)");
                    fatalFlag->store(true, std::memory_order_release);
                    consecFailures->fetch_add(
                        1, std::memory_order_relaxed);
                    return nullptr;
                }
                // `w` / `h` are physical pixels (logical size × devicePixelRatio).
                // computeAlignment() maps the artboard's content box into the
                // frame's pixel box; no additional dpr multiplier is needed.
                rive::AABB frame(0.0f,
                                 0.0f,
                                 static_cast<float>(w),
                                 static_cast<float>(h));
                rive::AABB content(0.0f, 0.0f, abWidth, abHeight);
                rive::Mat2D transform =
                    rive::computeAlignment(fit, alignment, frame, content);

                riveRenderer->save();
                riveRenderer->transform(transform);
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
                    consecFailures->fetch_add(
                        1, std::memory_order_relaxed);
                    return nullptr;
                }
                // Success path — reached only when clear + makeRenderer +
                // flush all succeeded. The texture now holds the painted
                // frame; the rcp return is unused on Android (GPU-direct
                // path), so the upstream ThreadedScene::renderedCount
                // can't see this success. Track it here instead.
                gpuRenderCount->fetch_add(1, std::memory_order_relaxed);
                consecFailures->store(0, std::memory_order_relaxed);

                // Push-not-poll: notify Dart that a new bg cycle produced
                // output. exchange returns the previous value — if it was
                // false we just claimed the post slot; if true a prior post
                // is still pending and Dart hasn't drained yet, so coalesce.
                const int64_t port =
                    pendingPort->load(std::memory_order_acquire);
                if (port != 0 &&
                    !pendingPosted->exchange(true,
                                             std::memory_order_acq_rel))
                {
                    Dart_PostInteger_DL(static_cast<Dart_Port_DL>(port), 1);
                }
                return nullptr; // GPU path — texture IS the output
            },
            std::move(viewModelInstance));

        return binding;
    }

    ~ThreadedSceneBinding()
    {
        m_scene = nullptr;
        releaseTransferredWrappers();
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

    void takeTransferredWrappers(void* wrappedArtboard, void* wrappedMachine)
    {
        m_wrappedArtboard = wrappedArtboard;
        m_wrappedMachine = wrappedMachine;
    }

    // Authoritative "frame painted" counter for Android. The bg render
    // callback always returns nullptr on this platform (GPU-direct path —
    // texture IS the output), so ThreadedScene::renderedCount() is
    // uniformly 0 here. Use this instead.
    uint64_t gpuRenderCount() const
    {
        return m_gpuRenderCount.load(std::memory_order_relaxed);
    }

    // Consecutive bg cycles where the render callback failed (any of
    // clear/makeRenderer/flush returned false, OR a paused/fatal short-
    // circuit hit). Resets to 0 on each successful render.
    uint64_t consecutiveRenderFailures() const
    {
        return m_consecutiveRenderFailures.load(std::memory_order_relaxed);
    }

    // Push-not-poll: register a Dart SendPort native handle. After each bg
    // cycle that produced output (i.e. any cycle reaching the success branch
    // of the render callback), the worker posts an integer once, gated by
    // m_pendingPosted so at most one notification is outstanding at a time.
    // Dart-side clears m_pendingPosted inside riveThreadedAcquireFrame after
    // draining; the next cycle re-arms. port=0 unsubscribes.
    void subscribePendingPort(int64_t port)
    {
        m_pendingPort.store(port, std::memory_order_release);
        m_pendingPosted.store(false, std::memory_order_release);
    }

    void unsubscribePendingPort()
    {
        m_pendingPort.store(0, std::memory_order_release);
    }

    void clearPendingPosted()
    {
        m_pendingPosted.store(false, std::memory_order_release);
    }

    // Accessors for the render-callback lambda capture.
    std::atomic<int64_t>* pendingPortPtr() { return &m_pendingPort; }
    std::atomic<bool>* pendingPostedPtr() { return &m_pendingPosted; }

private:
    ThreadedSceneBinding() = default;

    void releaseTransferredWrappers()
    {
        if (m_wrappedMachine != nullptr)
        {
            riveThreadedReleaseStateMachineWrapper(m_wrappedMachine);
            m_wrappedMachine = nullptr;
        }
        if (m_wrappedArtboard != nullptr)
        {
            riveThreadedReleaseArtboardWrapper(m_wrappedArtboard);
            m_wrappedArtboard = nullptr;
        }
    }

    AndroidRenderTexture* m_renderTexture = nullptr;
    std::unique_ptr<rive::ThreadedScene> m_scene;
    void* m_wrappedArtboard = nullptr;
    void* m_wrappedMachine = nullptr;
    float m_devicePixelRatio = 1.0f;
    std::atomic<bool> m_fatalError{false};
    std::atomic<bool> m_paused{false};

    // GPU-render success tracking. m_renderCallback always returns nullptr
    // on Android, so ThreadedScene::m_renderedCount never bumps. Track real
    // success here: bumped on full clear+makeRenderer+flush OK, consec
    // resets to 0 on success and increments on any failure path.
    std::atomic<uint64_t> m_gpuRenderCount{0};
    std::atomic<uint64_t> m_consecutiveRenderFailures{0};

    // Push-not-poll plumbing. m_pendingPort is the Dart SendPort native
    // handle (0 = unsubscribed). m_pendingPosted gates the post so at most
    // one notification is in flight; cleared by riveThreadedAcquireFrame.
    std::atomic<int64_t> m_pendingPort{0};
    std::atomic<bool> m_pendingPosted{false};
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
    float devicePixelRatio,
    int fit,
    float alignmentX,
    float alignmentY)
{
    auto* renderTexture =
        static_cast<AndroidRenderTexture*>(androidRenderTexturePtr);

    auto* wrappedArtboard = artboardPtr;
    auto* wrappedMachine = stateMachinePtr;

    auto artboard = std::unique_ptr<rive::ArtboardInstance>(
        static_cast<rive::ArtboardInstance*>(
            riveThreadedTakeArtboard(wrappedArtboard)));
    auto stateMachine = std::unique_ptr<rive::StateMachineInstance>(
        static_cast<rive::StateMachineInstance*>(
            riveThreadedTakeStateMachine(wrappedMachine)));

    rive::rcp<rive::ViewModelInstanceRuntime> vmi;
    if (viewModelInstancePtr)
    {
        auto* raw = static_cast<rive::ViewModelInstanceRuntime*>(
            riveThreadedRefViewModelInstance(viewModelInstancePtr));
        vmi = rive::rcp<rive::ViewModelInstanceRuntime>(raw);
    }

    auto binding = ThreadedSceneBinding::create(
        renderTexture,
        std::move(artboard),
        std::move(stateMachine),
        std::move(vmi),
        width,
        height,
        devicePixelRatio,
        static_cast<rive::Fit>(fit),
        rive::Alignment(alignmentX, alignmentY));

    if (!binding)
    {
        return nullptr;
    }

    binding->takeTransferredWrappers(wrappedArtboard, wrappedMachine);
    return binding.release();
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
    stringValues.reserve(
        std::min(snapshot.size(), static_cast<size_t>(maxProperties)));

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
    propValues.reserve(
        std::min(snapshot.size(), static_cast<size_t>(maxProperties)));

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

    // Push-not-poll: re-arm the worker. Cleared AFTER the drain so that a
    // worker post racing with this drain still produces a notification on
    // the next cycle (the worker sees pendingPosted == true and skips its
    // post; on the next cycle pendingPosted is false again and posts).
    binding->clearPendingPosted();

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

// Bg-thread cycle counters. advance bumps every `runOneFrame` (state-machine
// advance + snapshot + event collection ran); rendered bumps only when the
// render callback produced a new RenderImage. Both diverge once damage
// tracking lands or the render callback no-ops (zero-size surface, paused
// worker). Used by the bench harness to expose decoupled "rive frames" vs
// flutter frames in the [BenchFps] log line.
EXPORT uint64_t riveThreadedAdvanceCount(void* bindingPtr)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (!binding || !binding->scene())
        return 0;
    return binding->scene()->advanceCount();
}

EXPORT uint64_t riveThreadedRenderedCount(void* bindingPtr)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (!binding || !binding->scene())
        return 0;
    return binding->scene()->renderedCount();
}

// Real GPU render success count on Android. Bumps each time clear +
// makeRenderer + flush all succeed in the bg-thread render callback. Use
// this in place of riveThreadedRenderedCount on Android — that one counts
// in-memory RenderImage returns from the callback, which the Android
// binding never produces (GPU-direct path; texture is the output, not an
// rcp<RenderImage>). On iOS / other platforms with returning callbacks
// this getter returns 0 (counter is incremented only from the Android
// binding); use riveThreadedRenderedCount there.
EXPORT uint64_t riveThreadedGpuRenderCount(void* bindingPtr)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (!binding)
        return 0;
    return binding->gpuRenderCount();
}

// Number of consecutive bg cycles where the render callback failed
// (fatal short-circuit, or clear/makeRenderer/flush returned false).
// Resets to 0 on each successful render. Paused cycles do NOT count as
// failures. Crossing the threshold used by riveThreadedRenderStalled
// signals the threaded path is silently dead even when hasFatalError is
// still false (most common cause: PLS unsupported on the device's GL
// driver — look for "Rive Renderer (PLS) NOT supported" in logcat).
EXPORT uint64_t riveThreadedConsecutiveRenderFailures(void* bindingPtr)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (!binding)
        return 0;
    return binding->consecutiveRenderFailures();
}

// True after 30+ consecutive failed bg cycles (~500ms at 60Hz). Distinct
// from hasFatalError: a stall doesn't mark the worker as dead (the SM
// keeps advancing) — it just signals nothing is being painted.
EXPORT bool riveThreadedRenderStalled(void* bindingPtr)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (!binding)
        return false;
    return binding->consecutiveRenderFailures() > 30;
}

// --- Push-not-poll ---
//
// One-shot Dart Native API DL initialization. Must be called exactly once per
// process before any subscribePendingPort call. Pass
// `NativeApi.initializeApiDLData` from Dart. Returns 0 on success, -1 on
// version mismatch (Dart_PostInteger_DL would crash otherwise).
EXPORT intptr_t riveThreadedInitDartApiDL(void* data)
{
    return Dart_InitializeApiDL(data);
}

// Register a Dart SendPort (its native handle) so the worker thread posts an
// integer (always 1) after each bg cycle that produced output. Coalesced:
// at most one outstanding notification per binding. Dart-side clears the
// gate inside `riveThreadedAcquireFrame`, so the next cycle re-arms.
//
// Resubscribing while a port is already registered replaces the port and
// clears the posted gate (so the new port gets a fresh notification on the
// next produce cycle).
EXPORT void riveThreadedSubscribePendingPort(void* bindingPtr, int64_t port)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding)
    {
        binding->subscribePendingPort(port);
    }
}

// Stop posting to the previously-registered port. Safe to call multiple
// times. Does not guarantee no in-flight post: a post that started before
// this call may still be delivered to the (now closed) ReceivePort, which
// silently drops it.
EXPORT void riveThreadedUnsubscribePendingPort(void* bindingPtr)
{
    auto* binding = static_cast<ThreadedSceneBinding*>(bindingPtr);
    if (binding)
    {
        binding->unsubscribePendingPort();
    }
}
