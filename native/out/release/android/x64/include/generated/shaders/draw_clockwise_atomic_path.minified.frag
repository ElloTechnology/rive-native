#ifdef FRAGMENT
K1
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
p0(P2,j0);
#endif
p0(Q2,e0);
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
od(d6,y6);
#endif
L1 K3 Fa(ha,Vd,S0);L3 e void ph(U4(float)l3,d q0,uint T1,g1(uint)p1,g1(d)G3){
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
if(min(l3,q0)>=1.){return;}
#endif
d q;uint Yd=p7(abs(q0));p1=y7(S0,T1,k.W1|Yd);if(p1<k.W1){q=q0;
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
G3=q0;
#endif
}else{
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
if((p1&d7)!=0u){p1=y7(S0,T1,k.W1|d7|Yd);}
#endif
d V1=V5(p1&ka)*ia;d H1=max(V1,q0);q=J8(V1,H1,l3);
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
G3=H1;
#endif
}l3*=q;}e void qh(U4(float)l3,d Q4,uint T1,g1(uint)p1,g1(d)G3){d q=.0;uint bb=p7(abs(Q4));p1=md(S0,T1);
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
if(min(l3,Q4)>=1.&&(p1<k.W1||p1>=(k.W1|h5))){return;}
#endif
if(p1<k.W1){uint Zd=k.W1|(h5+bb);uint a3=y7(S0,T1,Zd);
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
p1=a3;
#endif
if(a3<=k.W1){q=Q4;
#ifdef DRAW_INTERIOR_TRIANGLES
q=min(q,1.);
#endif
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
G3=q;
#endif
Q4=.0;}else if(a3<Zd){uint ae=(a3&ka)-h5;d V1=V5(ae)*ia;d H1=Q4;
#ifdef DRAW_INTERIOR_TRIANGLES
H1=min(H1,1.);
#endif
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
G3=H1;
#endif
q=J8(V1,H1,l3);bb=ae;Q4=V1;}}if(Q4>.0){uint ab=Ga(S0,T1,bb);d V1=Ba(ab);d H1=V1+Q4;V1=clamp(V1,.0,1.);H1=clamp(H1,.0,1.);
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
G3=H1;
#endif
q+=(1.-q*l3)*J8(V1,H1,l3);}l3*=q;}a5(HB){B(k1,g);
#ifdef DRAW_INTERIOR_TRIANGLES
Y(l1,d);
#else
Y(I,z2);
#endif
B(z0,d);
#ifdef ENABLE_CLIPPING
B(S1,D);
#endif
#ifdef ENABLE_CLIP_RECT
B(N0,g);
#endif
#ifdef ENABLE_ADVANCED_BLEND
B(Y1,d);
#endif
B(Z2,W0);B(g4,c);i F0=K7(k1,1. R2);
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
i cb=H0(j0);
#endif
d q0=
#ifdef DRAW_INTERIOR_TRIANGLES
l1;
#else
Ya(I);
#endif
c x6=g4;
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
x6+=(cb.xy+cb.zw)*k.ng;
#endif
x6=floor(x6);uint L7=Z2.y;uint T1=Z2.x+za(W0(x6),L7);d I1=1.;
#ifdef ENABLE_CLIP_RECT
if(ENABLE_CLIP_RECT){d db=c3(X4(N0));I1=min(db,I1);}
#endif
#ifdef ENABLE_CLIPPING
if(ENABLE_CLIPPING&&S1.x!=.0){d eb=H0(e0).x;I1=min(eb,I1);}
#endif
I1=max(I1,.0);q0=clamp(q0,.0,I1);uint p1;float G3;
#ifndef DRAW_INTERIOR_TRIANGLES
if(P5(I)){ph(F0.w,q0,T1,p1,G3);}else
#endif
{qh(F0.w,q0,T1,p1,G3);}
#ifdef ENABLE_DITHER
d E5;if(ENABLE_DITHER){E5=aa(T.xy,k.v3,k.w3);}
#endif
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
if(F0.w>.0){bool rh=p1>=k.W1&&(p1&d7)!=0u;if(!rh){F0.xyz=R4(F0.xyz,cb,W5(Y1));if(G3<1.){r N7=F0.xyz;
#ifdef ENABLE_DITHER
if(ENABLE_DITHER){N7+=E5*k.dd;}
#endif
sg(y6,B0(N7,.0));memoryBarrier();pg(S0,T1,d7);}}else{F0.xyz=rg(y6).xyz;}}
#endif
F0.xyz*=F0.w;
#ifdef ENABLE_DITHER
if(ENABLE_DITHER){F0.xyz+=E5;}
#endif
C0(e0,B0(.0));S3(F0);}
#endif
