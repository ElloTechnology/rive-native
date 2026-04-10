/*
 * ThreadedScene binding for Flutter — connects ThreadedScene to
 * MetalTextureRenderer for GPU-accelerated background rendering.
 *
 * All advance+draw work happens on a background C++ thread.
 * The Flutter UI thread only blits the cached GPU texture.
 */

#include "rive_native/external.hpp"
#include "rive/threaded_scene.hpp"
#include "rive/artboard.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/renderer/rive_renderer.hpp"
#include "rive/math/mat2d.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Forward declarations from pls_binding.mm (same translation unit linkage).
class MetalTextureRenderer;
EXPORT bool clear(MetalTextureRenderer* renderer, bool clear, uint32_t color);
EXPORT bool flush(MetalTextureRenderer* renderer, float devicePixelRatio);
EXPORT rive::Renderer* makeRenderer(MetalTextureRenderer* renderer);

namespace rive_flutter
{

/// Wraps a ThreadedScene wired to a MetalTextureRenderer for GPU rendering.
/// The background thread calls clear/draw/flush on the MetalTextureRenderer
/// via the RenderCallback, producing frames into the triple-buffered textures
/// that Flutter composites via TextureLayer.
class ThreadedSceneBinding
{
public:
    static std::unique_ptr<ThreadedSceneBinding> create(
        MetalTextureRenderer* renderer,
        std::unique_ptr<rive::ArtboardInstance> artboard,
        std::unique_ptr<rive::StateMachineInstance> stateMachine,
        rive::rcp<rive::ViewModelInstanceRuntime> viewModelInstance,
        int width,
        int height,
        float devicePixelRatio)
    {
        if (!renderer || !artboard || !stateMachine)
        {
            return nullptr;
        }

        auto binding = std::unique_ptr<ThreadedSceneBinding>(
            new ThreadedSceneBinding());
        binding->m_renderer = renderer;
        binding->m_devicePixelRatio = devicePixelRatio;

        float abWidth = artboard->width();
        float abHeight = artboard->height();

        rive::ThreadedScene::Config config;
        config.width = width;
        config.height = height;
        config.runFirstFrameSync = true;

        // The render callback runs on the background thread.
        // It drives MetalTextureRenderer::begin/draw/end which writes into
        // the triple-buffered GPU textures. Flutter reads the latest via
        // TextureLayer — no pixel copy needed.
        auto* rendererPtr = renderer;
        float dpr = devicePixelRatio;

        binding->m_scene = std::make_unique<rive::ThreadedScene>(
            std::move(artboard),
            std::move(stateMachine),
            config,
            [rendererPtr, abWidth, abHeight, dpr](
                rive::ArtboardInstance* ab,
                int w,
                int h) -> rive::rcp<rive::RenderImage> {
                if (!clear(rendererPtr, true, 0x00000000))
                {
                    return nullptr;
                }
                auto* riveRenderer = makeRenderer(rendererPtr);
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
                flush(rendererPtr, dpr);
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

    MetalTextureRenderer* m_renderer = nullptr;
    std::unique_ptr<rive::ThreadedScene> m_scene;
    float m_devicePixelRatio = 1.0f;
};

} // namespace rive_flutter

// ==========================================================================
// FFI exports — called from Dart via dart:ffi
// ==========================================================================

using namespace rive_flutter;

// --- Lifecycle ---

EXPORT void* riveThreadedCreate(
    void* metalTextureRendererPtr,
    void* artboardPtr,
    void* stateMachinePtr,
    void* viewModelInstancePtr,
    int width,
    int height,
    float devicePixelRatio)
{
    auto* renderer =
        static_cast<MetalTextureRenderer*>(metalTextureRendererPtr);

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
        renderer,
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
