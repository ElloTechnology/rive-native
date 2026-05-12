#pragma once

#include "tessellate.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char tessellate[] = R"===(#define Sg 10
#ifdef BB
A1(c0)r0(0,g,ZC);r0(1,g,AD);r0(2,g,OC);
#ifdef P9
r0(3,uint,RD);r0(4,uint,SD);r0(5,uint,TD);r0(6,uint,UD);
#else
r0(3,Q,RB);
#endif
B1
#endif
i2 J0 d0(0,g,v6);J0 d0(1,g,w6);J0 d0(2,g,M4);J0 d0(3,V,A5);S4 d0(4,uint,G7);Z1
#ifdef BB
P3 e6(j3,c7,QC);Q3 G4(c7,U9)z4 F4(wc,Jf,LB);F4(xc,Kf,XC);A4 C1(LF,c0,G,v,S){v0(S,G,ZC,g);v0(S,G,AD,g);v0(S,G,OC,g);
#ifdef P9
v0(S,G,RD,uint);v0(S,G,SD,uint);v0(S,G,TD,uint);v0(S,G,UD,uint);Q RB=Q(RD,SD,TD,UD);
#else
v0(S,G,RB,Q);
#endif
Y(v6,g);Y(w6,g);Y(M4,g);Y(A5,V);Y(G7,uint);c w0=ZC.xy;c x0=ZC.zw;c E0=AD.xy;c I0=AD.zw;bool Bd=v<4;float y=Bd?OC.z:OC.w;int Na=int(Bd?RB.x:RB.y);
#ifdef Yb
int Cd=Na<<16;if(RB.z==0xffffffffu){--Cd;}float X8=float(Cd>>16);
#else
float X8=float(Na<<16>>16);
#endif
float Y8=float(Na>>16);c m2=c((v&1)==0?X8:Y8,(v&2)==0?y+1.:y);if((Y8-X8)*k.ad<.0){m2.y=2.*y+1.-m2.y;}uint N2=RB.z&0x3ffu;uint Dd=(RB.z>>10)&0x3ffu;uint g2=RB.z>>20;uint f0=RB.w;uint E8=f0&sc;uint m0=E8>0u?P0(XC,max(E8,1u)-1u).z:0u;Q I4=m0!=0u?P0(LB,m0*4u+1u):Q(0u,0u,0u,0u);float G2=uintBitsToFloat(I4.z);float H2=uintBitsToFloat(I4.w);if(H2!=.0&&G2==.0){float Ed;float Tg=Oe(w0,x0,E0,I0,Ed);float Oa=H2*(1./fa);float Ug=Je(w0,x0,E0,I0,Ed,Oa);float H7=1.-Ug*(1./x3);float Vg=dot(I0-w0,I0-w0)/(Oa*Oa);float Wg=(Vg-1.)*.5;H7=min(H7,Wg);H7=min(H7,.99);float Xg=.5*H7;float x=Xb(Xg)*-2.+1.;float Fd=i8(x*H2,Tg);g Gd=mix(w0.xyxy,I0.xyxy,g(1./3.,1./3.,2./3.,2./3.));x0=mix(x0,Gd.xy,Fd);E0=mix(E0,Gd.zw,Fd);}if((f0&of)!=0u){a0 Hd=k2(uintBitsToFloat(P0(LB,m0*4u)));c Id=Z0(Hd,-2.*x0+E0+w0);c Jd=Z0(Hd,-2.*E0+I0+x0);float o1=max(dot(Id,Id),dot(Jd,Jd));float J3=max(ceil(sqrt(.75*4.*sqrt(o1))),1.);N2=min(uint(J3),N2);}uint Z8=N2+Dd+g2-1u;a0 E2=M9(w0,x0,E0,I0);float j1=acos(L9(E2[0],E2[1]));float f4=j1/float(Dd);float Pa=determinant(a0(E0-w0,I0-x0));if(Pa==.0)Pa=determinant(E2);if(Pa<.0)f4=-f4;v6=g(w0,x0);w6=g(E0,I0);M4=g(float(Z8)-abs(Y8-m2.x),float(Z8),(g2<<10)|N2,f4);if(g2>1u){a0 Qa=a0(E2[1],OC.xy);float Yg=acos(L9(Qa[0],Qa[1]));float Kd=float(g2);if((f0&(T3|y8))==(w8|y8)){Kd-=2.;}float Ra=Yg/Kd;if(determinant(Qa)<.0)Ra=-Ra;A5.xy=OC.xy;A5.z=Ra;}if(Y8<X8){f0|=A3;}G7=f0;g O=n8(m2,2./lf,k.ad);
#ifdef JC
O.y=-O.y;
#endif
l0(v6);l0(w6);l0(M4);l0(A5);l0(G7);D1(O);}
#endif
#ifdef EB
y3 z3 V2(C4,MF){B(v6,g);B(w6,g);B(M4,g);B(A5,V);B(G7,uint);c w0=v6.xy;c x0=v6.zw;c E0=w6.xy;c I0=w6.zw;a0 E2=M9(w0,x0,E0,I0);float Zg=max(floor(M4.x),.0);float Z8=M4.y;uint Ld=uint(M4.z);float N2=float(Ld&0x3ffu);float g2=float(Ld>>10);float f4=M4.w;uint f0=G7;float N4=Z8-g2;float y2=Zg;if(y2<=N4){f0&=~T3;}else{w0=x0=E0=I0;E2=a0(E2[1],A5.xy);N2=1.;y2-=N4;N4=g2;f4=A5.z;if((f0&T3)>w8){if(y2<2.5)f0|=ga;if(y2>1.5&&y2<3.5)f0|=qc;}else if((f0&y8)!=0u||(f0&T3)==x8){N4-=2.;--y2;}f0|=f4<.0?z8:rc;}c a9;float j1=.0;if(y2==.0||y2==N4||(f0&T3)>w8){bool H8=y2<N4*.5;a9=H8?w0:I0;j1=ac(H8?E2[0]:E2[1]);}else if((f0&pc)!=0u){a9=x0;}else{float w1,B5;if(N2==N4){w1=y2/N2;B5=.0;}else{c A,F,e2=x0-w0;c J6=I0-w0;c f8=E0-x0;F=f8-e2;A=-3.*f8+J6;c ah=F*(N2*2.);c L6=e2*(N2*N2);float c9=.0;float bh=min(N2-1.,y2);c Sa=normalize(E2[0]);float ch=-abs(f4);float dh=(1.+y2)*abs(f4);for(int Ta=Sg-1;Ta>=0;--Ta){float I7=c9+exp2(float(Ta));if(I7<=bh){c Ua=I7*A+ah;Ua=I7*Ua+L6;float eh=dot(normalize(Ua),Sa);float Va=I7*ch+dh;Va=min(Va,x3);if(eh>=cos(Va))c9=I7;}}float fh=c9/N2;float Md=y2-c9;float d9=acos(clamp(Sa.x,-1.,1.));d9=Sa.y>=.0?d9:-d9;j1=Md*f4+d9;c T2=c(sin(j1),-cos(j1));float o=dot(T2,A),e9=dot(T2,F),J1=dot(T2,e2);float gh=max(e9*e9-o*J1,.0);float q2=sqrt(gh);if(e9>.0)q2=-q2;q2-=e9;float Nd=-.5*q2*o;c Wa=(abs(q2*q2+Nd)<abs(o*J1+Nd))?c(q2,o):c(J1,q2);B5=(Wa.y!=.0)?Wa.x/Wa.y:.0;B5=clamp(B5,.0,1.);if(Md==.0)B5=.0;w1=max(fh,B5);}c hh=X5(w0,x0,w1);c Od=X5(x0,E0,w1);c ih=X5(E0,I0,w1);c Pd=X5(hh,Od,w1);c Qd=X5(Od,ih,w1);a9=X5(Pd,Qd,w1);if(w1!=B5)j1=ac(Qd-Pd);}C4 J7;J7.xy=S9(a9);if((f0&T3)==x8){J7.z=T9((uint(N4)<<16)|uint(y2));}else{J7.z=S9(mod(j1,o8));}J7.w=T9(f0);F2(J7);}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive