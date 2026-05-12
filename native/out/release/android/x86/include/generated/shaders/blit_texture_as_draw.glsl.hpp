#pragma once

#include "blit_texture_as_draw.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char blit_texture_as_draw[] = R"===(i2
#ifdef WC
J0 d0(0,c,U0);
#endif
Z1
#ifdef BB
P3 Q3 z4 A4 A1(c0)B1 C1(TE,c0,G,v,S){c m2;m2.x=(v&1)==0?-1.:1.;m2.y=(v&2)==0?-1.:1.;
#ifdef WC
Y(U0,c);U0.x=m2.x*.5+.5;U0.y=m2.y*-.5+.5;l0(U0);
#endif
g O=g(m2,0,1);D1(O);}
#endif
#ifdef EB
y3
#ifdef ED
Pe(R3,P6,ZB);
#else
U2(R3,P6,ZB);
#endif
z3
#ifdef WC
Y4 B4(R3,j8,Qe)Z4
#endif
V2(i,YD){i k8;
#ifdef WC
B(U0,c);k8=c2(ZB,Qe,U0,.0);
#elif defined(ED)
k8=(l8(ZB,0,U(floor(T.xy)))+l8(ZB,1,U(floor(T.xy)))+l8(ZB,2,U(floor(T.xy)))+l8(ZB,3,U(floor(T.xy))))*0.25;
#else
k8=F1(ZB,U(floor(T.xy)));
#endif
F2(k8);}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive