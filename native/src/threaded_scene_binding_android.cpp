/*
 * ThreadedScene binding for Flutter — Android GLES.
 *
 * COR-3538 Phase 2 spike. Models on threaded_scene_binding.mm (orphaned at
 * ElloTechnology/rive-native@ello, 378c75b652) but renders through Android's
 * RenderContextGLImpl + RenderTargetGL via the AndroidRenderTexture wrapper
 * already exported by native/platform/android/rive_native_android.cpp.
 *
 * Spike scope only — see specs/2026-05-11-COR-3538-revive-background-rive/
 * spike-results.md for the questions this binding is meant to answer:
 *
 *   1. Can a worker thread drive AndroidRenderTexture's clear/flush/makeRenderer
 *      while the main thread does *no* GL? (Flutter composites the produced
 *      texture via TextureLayer; the main thread never touches GL state for
 *      Rive content directly.)
 *   2. Does a shared EGL context (eglCreateContext with main thread's m_context
 *      as share_context) survive Android lifecycle events (sleep/wake/rotation)?
 *   3. Does Impeller GL state get corrupted by the worker thread's GL usage?
 *
 * Out of spike scope (Phase 3 productionization):
 *   - Error recovery on EGL context loss (EGL_CONTEXT_LOST handling)
 *   - Lifecycle hardening (sleep/wake/rotation/app-background)
 *   - Impeller-safe GL state save/restore around the worker's flush
 *   - Worker-thread EGL config negotiation (currently piggy-backs on main)
 *
 * All advance+draw work happens on a background C++ thread inside
 * ThreadedScene. The Flutter UI thread only composites the cached GPU
 * texture via Flutter's external-texture TextureLayer.
 */

#include "rive_native/external.hpp"
#include "rive/threaded_scene.hpp"
#include "rive/artboard.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/renderer/rive_renderer.hpp"
#include "rive/math/mat2d.hpp"

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Forward declarations from rive_native_android.cpp (same translation unit
// linkage on Android — the GL renderer wrapper is defined there).
class AndroidRenderTexture;
EXPORT bool clear(AndroidRenderTexture* renderTexture,
                  bool clear,
                  uint32_t color);
EXPORT bool flush(AndroidRenderTexture* renderTexture, float devicePixelRatio);
EXPORT rive::Renderer* makeRenderer(AndroidRenderTexture* renderTexture);

namespace rive_flutter
{

/// Wraps a ThreadedScene wired to an AndroidRenderTexture for GLES rendering.
/// The background thread calls clear/draw/flush on the AndroidRenderTexture
/// via the RenderCallback, producing frames into an offscreen texture that
/// Flutter composites via TextureLayer.
///
/// THREADING NOTE (spike question 1):
/// AndroidRenderTexture in rive_native_android.cpp serializes its public
/// methods with a flutterMutex. The render callback inside ThreadedScene
/// runs on the bg thread, so calls from this binding only contend with any
/// other UI-thread caller of clear/flush/makeRenderer. In the bg-mode
/// configuration, the UI thread should not be calling these symbols for the
/// same renderTexture instance — that contract is the spike validation.
///
/// THREADING NOTE (spike question 2):
/// EGLThreadState (the GL context holder in rive_native_android.cpp) is a
/// thread-local singleton constructed on first call to riveFactory() or
/// AndroidRenderTexture::beginFrame(). When the bg thread first invokes
/// `clear` via the render callback, it will construct its OWN EGLThreadState
/// (because thread_local storage is per-thread). That means the bg thread
/// gets a fresh EGL display + context, NOT sharing resources with the UI
/// thread's context. Resources (textures, programs) created on the bg
/// thread are NOT visible to the UI thread's GL context.
///
/// For the spike to actually produce a composited texture, one of two
/// changes is needed (TODO before first device run):
///   (a) Add `share_context` to the bg thread's eglCreateContext call so
///       the bg context shares resources with the UI thread's context. The
///       UI thread context must be accessible — likely requires an FFI
///       accessor on EGLThreadState or a way to look up the main thread's
///       EGLContext at bg-thread initialization time.
///   (b) Switch to producing an AHardwareBuffer / EGLImage on the bg thread
///       and importing it into Flutter's compositor on the UI thread. More
///       work, but isolates the contexts cleanly.
///
/// Easier path for the spike is (a). Hardening lives in Phase 3.
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
        auto* renderTexturePtr = renderTexture;
        float dpr = devicePixelRatio;

        binding->m_scene = std::make_unique<rive::ThreadedScene>(
            std::move(artboard),
            std::move(stateMachine),
            config,
            [renderTexturePtr, abWidth, abHeight, dpr](
                rive::ArtboardInstance* ab,
                int w,
                int h) -> rive::rcp<rive::RenderImage> {
                if (!clear(renderTexturePtr, true, 0x00000000))
                {
                    return nullptr;
                }
                auto* riveRenderer = makeRenderer(renderTexturePtr);
                if (!riveRenderer)
                {
                    return nullptr;
                }
                riveRenderer->save();
                riveRenderer->transform(rive::Mat2D::fromScale(
                    static_cast<float>(w) / abWidth * dpr,
                    static_cast<float>(h) / abHeight * dpr));
                ab->draw(riveRenderer);
                riveRenderer->restore();
                flush(renderTexturePtr, dpr);
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

private:
    ThreadedSceneBinding() = default;

    AndroidRenderTexture* m_renderTexture = nullptr;
    std::unique_ptr<rive::ThreadedScene> m_scene;
    float m_devicePixelRatio = 1.0f;
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
    if (binding && binding->scene())
    {
        binding->scene()->postElapsedTime(dt);
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
