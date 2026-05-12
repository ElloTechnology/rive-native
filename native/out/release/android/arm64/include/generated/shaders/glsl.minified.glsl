#define Yb
#ifndef GLSL_VERSION
#define GLSL_VERSION __VERSION__
#endif
#define c vec2
#define V vec3
#define I3 vec3
#define g vec4
#define d mediump float
#define D mediump vec2
#define r mediump vec3
#define i mediump vec4
#define U6 mediump mat3x3
#define V6 mediump mat2x3
#define f5 mediump mat4x4
#define U ivec2
#define Z5 ivec4
#define W0 uvec2
#define Q uvec4
#define X mediump uint
#define E4 bvec2
#define n6 bvec3
#define v7 bvec4
#define a0 mat2
#define e
#define g1(h2) out h2
#define U4(h2) inout h2
#ifdef GL_ANGLE_base_vertex_base_instance_shader_builtin
#extension GL_ANGLE_base_vertex_base_instance_shader_builtin:require
#endif
#ifdef ENABLE_KHR_BLEND
#extension GL_KHR_blend_equation_advanced:require
#endif
#ifdef ATLAS_RENDER_TARGET_R32UI_FRAMEBUFFER_FETCH
#extension GL_EXT_shader_framebuffer_fetch:require
#elif defined(ATLAS_RENDER_TARGET_R8_PLS_EXT)
#extension GL_EXT_shader_pixel_local_storage:require
#elif defined(ATLAS_RENDER_TARGET_R32UI_PLS_ANGLE)
#extension GL_ANGLE_shader_pixel_local_storage:require
#elif defined(ATLAS_RENDER_TARGET_R32I_ATOMIC_TEXTURE)
#ifdef GL_ARB_shader_image_load_store
#extension GL_ARB_shader_image_load_store:require
#endif
#ifdef GL_OES_shader_image_atomic
#extension GL_OES_shader_image_atomic:require
#endif
#endif
#if defined(RENDER_MODE_MSAA)&&defined(ENABLE_CLIP_RECT)&&defined(GL_ES)&&!defined(DISABLE_CLIP_DISTANCE_FOR_UBERSHADERS)
#ifdef GL_EXT_clip_cull_distance
#extension GL_EXT_clip_cull_distance:require
#elif defined(GL_ANGLE_clip_cull_distance)
#extension GL_ANGLE_clip_cull_distance:require
#endif
#endif
#if GLSL_VERSION>=310
#define l6(f,a) layout(binding=f,std140)uniform a{
#else
#define l6(f,a) layout(std140)uniform a{
#endif
#define r7(a) }a;
#define A1(a)
#define r0(f,W,a) layout(location=f)in W a
#define B1
#define v0(M8,G,a,W)
#ifdef VERTEX
#if GLSL_VERSION>=310
#define d0(f,W,a) layout(location=f)out W a
#else
#define d0(f,W,a) out W a
#endif
#else
#if GLSL_VERSION>=310
#define d0(f,W,a) layout(location=f)in W a
#else
#define d0(f,W,a) in W a
#endif
#endif
#define S4 flat
#define i2
#define Z1
#ifdef TARGET_VULKAN
#define J0
#else
#ifdef GL_NV_shader_noperspective_interpolation
#extension GL_NV_shader_noperspective_interpolation:require
#define J0 noperspective
#else
#define J0
#endif
#endif
#ifdef VERTEX
#define P3
#define Q3
#endif
#ifdef FRAGMENT
#define y3
#define z3
#endif
#define Y4
#define Z4
#ifdef TARGET_VULKAN
#define D4(N,f,a) layout(set=N,binding=f)uniform highp utexture2D a
#define c5(N,f,a) layout(set=N,binding=f)uniform highp texture2D a
#define U2(N,f,a) layout(set=N,binding=f)uniform mediump texture2D a
#define i5(N,f,a) layout(binding=f)uniform mediump texture2D a
#if defined(FRAGMENT)&&defined(RENDER_MODE_MSAA)
#endif
#elif GLSL_VERSION>=310
#define D4(N,f,a) layout(binding=f)uniform highp usampler2D a
#define c5(N,f,a) layout(binding=f)uniform highp sampler2D a
#define U2(N,f,a) layout(binding=f)uniform mediump sampler2D a
#define i5(N,f,a) layout(binding=f)uniform mediump sampler2D a
#else
#define D4(N,f,a) uniform highp usampler2D a
#define c5(N,f,a) uniform highp sampler2D a
#define U2(N,f,a) uniform mediump sampler2D a
#define i5(N,f,a) uniform mediump sampler2D a
#endif
#ifdef TARGET_VULKAN
#define G4(o5,a) layout(set=Ac,binding=o5)uniform mediump sampler a;
#define B4(N,f,a) layout(set=N,binding=f)uniform mediump sampler a;
#define p5(a,p,l) texture(sampler2D(a,p),l)
#define c2(a,p,l,X0) textureLod(sampler2D(a,p),l,X0)
#define q5(a,p,l,P1) texture(sampler2D(a,p),l,P1)
#if defined(FRAGMENT)&&defined(RENDER_MODE_MSAA)
#extension GL_OES_sample_variables:require
#endif
#else
#define G4(o5,a)
#define B4(N,f,a)
#define p5(a,p,l) texture(a,l)
#define c2(a,p,l,X0) textureLod(a,l,X0)
#define q5(a,p,l,P1) texture(a,l,P1)
#endif
#define e8(h0,p,l) p5(h0,p,l)
#define P8(h0,p,l,X0) c2(h0,p,l,X0)
#define w7(h0,p,l,P1) q5(h0,p,l,P1)
#define e6(N,f,a) i5(N,f,a)
#define T6(a,p,q,o6,Q8,X0) c2(a,p,c(q,Q8),X0)
#define og(N,f,a) D4(N,f,a)
#define C3
#define i1
#define F1(a,l) texelFetch(a,l,0)
#ifdef TARGET_VULKAN
#elif GLSL_VERSION>=310
#else
#endif
#define z4
#define A4
#define K3
#define L3
#ifdef DISABLE_SHADER_STORAGE_BUFFERS
#define H5(f,y1,a) D4(j3,f,a)
#define F4(f,y1,a) og(j3,f,a)
#define I5(f,y1,a) c5(j3,f,a)
#define P0(a,y0) F1(a,U((y0)&oc,(y0)>>nc))
#define J5(a,y0) F1(a,U((y0)&oc,(y0)>>nc)).xy
#else
#ifdef GL_ARB_shader_storage_buffer_object
#extension GL_ARB_shader_storage_buffer_object:require
#endif
#define H5(f,y1,a) layout(std430,binding=f)readonly buffer y1{W0 a4[];}a
#define F4(f,y1,a) layout(std430,binding=f)readonly buffer y1{Q a4[];}a
#define I5(f,y1,a) layout(std430,binding=f)readonly buffer y1{g a4[];}a
#define Fa(f,y1,a) layout(std430,binding=f)buffer y1{uint a4[];}a
#define P0(a,y0) a.a4[y0]
#define J5(a,y0) a.a4[y0]
#define md(a,y0) a.a4[y0]
#define y7(a,y0,q) atomicMax(a.a4[y0],q)
#define Ga(a,y0,q) atomicAdd(a.a4[y0],q)
#define pg(a,y0,q) atomicOr(a.a4[y0],q)
#endif
#ifdef PLS_IMPL_ANGLE
#extension GL_ANGLE_shader_pixel_local_storage:require
#define K1
#define p0(f,a) layout(binding=f,rgba8)uniform mediump pixelLocalANGLE a
#define f1(f,a) layout(binding=f,r32ui)uniform highp upixelLocalANGLE a
#define L1
#define H0(h) pixelLocalLoadANGLE(h)
#define e1(h) pixelLocalLoadANGLE(h).x
#define C0(h,C) pixelLocalStoreANGLE(h,C)
#define h1(h,C) pixelLocalStoreANGLE(h,uvec4(C))
#define r2(h)
#define X1(h)
#define v2
#define w2
#endif
#ifdef PLS_IMPL_EXT_NATIVE
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#extension GL_EXT_shader_pixel_local_storage2:require
#else
#extension GL_EXT_shader_pixel_local_storage:require
#endif
#define K1 __pixel_localEXT n1{
#define p0(f,a) layout(rgba8)mediump vec4 a
#define R8(f,a) layout(rgb10_a2)mediump vec4 a
#define f1(f,a) layout(r32ui)highp uint a
#define L1 };
#define H0(h) h
#define e1(h) h
#define C0(h,C) h=(C)
#define h1(h,C) h=(C)
#define r2(h) h=h
#define X1(h) h=h
#define v2
#define w2
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#define o2(a) layout(location=0,rgba8)out i m1;v1(a)
#define q4(a) layout(location=0,rgba8)out i m1;v1(a)
#endif
#endif
#ifdef PLS_IMPL_STORAGE_TEXTURE
#ifdef GL_ARB_shader_image_load_store
#extension GL_ARB_shader_image_load_store:require
#endif
#if defined(GL_ARB_fragment_shader_interlock)
#extension GL_ARB_fragment_shader_interlock:require
#define v2 beginInvocationInterlockARB()
#define w2 endInvocationInterlockARB()
#elif defined(GL_INTEL_fragment_shader_ordering)
#extension GL_INTEL_fragment_shader_ordering:require
#define v2 beginFragmentShaderOrderingINTEL()
#define w2
#else
#define v2
#define w2
#endif
#define K1
#ifdef TARGET_VULKAN
#define p0(f,a) layout(set=B3,binding=f,rgba8)uniform mediump coherent image2D a
#define R8(f,a) layout(set=B3,binding=f,rgb10_a2)uniform mediump coherent image2D a
#define f1(f,a) layout(set=B3,binding=f,r32ui)uniform highp coherent uimage2D a
#else
#define p0(f,a) layout(binding=f,rgba8)uniform mediump coherent image2D a
#define R8(f,a) layout(binding=f,rgb10_a2)uniform mediump coherent image2D a
#define f1(f,a) layout(binding=f,r32ui)uniform highp coherent uimage2D a
#endif
#define L1
#define H0(h) imageLoad(h,E)
#define e1(h) imageLoad(h,E).x
#define C0(h,C) imageStore(h,E,C)
#define h1(h,C) imageStore(h,E,uvec4(C))
#define r2(h)
#define X1(h)
#ifndef USING_PLS_STORAGE_TEXTURES
#define USING_PLS_STORAGE_TEXTURES
#endif
#endif
#ifdef PLS_IMPL_SUBPASS_LOAD
#define K1
#define k4(f,a) layout(input_attachment_index=f,binding=f,set=B3)uniform mediump subpassInput z7##a
#define nd(f,a) layout(location=f)out mediump vec4 a
#define p0(f,a) k4(f,a);nd(f,a)
#define f1(f,a) layout(input_attachment_index=f,binding=f,set=B3)uniform highp usubpassInput z7##a;layout(location=f)out highp uvec4 a
#define L1
#define H0(h) subpassLoad(z7##h)
#define e1(h) subpassLoad(z7##h).x
#define C0(h,C) h=(C)
#define h1(h,C) h.x=(C)
#define r2(h) C0(h,subpassLoad(z7##h))
#define X1(h) h1(h,subpassLoad(z7##h).x)
#define v2
#define w2
#endif
#ifdef PLS_IMPL_NONE
#define K1
#define p0(f,a) layout(location=f)out mediump vec4 a
#define f1(f,a) layout(location=f)out highp uvec4 a
#define L1
#define H0(h) vec4(0)
#define e1(h) 0u
#define C0(h,C) h=(C)
#define h1(h,C) h.x=(C)
#define r2(h) h=vec4(0)
#define X1(h) h.x=0u
#define v2
#define w2
#endif
#ifndef k4
#define k4 p0
#endif
#ifdef TARGET_VULKAN
#define gl_VertexID gl_VertexIndex
#endif
#ifdef ENABLE_INSTANCE_INDEX
#ifdef TARGET_VULKAN
#define S8 gl_InstanceIndex
#else
#ifdef BASE_INSTANCE_UNIFORM_NAME
uniform highp int BASE_INSTANCE_UNIFORM_NAME;
#define S8 (gl_InstanceID+BASE_INSTANCE_UNIFORM_NAME)
#else
#define S8 (gl_InstanceID+gl_BaseInstance)
#endif
#endif
#else
#define S8 0
#endif
#define h6
#define p3
#define Z6
#define r5
#define C1(a,c0,G,v,S) void main(){int v=gl_VertexID;int S=S8;
#define Q7 C1
#define D6(a,d3,e3,q3,r3,v) C1(a,d3,e3,v,S)
#define Y(a,W)
#define l0(a)
#define B(a,W)
#define D1(L0) gl_Position=L0;}
#define V2(Q1,a) layout(location=0)out Q1 qg;void main()
#define p6 V2
#define q6 gl_FrontFacing
#define F2(C) qg=C
#define T gl_FragCoord.xy
#define F6
#define R2
#ifdef USING_PLS_STORAGE_TEXTURES
#ifdef TARGET_VULKAN
#define m4(f,a) layout(set=B3,binding=f,r32ui)uniform highp coherent uimage2D a
#define od(f,a) layout(set=B3,binding=f,rgb10_a2)uniform mediump coherent image2D a
#else
#define m4(f,a) layout(binding=f,r32ui)uniform highp coherent uimage2D a
#define od(f,a) layout(binding=f,rgb10_a2)uniform mediump coherent image2D a;
#endif
#define N3(h) imageLoad(h,E).x
#define O3(h,C) imageStore(h,E,uvec4(C))
#define rg(h) imageLoad(h,E)
#define sg(h,C) imageStore(h,E,C)
#define O5(h,q) imageAtomicMax(h,E,q)
#define Q5(h,q) imageAtomicAdd(h,E,q)
#define p4 ,U E
#define U1 ,E
#define v1(a) void main(){U E=ivec2(floor(T));
#define d2 }
#define pd(A7,h,C) if(!(A7)){C0(h,C);}
#define qd(A7,h,C) if(!(A7)){h1(h,C);}
#else
#define p4
#define U1
#define v1(a) void main()
#define d2
#define pd(A7,h,C) C0(h,C);
#define qd(A7,h,C) h1(h,C);
#endif
#define M5(a) v1(a)
#ifndef o2
#define o2(a) layout(location=0)out i m1;v1(a)
#endif
#ifndef q4
#define q4(a) layout(location=0)out i m1;v1(a)
#endif
#define h3 d2
#if defined(TARGET_VULKAN)&&!defined(INPUT_ATTACHMENT_NONE)
#define f7(a) layout(input_attachment_index=0,binding=P2,set=B3)uniform mediump subpassInputMS a
#define T8(a) kc(mat4(subpassLoad(a,0),subpassLoad(a,1),subpassLoad(a,2),subpassLoad(a,3)),gl_SampleMaskIn[0])
#else
#define f7(a) U2(j3,vf,a)
#define T8(a) texelFetch(a,ivec2(floor(T.xy)),0)
#endif
#define Z0(A,F) ((A)*(F))
precision highp float;precision highp int;
#if GLSL_VERSION<310
e i tg(uint u){Q R1=Q(u&0xffu,(u>>8)&0xffu,(u>>16)&0xffu,u>>24);return g(R1)*(1./255.);}
#define unpackUnorm4x8 tg
#endif
