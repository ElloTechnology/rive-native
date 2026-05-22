// GPU-canvas and frame-boundary exports for Dart FFI.
// Split out from rive_luau_binding.cpp because ore_context.hpp pulls in
// Metal headers via #import on Apple, which is invalid in a plain .cpp file.
#ifdef RIVE_CANVAS
#include "rive_native/external.hpp"
#include "rive/lua/rive_lua_libs.hpp"
#include "rive/renderer/render_context.hpp"
#include "rive/renderer/render_canvas.hpp"
#ifdef RIVE_ORE
#include "rive/renderer/ore/ore_context.hpp"
#endif

using namespace rive;

// ---------------------------------------------------------------------------
// riveLuaPushCanvas — create a 2D Canvas, push onto Lua stack.
// ---------------------------------------------------------------------------
EXPORT int riveLuaPushCanvas(lua_State* L, uint32_t width, uint32_t height)
{
    auto* scriptingCtx = static_cast<ScriptingContext*>(lua_getthreaddata(L));
    auto* renderCtx =
        static_cast<gpu::RenderContext*>(scriptingCtx->renderContext());
    if (renderCtx == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    auto canvas = renderCtx->makeRenderCanvas(width, height);
    if (!canvas)
    {
        lua_pushnil(L);
        return 1;
    }
    auto* handle = lua_newrive<ScriptedCanvas>(L);
    handle->m_L = L;
    handle->canvas = std::move(canvas);
    handle->renderCtx = renderCtx;
    // Create a ScriptedImage backed by canvas->renderImage() so
    // the script can composite it with renderer:drawImage()
    auto* img = lua_newrive<ScriptedImage>(L);
    img->image =
        ref_rcp(static_cast<RenderImage*>(handle->canvas->renderImage()));
    handle->m_imageRef = lua_ref(L, -1);
    lua_pop(L, 1); // pop image, handle remains on top
    return 1;
}

// ---------------------------------------------------------------------------
// riveLuaPushGPUCanvas — create a GPUCanvas, push onto Lua stack.
// ---------------------------------------------------------------------------
EXPORT int riveLuaPushGPUCanvas(lua_State* L, uint32_t width, uint32_t height)
{
#ifdef RIVE_ORE
    auto* scriptingCtx = static_cast<ScriptingContext*>(lua_getthreaddata(L));
    auto* renderCtx =
        static_cast<gpu::RenderContext*>(scriptingCtx->renderContext());
    auto* oreCtx = static_cast<ore::Context*>(scriptingCtx->oreContext());
    if (renderCtx == nullptr || oreCtx == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    auto canvas = renderCtx->makeRenderCanvas(width, height);
    if (!canvas)
    {
        lua_pushnil(L);
        return 1;
    }
    auto colorView = oreCtx->wrapCanvasTexture(canvas.get());
    if (!colorView)
    {
        lua_pushnil(L);
        return 1;
    }

    auto* handle = lua_newrive<ScriptedGPUCanvas>(L);
    handle->m_L = L;
    handle->canvas = std::move(canvas);
    handle->oreColorView = std::move(colorView);
    handle->renderCtx = renderCtx;

    auto* img = lua_newrive<ScriptedImage>(L);
    img->image =
        ref_rcp(static_cast<RenderImage*>(handle->canvas->renderImage()));
    handle->m_imageRef = lua_ref(L, -1);
    lua_pop(L, 1); // pop image; handle remains on top
    return 1;
#else
    (void)width;
    (void)height;
    lua_pushnil(L);
    return 1;
#endif
}

// ---------------------------------------------------------------------------
// riveLuaContextBeginRenderPass — wire context:beginRenderPass(desc) from
// the Dart-side context builder (editor path) into the C++ implementation
// shared with the runtime context's namecall dispatch.
// ---------------------------------------------------------------------------
#ifdef RIVE_ORE
extern int context_beginrenderpass(lua_State* L);

EXPORT int riveLuaContextBeginRenderPass(lua_State* L)
{
    return context_beginrenderpass(L);
}
#else
EXPORT int riveLuaContextBeginRenderPass(lua_State* L)
{
    luaL_error(L, "context:beginRenderPass() requires a RIVE_ORE build");
    return 0;
}
#endif

// ---------------------------------------------------------------------------
// riveGPUBeginFrame / riveGPUEndFrame — frame boundaries for GPU scripting.
// ---------------------------------------------------------------------------
EXPORT void riveGPUBeginFrame(lua_State* L)
{
#ifdef RIVE_ORE
    auto* scriptingCtx = static_cast<ScriptingContext*>(lua_getthreaddata(L));
    auto* ctx = static_cast<ore::Context*>(scriptingCtx->oreContext());
    if (ctx)
        ctx->beginFrame();
#endif
}

EXPORT void riveGPUEndFrame(lua_State* L)
{
#ifdef RIVE_ORE
    auto* scriptingCtx = static_cast<ScriptingContext*>(lua_getthreaddata(L));
    auto* ctx = static_cast<ore::Context*>(scriptingCtx->oreContext());
    if (!ctx)
        return;

    // Safety net: auto-finish any render pass the Lua script left open.
    if (ctx->activeRenderPass() && !ctx->activeRenderPass()->isFinished())
    {
        ctx->activeRenderPass()->finish();
        ctx->setActiveRenderPass(nullptr);
    }

    ctx->endFrame();
#endif
}
#endif // RIVE_CANVAS
