#ifdef FRAGMENT
K1
#ifndef FIXED_FUNCTION_COLOR_OUTPUT
p0(P2,j0);
#endif
nd(Q2,e0);L1
#ifdef NESTED_CLIP_UPDATE_ONLY
K3 Fa(ha,Vd,S0);L3
#endif
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#define a5 o2
#define S3(n5) m1=n5;h3
#else
#define a5 v1
#define S3(n5) C0(j0,n5);d2;
#endif
a5(HB){
#ifdef DRAW_INTERIOR_TRIANGLES
B(l1,d);d q0=l1;
#else
B(I,z2);d q0=I.x;
#endif
#ifdef NESTED_CLIP_UPDATE_ONLY
if(NESTED_CLIP_UPDATE_ONLY){B(Z2,W0);B(g4,c);uint L7=Z2.y;uint T1=Z2.x+za(W0(floor(g4)),L7);uint p1=md(S0,T1);d Za;if(q0>=1.&&(p1<k.W1||p1>=(k.W1|h5))){Za=.0;}else{d Xd=q0;d h9=q0;if(p1<k.W1){uint M7=k.W1|(h5+p7(abs(q0)));uint a3=y7(S0,T1,M7);if(a3<=k.W1){h9=.0;}else if(a3<M7){h9=Ba(a3);}}if(h9>.0){uint ab=Ga(S0,T1,p7(abs(h9)));Xd=Ba(ab)+q0;}Za=1.-Xd;}C0(e0,B0(Za));S3(B0(1.))}else
#endif
{C0(e0,B0(q0));S3(B0(.0))}}
#endif
