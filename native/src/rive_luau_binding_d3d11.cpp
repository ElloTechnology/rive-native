// D3D11-specific GPU scripting initialisation.
// Provides riveInitGPUScriptingD3D11() which derives the ore::Context entirely
// from the already-created RenderContext — no extra Dart-side handles needed.
#ifdef RIVE_CANVAS
#include "rive_native/external.hpp"
#include "rive/renderer/render_context.hpp"
#include "rive/renderer/d3d11/render_context_d3d_impl.hpp"
#include "rive/renderer/ore/ore_context_d3d11.hpp"

#include <memory>

static std::unique_ptr<rive::ore::Context> g_ownedOreContext;

// Called once at editor startup (Windows). Returns the created ore::Context
// pointer — Dart should store it and pass to scriptingWorkspaceSetOreContext().
EXPORT void* riveInitGPUScriptingD3D11(void* renderContextPtr)
{
    if (g_ownedOreContext)
        return g_ownedOreContext.get();

    auto* rc = static_cast<rive::gpu::RenderContext*>(renderContextPtr);
    auto* impl = rc->static_impl_cast<rive::gpu::RenderContextD3DImpl>();

    g_ownedOreContext =
        rive::ore::ContextD3D11::Make(impl->gpu(), impl->gpuContext());
    return g_ownedOreContext.get();
}
#endif
