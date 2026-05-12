#pragma once

#include "bezier_utils.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char bezier_utils[] = R"===(#ifndef Db
#define Db g
#endif
#ifndef I6
#define I6 c
#endif
e float L9(c o,c b){float He=dot(o,b);float Eb=dot(o,o)*dot(b,b);return(Eb==.0)?1.:clamp(He*inversesqrt(Eb),-1.,1.);}e void Ie(c w0,c x0,c E0,c I0,g1(c)A,g1(c)F,g1(c)e2){e2=x0-w0;c J6=E0-x0;c f8=I0-w0;F=J6-e2;A=-3.*J6+f8;}e a0 M9(c w0,c x0,c E0,c I0){a0 t;t[0]=(any(notEqual(w0,x0))?x0:any(notEqual(x0,E0))?E0:I0)-w0;t[1]=I0-(any(notEqual(I0,E0))?E0:any(notEqual(E0,x0))?x0:w0);return t;}e float Je(c w0,c x0,c E0,c I0,float w1,float Ke){c A,F,e2;Ie(w0,x0,E0,I0,A,F,e2);c K6=3.*(((A*w1)+2.*F)*w1+e2);float Fb=length(K6);if(Fb==.0){return.0;}K6*=1./Fb;float g8=2.*dot(A,K6);float L6=3.*(g8*w1+4.*dot(F,K6))*w1+6.*dot(e2,K6);float N9=min(w1,1.-w1);float Le=(g8*N9*N9+L6)*N9;float Gb=min(Ke,Le*.9999);float S2;if(g8==.0){S2=Gb/L6;}else{float H=1./g8;float b=L6*H,J1=-Gb*H;float M6=(-1./3.)*b,N6=.5*J1;float Hb=N6*N6-M6*M6*M6;if(Hb<.0){float h8=sqrt(M6);float j1=acos(N6/(h8*h8*h8));S2=-2.*h8*cos(j1*(1./3.)+(-x3*2./3.));}else{float A=pow(abs(N6)+sqrt(Hb),1./3.);if(N6<.0)A=-A;S2=A!=.0?A+M6/A:.0;}}S2=abs(S2);g t0011=w1+Db(-S2,-S2,S2,S2);g Ib=(A.xyxy*t0011+2.*F.xyxy)*t0011+e2.xyxy;a0 E2=M9(w0,x0,E0,I0);c Me=t0011.x<1e-3?E2[0]:Ib.xy;c Ne=t0011.z>1.-1e-3?E2[1]:Ib.zw;return acos(L9(Me,Ne));}e float i8(float o,float b){o=b<.0?-o:o;b=abs(b);return o>.0?(o<b?o/b:1.):.0;}float Oe(c w0,c x0,c E0,c I0,g1(float)O9){c Jb=I0-w0;float Kb=length(I0-w0);if(Kb==.0){O9=.5;return.0;}c T2=I6(-Jb.y,Jb.x)/Kb;float Lb=dot(T2,E0-w0);float x4=dot(T2,x0-w0);float y4=x4-Lb;
#if 0
float o=3.*y4;float Mb=y4+x4;float J1=x4;float q2=sqrt(max(y4*y4+Lb*x4,.0));if(Mb<.0)q2=-q2;q2+=Mb;c O6=I6(i8(q2,o),i8(J1,q2));c T5=3.*(O6*(O6*(O6*y4-(x4+y4))+x4));T5=abs(T5);O9=T5.x>T5.y?O6.x:O6.y;return max(T5.x,T5.y);
#else
float Nb=3.*y4;float F=-x4-y4;float e2=x4;float t=.5;for(int D0=0;D0<3;++D0){float Ob=Nb*t;t=i8(Ob*t-e2,2.*(Ob+F));}O9=t;return abs(t*(t*(t*Nb+3.*F)+3.*e2));
#endif
}
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive