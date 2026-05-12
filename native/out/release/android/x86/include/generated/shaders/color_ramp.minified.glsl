#ifdef VERTEX
A1(c0)
#ifdef P9
r0(0,uint,FD);r0(1,uint,GD);r0(2,uint,HD);r0(3,uint,ID);
#else
r0(0,Q,AC);
#endif
B1
#endif
i2 J0 d0(0,i,Q6);Z1
#ifdef VERTEX
P3 Q3 z4 A4 i Re(uint j){return Pb((Q(j,j,j,j)>>Q(16,8,0,24))&0xffu)/255.;}C1(UE,c0,G,v,S){
#ifdef P9
v0(S,G,FD,uint);v0(S,G,GD,uint);v0(S,G,HD,uint);v0(S,G,ID,uint);Q AC=Q(FD,GD,HD,ID);
#else
v0(S,G,AC,Q);
#endif
Y(Q6,i);int m8=v>>1;float x=float(m8<=1?AC.x&0xffffu:AC.x>>16)/65536.;float Q9=(v&1)==0?.0:1.;if(k.Qb<.0){Q9=1.-Q9;}uint R6=AC.y;float y=float(R6&~Se)+Q9;if((R6&Rb)!=0u&&m8==0){if((R6&R9)!=0u)x=.0;else x-=Sb;}if((R6&Tb)!=0u&&m8==3){if((R6&R9)!=0u)x=1.;else x+=Sb;}Q6=Re(m8<=1?AC.z:AC.w);g O=n8(c(x,y),2.,k.Qb);
#ifdef POST_INVERT_Y
O.y=-O.y;
#endif
l0(Q6);D1(O);}
#endif
#ifdef FRAGMENT
y3 z3 V2(i,VE){B(Q6,i);F2(Q6);}
#endif
