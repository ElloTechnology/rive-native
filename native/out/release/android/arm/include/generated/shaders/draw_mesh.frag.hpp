#pragma once

#include "draw_mesh.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_mesh_frag[] = R"===(#ifdef EB
#if(defined(K)&&!defined(M))||defined(PB)
#undef gb
#else
#define gb
#endif
K1
#ifndef K
p0(P2,j0);
#endif
#ifndef PB
f1(Q2,e0);
#ifndef K
p0(d6,d4);
#endif
f1(E6,S0);
#else
p0(Q2,e0);
#endif
L1
#ifdef KB
y3 U2(R3,P6,DC);z3 Y4 B4(R3,j8,R5)Z4 K3 L3
#endif
#ifdef K
#ifdef KB
q4(HB)
#else
o2(HB)
#endif
#else
#ifdef KB
M5(HB)
#else
v1(HB)
#endif
#endif
{
#ifdef DB
B(k1,g);B(C2,c);
#endif
#ifdef KB
B(U0,c);
#endif
#ifdef M
B(F3,d);
#endif
#ifdef Z
B(N0,g);
#endif
#if defined(DB)&&defined(FB)
B(Y1,d);
#endif
#ifdef DB
i j=K7(k1,1. R2);d n=clamp(c2(VC,J9,C2,.0).x,G0(.0),G0(1.));
#endif
#ifdef KB
i j=w7(DC,R5,U0,k.cd);d n=1.;
#endif
#ifdef Z
if(Z){d V4=max(c3(X4(N0)),G0(.0));n=min(V4,n);}
#endif
#ifdef gb
v2;
#endif
#if defined(M)
if(M&&F3!=.0){d o3;
#ifndef PB
D O0=unpackHalf2x16(e1(e0));d z6=O0.y;o3=max(z6==F3?O0.x:G0(.0),G0(.0));
#else
o3=H0(e0).x;
#endif
o3=max(o3,G0(.0));n=min(n,o3);}
#endif
#ifdef KB
n*=A0.w4;
#endif
#if!defined(K)
i M1=H0(j0);
#ifdef FB
if(FB){
#ifdef DB
X n2=W5(Y1);
#endif
#ifdef KB
j.xyz=A6(j);X n2=j2(A0.n2);
#endif
if(n2!=K5){j.xyz=R4(j.xyz,M1,n2);}j.w*=n;j.xyz*=j.w;}else
#endif
{j*=n;}
#ifdef VB
if(VB){j=g3(j);}
#endif
j.xyz=M3(j.xyz,T.xy,k.v3,k.w3);
#ifndef PB
j=M1*(1.-j.w)+j;
#endif
C0(j0,j);
#endif
#ifndef PB
X1(e0);X1(S0);
#else
C0(e0,B0(.0));
#endif
#ifdef gb
w2;
#endif
#ifdef K
j=(j*n);j.xyz=M3(j.xyz,T.xy,k.v3,k.w3);m1=j;h3
#else
d2;
#endif
}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive