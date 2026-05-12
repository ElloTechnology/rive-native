#ifdef FRAGMENT
#if(defined(FIXED_FUNCTION_COLOR_OUTPUT)&&!defined(ENABLE_CLIPPING))||defined(RENDER_MODE_CLOCKWISE_ATOMIC)
#undef gb
#else
#define gb
#endif
K1
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
p0(P2,j0);
#endif
#ifndef RENDER_MODE_CLOCKWISE_ATOMIC
f1(Q2,e0);
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
p0(d6,d4);
#endif
f1(E6,S0);
#else
p0(Q2,e0);
#endif
L1
#ifdef DRAW_IMAGE_MESH
y3 U2(R3,P6,DC);z3 Y4 B4(R3,j8,R5)Z4 K3 L3
#endif
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#ifdef DRAW_IMAGE_MESH
q4(HB)
#else
o2(HB)
#endif
#else
#ifdef DRAW_IMAGE_MESH
M5(HB)
#else
v1(HB)
#endif
#endif
{
#ifdef ATLAS_BLIT
B(k1,g);B(C2,c);
#endif
#ifdef DRAW_IMAGE_MESH
B(U0,c);
#endif
#ifdef ENABLE_CLIPPING
B(F3,d);
#endif
#ifdef ENABLE_CLIP_RECT
B(N0,g);
#endif
#if defined(ATLAS_BLIT)&&defined(ENABLE_ADVANCED_BLEND)
B(Y1,d);
#endif
#ifdef ATLAS_BLIT
i j=K7(k1,1. R2);d n=clamp(c2(VC,J9,C2,.0).x,G0(.0),G0(1.));
#endif
#ifdef DRAW_IMAGE_MESH
i j=w7(DC,R5,U0,k.cd);d n=1.;
#endif
#ifdef ENABLE_CLIP_RECT
if(ENABLE_CLIP_RECT){d V4=max(c3(X4(N0)),G0(.0));n=min(V4,n);}
#endif
#ifdef gb
v2;
#endif
#if defined(ENABLE_CLIPPING)
if(ENABLE_CLIPPING&&F3!=.0){d o3;
#ifndef RENDER_MODE_CLOCKWISE_ATOMIC
D O0=unpackHalf2x16(e1(e0));d z6=O0.y;o3=max(z6==F3?O0.x:G0(.0),G0(.0));
#else
o3=H0(e0).x;
#endif
o3=max(o3,G0(.0));n=min(n,o3);}
#endif
#ifdef DRAW_IMAGE_MESH
n*=A0.w4;
#endif
#if!defined(FIXED_FUNCTION_COLOR_OUTPUT)
i M1=H0(j0);
#ifdef ENABLE_ADVANCED_BLEND
if(ENABLE_ADVANCED_BLEND){
#ifdef ATLAS_BLIT
X n2=W5(Y1);
#endif
#ifdef DRAW_IMAGE_MESH
j.xyz=A6(j);X n2=j2(A0.n2);
#endif
if(n2!=K5){j.xyz=R4(j.xyz,M1,n2);}j.w*=n;j.xyz*=j.w;}else
#endif
{j*=n;}
#ifdef NEEDS_GAMMA_CORRECTION
if(NEEDS_GAMMA_CORRECTION){j=g3(j);}
#endif
j.xyz=M3(j.xyz,T.xy,k.v3,k.w3);
#ifndef RENDER_MODE_CLOCKWISE_ATOMIC
j=M1*(1.-j.w)+j;
#endif
C0(j0,j);
#endif
#ifndef RENDER_MODE_CLOCKWISE_ATOMIC
X1(e0);X1(S0);
#else
C0(e0,B0(.0));
#endif
#ifdef gb
w2;
#endif
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
j=(j*n);j.xyz=M3(j.xyz,T.xy,k.v3,k.w3);m1=j;h3
#else
d2;
#endif
}
#endif
