#ifdef VERTEX
A1(c0)r0(0,g,TB);r0(1,g,UB);B1
#endif
i2 J0 d0(0,g,I);Z1
#ifdef VERTEX
C1(HF,c0,G,v,S){v0(v,G,TB,g);v0(v,G,UB,g);Y(I,g);g O;uint m0;c i0;if(q9(TB,UB,S,m0,i0,I p3)){Q K4=P0(LB,m0*4u+2u);V o7=uintBitsToFloat(K4.yzw);i0=i0*o7.x+o7.yz;O=n8(i0,k.bd.x,k.bd.y);
#ifdef POST_INVERT_Y
O.y=-O.y;
#endif
}else{O=g(k.O2,k.O2,k.O2,k.O2);}l0(I);D1(O);}
#endif
#ifdef FRAGMENT
#ifdef ATLAS_FEATHERED_FILL
e d r6(g J,bool Cg C3){d n=a8(J i1);if(!Cg)n=-n;return n;}
#endif
#ifdef ATLAS_RENDER_TARGET_R32UI_FRAMEBUFFER_FETCH
layout(location=0)inout Q n0;
#ifdef ATLAS_FEATHERED_FILL
void main(){float n=uintBitsToFloat(n0.x);n+=r6(I,gl_FrontFacing i1);n0.x=floatBitsToUint(n);}
#endif
#ifdef ATLAS_FEATHERED_STROKE
void main(){float n=uintBitsToFloat(n0.x);n=max(n,r4(I));n0.x=floatBitsToUint(n);}
#endif
#elif defined(ATLAS_RENDER_TARGET_R8_PLS_EXT)
__pixel_localEXT n1{layout(r32f)float n0;};
#ifdef ATLAS_FEATHERED_FILL
void main(){n0+=r6(I,gl_FrontFacing i1);}
#endif
#ifdef ATLAS_FEATHERED_STROKE
void main(){n0=max(n0,r4(I));}
#endif
#elif defined(ATLAS_RENDER_TARGET_R32UI_PLS_ANGLE)
layout(binding=0,r32ui)uniform highp upixelLocalANGLE n0;
#ifdef ATLAS_FEATHERED_FILL
void main(){float n=uintBitsToFloat(pixelLocalLoadANGLE(n0).x);n+=r6(I,gl_FrontFacing i1);pixelLocalStoreANGLE(n0,Q(floatBitsToUint(n)));}
#endif
#ifdef ATLAS_FEATHERED_STROKE
void main(){float n=uintBitsToFloat(pixelLocalLoadANGLE(n0).x);n=max(n,r4(I));pixelLocalStoreANGLE(n0,Q(floatBitsToUint(n)));}
#endif
#elif defined(ATLAS_RENDER_TARGET_R32I_ATOMIC_TEXTURE)
layout(binding=0,r32i)uniform highp coherent iimage2D W8;ivec2 xd(){return ivec2(floor(T));}int yd(float n){return int(n*Bc);}
#ifdef ATLAS_FEATHERED_FILL
void main(){int n=yd(r6(I,gl_FrontFacing i1));imageAtomicAdd(W8,xd(),n);}
#endif
#ifdef ATLAS_FEATHERED_STROKE
void main(){int n=yd(r4(I));imageAtomicMax(W8,xd(),n);}
#endif
#elif defined(ATLAS_RENDER_TARGET_RGBA8_UNORM)
#ifdef ATLAS_FEATHERED_FILL
p6(i,HE){B(I,g);d n=r6(I,q6 i1);if(abs(n)>nf-1e-3){F2(n>.0?B0(.0,.0,1./255.,.0):B0(.0,.0,.0,1./255.));}else{n*=1./la;F2(B0(max(n,.0),max(-n,.0),.0,.0));}}
#endif
#ifdef ATLAS_FEATHERED_STROKE
V2(i,IE){B(I,g);d n=r4(I i1);n*=1./la;F2(B0(n,.0,.0,.0));}
#endif
#else
#ifdef ATLAS_FEATHERED_FILL
p6(float,HE){B(I,g);F2(r6(I,q6 i1));}
#endif
#ifdef ATLAS_FEATHERED_STROKE
V2(float,IE){B(I,g);F2(r4(I i1));}
#endif
#endif
#endif
