#ifdef FRAGMENT
#ifdef DRAW_IMAGE_MESH
y3 U2(R3,P6,DC);
#ifdef ENABLE_ADVANCED_BLEND
f7(JD);
#endif
z3 Y4 B4(R3,j8,R5)Z4
#endif
V2(i,HB){
#ifdef DRAW_IMAGE_MESH
B(U0,c);
#else
B(k1,g);
#ifdef ATLAS_BLIT
B(C2,c);
#endif
#ifdef ENABLE_ADVANCED_BLEND
B(Y1,d);
#endif
#endif
#ifdef DRAW_IMAGE_MESH
i j=w7(DC,R5,U0,k.cd)*A0.w4;
#else
d n=
#ifdef ATLAS_BLIT
clamp(c2(VC,J9,C2,.0).x,G0(.0),G0(1.));
#else
1.;
#endif
i j=K7(k1,n R2);
#endif
#if defined(ENABLE_ADVANCED_BLEND)&&!defined(FIXED_FUNCTION_COLOR_OUTPUT)
#ifdef DRAW_IMAGE_MESH
j.xyz=A6(j);X n2=j2(A0.n2);
#else
X n2=W5(Y1);
#endif
i M1=T8(JD);j.xyz=R4(j.xyz,M1,n2);j.xyz*=j.w;
#elif defined(SPEC_CONST_NONE)&&defined(FIXED_FUNCTION_COLOR_OUTPUT)&&!defined(DRAW_IMAGE_MESH)
j.xyz*=j.w;
#endif
#ifdef NEEDS_GAMMA_CORRECTION
if(NEEDS_GAMMA_CORRECTION){j=g3(j);}
#endif
j.xyz=M3(j.xyz,T.xy,k.v3,k.w3);F2(j);}
#endif
