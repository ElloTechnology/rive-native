#pragma once

#include "common.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char common[] = R"===(#define x3 3.14159265359
#define o8 6.28318530718
#define S6 1.57079632679
#ifndef AB
#define i4 float(.5)
#else
#define i4 float(.0)
#endif
#define H3(l) n8(l,k.Te,k.Ue)
#ifdef WE
#define Ub(N,f,a) c5(N,f,a)
#define C4 g
#define S9(q) q
#define U5(q) q
#define T9(q) uintBitsToFloat(q)
#define d5(q) floatBitsToUint(q)
#else
#define Ub(N,f,a) D4(N,f,a)
#define C4 Q
#define S9(q) floatBitsToUint(q)
#define U5(q) uintBitsToFloat(q)
#define T9(q) q
#define d5(q) q
#endif
#define Ve(a,l,p8) F1(a,U(l)+U(-1,0))p8,F1(a,U(l)+U(0,0))p8,F1(a,U(l)+U(0,-1))p8,F1(a,U(l)+U(-1,-1))p8
#define e5(q) T6(QC,U9,q,Vb,float(Vb),.0).x
#define Xb(q) T6(QC,U9,q,Wb,float(Wb),.0).x
#ifdef Yb
e d j4(float x){return x;}e d V5(uint x){return float(x);}e d We(X x){return float(x);}e d V9(int x){return float(x);}e i X4(g xyzw){return xyzw;}e D P7(c xy){return xy;}e i Pb(Q xyzw){return vec4(xyzw);}e X W5(d x){return uint(x);}e X j2(uint x){return x;}
#else
e d j4(float x){return(d)x;}e d V5(uint x){return(d)x;}e d We(X x){return(d)x;}e d V9(int x){return(d)x;}e i X4(g xyzw){return(i)xyzw;}e D P7(c xy){return(D)xy;}e i Pb(Q xyzw){return(i)xyzw;}e X W5(d x){return(X)x;}e X j2(uint x){return(X)x;}
#endif
e d G0(d x){return x;}e D A2(D xy){return xy;}e D A2(d x,d y){D L;L.x=x,L.y=y;return L;}e D A2(d x){D L;L.x=x,L.y=x;return L;}e c I6(float x){return c(x,x);}e r T0(d x,d y,d z){r L;L.x=x,L.y=y,L.z=z;return L;}e r T0(d x){r L;L.x=x,L.y=x,L.z=x;return L;}e i B0(d x,d y,d z,d w){i L;L.x=x,L.y=y,L.z=z,L.w=w;return L;}e i B0(r xyz,d w){i L;L.xyz=xyz;L.w=w;return L;}e i B0(d x){i L;L.x=x,L.y=x,L.z=x,L.w=x;return L;}e i B0(i x){return x;}e E4 Xe(bool b){return E4(b,b);}e U6 wh(r o,r b,r J1){U6 L;L[0]=o;L[1]=b;L[2]=J1;return L;}e V6 xh(r o,r b){V6 L;L[0]=o;L[1]=b;return L;}e f5 yh(i o,i b,i J1,i Ye){f5 L;L[0]=o;L[1]=b;L[2]=J1;L[3]=Ye;return L;}e a0 k2(g x){return a0(x.xy,x.zw);}e uint Cb(X x){return x;}e c X5(c o,c b,float t){return(b-o)*t+o;}e d q8(uint Zb,uint Y5){return Zb==0u?.0:unpackHalf2x16((Zb+Ze)*Y5).x;}e float ac(c f2){f2=normalize(f2);float j1=acos(clamp(f2.x,-1.,1.));return f2.y>=.0?j1:-j1;}e i zh(i j){return B0(j.xyz*j.w,j.w);}e r A6(i W9){return W9.xyz*(W9.w!=.0?1./W9.w:.0);}e d c3(D W6){return min(W6.x,W6.y);}e d c3(r bc){return min(c3(bc.xy),bc.z);}e d c3(i cc){D W6=min(cc.xy,cc.zw);d af=min(W6.x,W6.y);return af;}e d G5(D X6){return max(X6.x,X6.y);}e d G5(r dc){return max(G5(dc.xy),dc.z);}e d G5(i ec){D X6=max(ec.xy,ec.zw);d bf=max(X6.x,X6.y);return bf;}e float x9(c x){return abs(x.x)+abs(x.y);}e d X9(d x,d Y9,d Z9){
#if defined(XE)||defined(RC)
#ifdef RC
if(RC==cf)
#endif
{if(x<Z9)if(x>Y9)return x;else return Y9;else return Z9;}
#endif
return clamp(x,Y9,Z9);}e d fc(c K0,d B2,d i3){d df=fract(0.06711056*K0.x+0.00583715*K0.y);d ef=fract(52.9829189*df);return(ef*B2)+i3;}
#if 0
e d Ah(c K0,float B2,float i3){int x=int(K0.x);int y=int(K0.y);int gc=(x^y);int b=(y>>1)&1;b|=(gc&2);b|=(y&1)<<2;b|=(gc&1)<<3;float ff=float(b);d gf=j4(ff)/16.0;return(gf*B2)+i3;}e d Bh(c K0,float B2,float i3){K0.y*=0.5;K0.x=fract(K0.x*0.5+K0.y);K0.y=fract(K0.y);float J3=(K0.y*0.5+K0.x);return(J3*B2)+i3;}
#endif
#ifdef IB
e d aa(c K0,d B2,d i3){return IB?fc(K0,B2,i3):.0;}e r M3(r j,c K0,d B2,d i3){return IB?(fc(K0,B2,i3)+j):j;}
#else
e d aa(c K0,float B2,float i3){return 0.;}e r M3(r j,c K0,d B2,d i3){return j;}
#endif
#ifdef BB
e g n8(c hc,float hf,float ic){return g(hc.x*hf-1.,hc.y*ic-sign(ic),0.,1.);}
#ifndef AB
e g R7(a0 l2,c D2,c ba){c ca=abs(l2[0])+abs(l2[1]);if(ca.x!=.0&&ca.y!=.0){c H=1./ca;c g5=Z0(l2,ba)+D2;const float jf=.5;return g(g5,-g5)*H.xyxy+H.xyxy+jf;}else{return D2.xyxy;}}
#else
e float da(uint Y6){return 1.-float(Y6)*(2./32768.);}
#ifdef Z
e void jc(a0 l2,c D2,c ba Z6){
#ifndef ZD
if(any(notEqual(g(l2),g(.0,.0,.0,.0)))){c g5=Z0(l2,ba)+D2.xy;gl_ClipDistance[0]=g5.x+1.;gl_ClipDistance[1]=g5.y+1.;gl_ClipDistance[2]=1.-g5.x;gl_ClipDistance[3]=1.-g5.y;}else{gl_ClipDistance[0]=gl_ClipDistance[1]=gl_ClipDistance[2]=gl_ClipDistance[3]=D2.x-.5;}
#endif
}
#endif
#endif
#endif
#ifdef EB
#ifdef VB
e d g3(d j){return(j<=0.04045)?j/12.92:pow(abs((j+0.055)/1.055),2.4);}e r g3(r j){return T0(g3(j.x),g3(j.y),g3(j.z));}e i g3(i j){return B0(g3(j.xyz),j.w);}
#endif
#endif
#if defined(EB)&&defined(AB)&&!defined(K)
e i kc(f5 a7,int r8){if(r8==0xf){return(a7[0]+a7[1]+a7[2]+a7[3])*.25;}else{i kf=g(notEqual(r8&Z5(1,2,4,8),Z5(0)));i L=Z0(a7,kf);int v8=(r8&5)+((r8>>1)&5);v8=(v8&3)+(v8>>2);L*=1./float(v8);return L;}}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive