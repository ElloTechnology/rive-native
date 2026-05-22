// Metal-specific GPU scripting initialisation.
// Provides riveInitGPUScriptingMetal() which derives the ore::Context from the
// already-created RenderContext + MTLCommandQueue. Returns the ore::Context
// pointer — Dart stores it and passes to scriptingWorkspaceSetOreContext().
// The render context is set separately via
// scriptingWorkspaceSetRenderContext().
#ifdef RIVE_CANVAS
#include "rive_native/external.hpp"
#include "rive/renderer/render_context.hpp"

#include "rive/renderer/metal/render_context_metal_impl.h"
#include "rive/renderer/ore/ore_context_metal.hpp"

#import <Metal/Metal.h>
#include <memory>

// Process-lifetime ore::Context ownership. Created once by
// riveInitGPUScriptingMetal() and kept alive for the duration of the process.
static std::unique_ptr<rive::ore::Context> g_ownedOreContext;

// Returns a pointer to the created ore::Context (owned by this translation
// unit for the lifetime of the process). Dart should store this pointer and
// pass it to scriptingWorkspaceSetOreContext() before calling requestVM.
EXPORT void* riveInitGPUScriptingMetal(void* renderContextPtr,
                                       void* queueBridged)
{
    if (g_ownedOreContext)
        return g_ownedOreContext.get();

    if (renderContextPtr == nullptr || queueBridged == nullptr)
        return nullptr;

    auto* rc = static_cast<rive::gpu::RenderContext*>(renderContextPtr);
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)queueBridged;

    auto* impl = rc->static_impl_cast<rive::gpu::RenderContextMetalImpl>();
    if (impl == nullptr)
        return nullptr;

    g_ownedOreContext = rive::ore::ContextMetal::Make(impl->gpu(), queue);
    return g_ownedOreContext.get();
}
#endif
