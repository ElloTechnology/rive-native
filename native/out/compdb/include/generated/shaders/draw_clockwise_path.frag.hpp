#pragma once

#include "draw_clockwise_path.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_clockwise_path_frag[] = R"===(#ifdef EB
K1
#ifndef K
p0(P2,j0);
#endif
f1(Q2,e0);
#ifndef K
R8(d6,y6);
#endif
f1(E6,S0);L1
#ifdef K
o2(HB)
#else
v1(HB)
#endif
{B(k1,g);
#ifdef CB
B(l1,d);
#else
B(I,z2);
#endif
B(z0,d);
#ifdef M
B(S1,D);
#endif
#ifdef Z
B(N0,g);
#endif
#ifdef FB
B(Y1,d);
#endif
d q0=
#ifdef CB
l1;
#else
Ya(I);
#endif
i F0;d I1;
#if defined(CB)&&defined(QB)
if(!QB)
#endif
{F0=K7(k1,1. R2);I1=1.;
#ifdef Z
if(Z){d db=c3(X4(N0));I1=min(db,I1);}
#endif
}v2;
#if defined(CB)&&defined(QB)
if(QB){h1(S0,packHalf2x16(A2(q0,z0)));
#ifndef K
r2(j0);
#endif
}else
#endif
{D O4=unpackHalf2x16(e1(S0));d i9=O4.y;d P4=i9==z0?O4.x:G0(.0);d be=
#ifndef CB
P5(I)?max(P4,q0):
#endif
P4+q0;
#ifdef M
if(M&&S1.x!=.0){D O0=unpackHalf2x16(e1(e0));d F5=O0.y;d eb=F5==S1.x?O0.x:G0(.0);I1=min(eb,I1);}
#endif
I1=max(I1,.0);d V1=X9(P4,.0,I1);d H1=X9(be,.0,I1);
#ifdef IB
d E5;if(IB){E5=aa(T.xy,k.v3,k.w3);}
#endif
#ifndef K
i M1=H0(j0);
#ifdef FB
if(FB){if(Y1!=V5(K5)&&H1!=.0){if(V1==.0){F0.xyz=R4(F0.xyz,M1,W5(Y1));
#ifndef CB
if(H1<I1){r N7=F0.xyz;
#ifdef IB
if(IB){N7+=E5*k.dd;}
#endif
C0(y6,B0(N7,0.0));}
#endif
}else{F0.xyz=H0(y6).xyz;r2(y6);}}F0.xyz*=F0.w;}
#endif
#endif
F0*=J8(V1,H1,F0.w);
#ifdef IB
if(IB){F0.xyz+=E5;}
#endif
#ifndef CB
#ifdef FB
#define ce (!FB||Y1==V5(K5))&&F0.w>=1.
#else
#define ce F0.w>=1.
#endif
qd(ce,S0,packHalf2x16(A2(be,z0)));
#else
X1(S0);
#endif
#ifndef K
pd(F0.w==.0,j0,M1*(1.-F0.w)+F0);
#endif
}X1(e0);w2;
#ifdef K
m1=F0;h3
#else
d2;
#endif
}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive