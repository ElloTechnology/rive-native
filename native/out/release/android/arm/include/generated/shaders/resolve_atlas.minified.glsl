#ifdef VERTEX
C1(IF,c0,G,v,S){g O;O.x=(v!=2)?-1.:3.;O.y=(v!=1)?-1.:3.;O.zw=c(.0,1.);D1(O);}
#endif
#ifdef FRAGMENT
e ivec2 zd(){return ivec2(floor(gl_FragCoord));}
#ifdef ATLAS_RENDER_TARGET_R32UI_FRAMEBUFFER_FETCH
layout(location=0)inout Q n0;layout(location=1)out i e4;void main(){e4.x=uintBitsToFloat(n0.x);}
#elif defined(ATLAS_RENDER_TARGET_R8_PLS_EXT)
#ifdef CLEAR_COVERAGE
__pixel_local_outEXT n1{layout(r32f)float n0;};
#else
__pixel_local_inEXT n1{layout(r32f)float n0;};layout(location=0)out i e4;
#endif
void main(){
#ifdef CLEAR_COVERAGE
n0=.0;
#else
e4.x=n0;
#endif
}
#elif defined(ATLAS_RENDER_TARGET_R32UI_PLS_ANGLE)
layout(binding=0,r32ui)uniform highp upixelLocalANGLE n0;layout(location=0)out i e4;void main(){e4.x=uintBitsToFloat(pixelLocalLoadANGLE(n0).x);}
#elif defined(ATLAS_RENDER_TARGET_R32I_ATOMIC_TEXTURE)
layout(binding=0,r32i)uniform highp coherent iimage2D W8;layout(location=0)out i e4;void main(){e4.x=float(imageLoad(W8,zd()).x)*(1./Bc);}
#elif defined(ATLAS_RENDER_TARGET_RGBA8_UNORM)
U2(j3,0,JE);layout(location=0)out i e4;void main(){i J=F1(JE,zd());e4.x=(J.x-J.y)*la+(J.z-J.w)*255.;}
#endif
#endif
