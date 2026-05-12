#define e7 -2.
#define Cc -1.5
#define Dc .25
#define B8 1e3
#define Ec (B8*B8)
#ifdef VERTEX
P3 Ub(j3,uf,BC);
#ifdef ENABLE_FEATHER
e6(j3,c7,QC);
#endif
Q3 z4 F4(wc,Jf,LB);H5(rb,xe,UC);I5(sb,ye,OB);F4(xc,Kf,XC);A4
#endif
#if defined(ENABLE_FEATHER)||defined(ATLAS_BLIT)
G4(c7,U9)
#endif
#ifdef FRAGMENT
y3 U2(j3,yc,DD);
#if defined(ENABLE_FEATHER)||defined(ATLAS_BLIT)
e6(j3,c7,QC);
#endif
#ifdef ATLAS_BLIT
i5(j3,zc,VC);
#endif
U2(R3,P6,DC);
#if defined(RENDER_MODE_MSAA)&&defined(ENABLE_ADVANCED_BLEND)&&!defined(FIXED_FUNCTION_COLOR_OUTPUT)
f7(JD);
#endif
z3 G4(yc,xb)
#ifdef ATLAS_BLIT
G4(zc,J9)
#endif
Y4 B4(R3,j8,R5)Z4
#endif
#ifdef FRAGMENT
e bool P5(g J){return J.y>=.0;}e bool P5(D J){return J.y>=.0;}
#endif
#if defined(FRAGMENT)&&defined(ENABLE_FEATHER)
e bool zb(g J){return J.x<Cc;}e bool Ab(g J){return J.y<Cc;}
#endif
#ifdef VERTEX
g Fc(float ma,c C8,float G1){c f6=(1.-C8*abs(G1))*.5;float U3,j5;if(abs(ma-S6)<1./B8){U3=.0;j5=.0;}else{float na=tan(ma);U3=sign(S6-ma)/max(abs(na),1./Ec);j5=U3>=.0?f6.y-(1.-f6.x)*na:f6.y+f6.x*na;}g J;J.x=max(f6.x,.0)+Dc;J.y=-f6.y+e7;J.z=U3;J.w=j5;return J;}
#endif
#ifdef ENABLE_FEATHER
e d a8(g J C3){d U3=J.z;d j5=max(J.w,.0);d g6=U3>=.0?e5(j5):.0;if(abs(U3)<B8){d x=abs(J.x)-Dc;d y=-J.y+e7;d S2=(y-j5)*0.5984134206;i t=j5+S2*B0(0.20888568955,0.62665706865,1.04442844776,1.46219982687);i u=t*-U3+(y*U3+x);i Lf=B0(e5(u[0]),e5(u[1]),e5(u[2]),e5(u[3]));i Gc=t*5.09593080173+-2.54796540086;i Mf=exp2(-Gc*Gc);g6+=dot(Lf,Mf)*S2;}return g6*sign(J.x);}e d r4(g J C3){float g6=1.;float Nf=(1.-e7)+J.x;g6-=e5(Nf);float Of=1.-J.y;g6-=e5(Of);return g6;}
#endif
#if defined(VERTEX)&&defined(DRAW_PATH)
e U k5(int Hc){return U(Hc&((1<<lc)-1),Hc>>lc);}e float Ic(a0 Y0,c Pf){c f2=Z0(Y0,Pf);return(abs(f2.x)+abs(f2.y))*(1./dot(f2,f2));}e bool q9(g g7,g oa,int S,g1(uint)X2,g1(c)Qf
#ifndef RENDER_MODE_MSAA
,g1(g)N1
#else
,g1(X)h7
#endif
h6){int D8=int(g7.x);float G1=g7.y;float pa=g7.z;int Jc=floatBitsToInt(g7.w)>>2;int i7=floatBitsToInt(g7.w)&3;int qa=min(D8,Jc-1);int H4=S*Jc+qa;C4 l5=F1(BC,k5(H4));uint f0=d5(l5.w);uint E8=max(f0&sc,1u);Q ra=P0(XC,E8-1u);c Kc=uintBitsToFloat(ra.xy);X2=ra.z&0xffffu;uint Lc=ra.w;a0 Y0=k2(uintBitsToFloat(P0(LB,X2*4u)));Q I4=P0(LB,X2*4u+1u);c a2=uintBitsToFloat(I4.xy);float G2=uintBitsToFloat(I4.z);float H2=uintBitsToFloat(I4.w);uint Mc=f0&A3;if(Mc!=0u){D8=int(oa.x);G1=oa.y;pa=oa.z;}if(D8!=qa){int Nc=H4+D8-qa;C4 Oc=F1(BC,k5(Nc));if((d5(Oc.w)&(A3|0xffffu))!=(f0&(A3|0xffffu))){bool Rf=G2==.0||Kc.x!=.0;if(Rf){H4=int(Lc);l5=F1(BC,k5(H4));}}else{H4=Nc;l5=Oc;}f0=(d5(l5.w)&~A3)|Mc;}float j1;
#ifdef ENABLE_FEATHER
float j7;float x1;if((f0&T3)==x8&&i7==A8){uint Pc=d5(l5.z);float V3=float(Pc&0xffffu);float g2=float(Pc>>16);U F8=U(-V3-1.,g2-V3+1.);if((f0&A3)!=0u)F8=-F8;C4 Qc=F1(BC,k5(H4+F8.x));C4 sa=F1(BC,k5(H4+F8.y));if((d5(sa.w)&(A3|0xffffu))!=(d5(Qc.w)&(A3|0xffffu))){sa=F1(BC,k5(int(Lc)));}j7=U5(Qc.z);float Rc=U5(sa.z);x1=Rc-j7;if(abs(x1)>x3)x1-=o8*sign(x1);float ta=g2+1.-float(mc);float Sc=clamp(round(abs(x1)/x3*ta),1.,ta-1.);float k7=ta-Sc;if(V3<=k7){x1=-(x3*sign(x1)-x1);g2=k7;if(V3==k7)G1=-G1;}else if(V3==k7+1.){V3=.0;g2=.0;G1=.0;}else{V3-=k7+2.;g2=Sc;}if(V3==g2){j1=Rc;}else{j1=j7+x1*(V3/g2);}}else
#endif
{j1=U5(l5.z);}c T2=c(sin(j1),-cos(j1));c Tc=U5(l5.xy);c G8=c(0,0);if(H2!=.0){H2=max(H2,(fa/3.)/length(Z0(Y0,T2)));}if(G2!=.0){G1*=sign(determinant(Y0));if((f0&z8)!=0u)G1=min(G1,.0);if((f0&rc)!=0u)G1=max(G1,.0);float J4=H2!=.0?H2:Ic(Y0,T2)*i4;d Uc=1.;if(J4>G2&&H2==.0){Uc=j4(G2)/j4(J4);G2=J4;}c m5=T2*(G2+J4);
#ifndef RENDER_MODE_MSAA
float x=G1*(G2+J4);N1.xy=(1./(J4*2.))*(c(x,-x)+G2)+.5;N1.zw=I6(.0);
#endif
uint ua=f0&T3;if(ua>w8){int l7=2;if((f0&ga)==0u)l7=-l7;if((f0&A3)!=0u)l7=-l7;U Sf=k5(H4+l7);C4 Tf=F1(BC,Sf);float Uf=U5(Tf.z);float m7=abs(Uf-j1);if(m7>x3)m7=o8-m7;bool H8=(f0&ga)!=0u;bool Vf=(f0&z8)!=0u;float Vc=m7*(H8==Vf?-.5:.5)+j1;c I8=c(sin(Vc),-cos(Vc));float va=Ic(Y0,I8);float n7=cos(m7*.5);float wa;if((ua==pf)||(ua==qf&&n7>=.25)){float Wf=(f0&y8)!=0u?1.:.25;wa=G2*(1./max(n7,Wf));}else{wa=G2*n7+va*.5;}float xa=wa+va*i4;if((f0&qc)!=0u){float Wc=G2+J4;float Xf=J4*.125;if(Wc<=xa*n7+Xf){float Yf=Wc*(1./n7);m5=I8*Yf;}else{c ya=I8*xa;c Zf=c(dot(m5,m5),dot(ya,ya));m5=Z0(Zf,inverse(a0(m5,ya)));}}c ag=abs(G1)*m5;float Xc=(xa-dot(ag,I8))/(va*(i4*2.));
#ifndef RENDER_MODE_MSAA
if((f0&z8)!=0u)N1.y=Xc;else N1.x=Xc;
#endif
}
#ifndef RENDER_MODE_MSAA
N1.xy*=Uc;N1.y=max(N1.y,1e-4);if(H2!=.0){N1.x=e7-N1.x;}
#endif
G8=Z0(Y0,G1*m5);if(i7!=A8)return false;}else{
#ifndef RENDER_MODE_MSAA
N1=g(pa,-1.,.0,.0);
#ifdef ENABLE_FEATHER
if(H2!=.0){N1.y=e7;N1.z=Ec;N1.w=pa;if((f0&T3)==x8&&i7==A8){if(x1<.0){j7+=x1;x1=-x1;}float W3=j1-j7;W3=mod(W3+S6,o8)-S6;W3=clamp(W3,.0,x1);if(W3>x1*.5){W3=x1-W3;}c C8=c(sin(W3),cos(W3));
#if 0
float O1=1.+.33*log2(S6/(x3-min(x1,x3-x3/16.)));g bg=Fc(x1,C8,.5*(O1/3.));float cg=a8(bg i1);float dg=Xb(cg);float eg=(.5-dg)*(fa*2.);float fg=O1/max(eg,O1);G1*=fg;
#endif
N1=Fc(x1,C8,G1);}G8=Z0(Y0,(G1*H2)*T2);}else
#endif
{G8=sign(Z0(G1*T2,inverse(Y0)))*i4;}if(bool(f0&A3)!=bool(f0&rf)){N1*=g(-1.,+1.,+1.,+1.);}
#endif
if(i7==uc)Tc=Kc;if((f0&pc)!=0u&&i7!=tc){return false;}}Qf=Z0(Y0,Tc)+G8+a2;
#ifdef RENDER_MODE_MSAA
Q K4=P0(LB,X2*4u+2u);h7=j2(K4.x);
#else
N1.xy=mix(N1.xy,c(1.,-1.),Xe(k.gg!=0u));
#endif
return true;}
#endif
#if defined(VERTEX)&&defined(DRAW_INTERIOR_TRIANGLES)
e c pb(V i6,g1(uint)X2
#ifdef RENDER_MODE_MSAA
,g1(X)h7
#else
,g1(d)hg
#endif
h6){X2=floatBitsToUint(i6.z)&0xffffu;
#ifdef RENDER_MODE_MSAA
Q K4=P0(LB,X2*4u+2u);h7=j2(K4.x);
#else
hg=V9(floatBitsToInt(i6.z)>>16);
#endif
c j6=i6.xy;a0 Y0=k2(uintBitsToFloat(P0(LB,X2*4u)));Q I4=P0(LB,X2*4u+1u);c a2=uintBitsToFloat(I4.xy);j6=Z0(Y0,j6)+a2;return j6;}
#endif
#if defined(VERTEX)&&defined(ATLAS_BLIT)
e c ob(V i6,g1(uint)X2,
#ifdef RENDER_MODE_MSAA
g1(X)h7,
#endif
g1(c)ig h6){X2=floatBitsToUint(i6.z)&0xffffu;Q K4=P0(LB,X2*4u+2u);
#ifdef RENDER_MODE_MSAA
h7=j2(K4.x);
#endif
c j6=i6.xy;V o7=uintBitsToFloat(K4.yzw);ig=(j6*o7.x+o7.yz)*k.jg;return j6;}
#endif
e d J8(d V1,d H1,d l3){return(H1-V1)/max(1.-V1*l3,o9);}
#ifdef RENDER_MODE_CLOCKWISE_ATOMIC
#ifdef FIXED_FUNCTION_COLOR_OUTPUT
#define a5 o2
#define S3(n5) m1=n5;h3
#else
#define a5 v1
#define S3(n5) C0(j0,n5);d2;
#endif
e uint za(W0 k6,uint kg){uint Aa=(k6.y>>5u)*(kg<<5u)+(k6.x>>5u)*(32u<<5u);Aa+=((k6.x&0x1fu)>>2u)*(32u<<2u)+((k6.y&0x1fu)>>2u)*(4u<<2u);Aa+=(k6.y&0x3u)*4u+(k6.x&0x3u);return Aa;}e d Ba(uint lg){return V9(int((lg&ka)-h5))*ia;}e uint p7(d n){return uint(n*wf+.5);}
#endif
