#pragma once

#include "advanced_blend.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char advanced_blend[] = R"===(#ifdef EB
#ifdef VD
layout(
#ifdef SB
blend_support_all_equations
#else
blend_support_multiply,blend_support_screen,blend_support_overlay,blend_support_darken,blend_support_lighten,blend_support_colordodge,blend_support_colorburn,blend_support_hardlight,blend_support_softlight,blend_support_difference,blend_support_exclusion
#endif
)out;
#endif
#ifdef FB
#ifdef SB
d hb(r J1){return dot(J1,T0(.30,.59,.11));}r k9(r ib,r l9){d m9=hb(l9);r n9=ib-hb(ib);D jb=A2(m9,1.0-m9)/max(A2(o9),A2(-c3(n9),G5(n9)));d de=min(G0(1.0),min(jb.x,jb.y));return n9*de+m9;}r kb(r O7,r lb,r l9){float ee=G5(lb)-c3(lb);O7-=c3(O7);float fe=G5(O7);float B2=ee/max(o9,fe);return k9(O7*B2,l9);}
#endif
r ge(r k0,i z1,X p9){r o0=A6(z1);r d1;switch(p9){case he:d1=k0.xyz*o0.xyz;break;case ie:d1=k0.xyz+o0.xyz-k0.xyz*o0.xyz;break;case je:{r B6=k0*o0;d1=2.0*mix(B6,k0+o0-B6-0.5,greaterThan(o0,T0(0.5)));break;}case ke:d1=min(k0.xyz,o0.xyz);break;case le:d1=max(k0.xyz,o0.xyz);break;case me:{z1.xyz=clamp(z1.xyz,T0(.0),z1.www);r mb=clamp(1.-k0,T0(.0),T0(1.))*z1.w;d1=mix(min(T0(1.),z1.xyz/mb),sign(z1.xyz),equal(mb,T0(.0)));break;}case oe:{k0=clamp(k0,T0(.0),T0(1.));z1.xyz=clamp(z1.xyz,T0(.0),z1.www);if(z1.w==.0)z1.w=1.;r nb=z1.w-z1.xyz;d1=1.-mix(min(T0(1.),nb/(k0*z1.w)),sign(nb),equal(k0,T0(.0)));break;}case pe:{r B6=k0*o0;d1=2.0*mix(B6,k0+o0-B6-0.5,greaterThan(k0,T0(0.5)));break;}case qe:{for(int D0=0;D0<3;++D0){if(k0[D0]<=0.5)d1[D0]=(1.0-o0[D0]);else if(o0[D0]<=0.25)d1[D0]=((16.0*o0[D0]-12.0)*o0[D0]+3.0);else d1[D0]=(inversesqrt(o0[D0])-1.0);}d1=o0+o0*(2.0*k0-1.0)*d1;break;}case re:d1=abs(o0.xyz-k0.xyz);break;case se:d1=k0.xyz+o0.xyz-2.*k0.xyz*o0.xyz;break;
#ifdef SB
case te:if(SB){k0.xyz=clamp(k0.xyz,T0(.0),T0(1.));d1=kb(k0.xyz,o0.xyz,o0.xyz);}break;case ue:if(SB){k0.xyz=clamp(k0.xyz,T0(.0),T0(1.));d1=kb(o0.xyz,k0.xyz,o0.xyz);}break;case ve:if(SB){k0.xyz=clamp(k0.xyz,T0(.0),T0(1.));d1=k9(k0.xyz,o0.xyz);}break;case we:if(SB){k0.xyz=clamp(k0.xyz,T0(.0),T0(1.));d1=k9(o0.xyz,k0.xyz);}break;
#endif
}return d1;}e r R4(r k0,i z1,X p9){r d1=ge(k0,z1,p9);return mix(k0,d1,T0(z1.w));}
#endif
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive