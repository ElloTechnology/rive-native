#ifdef FRAGMENT
layout(input_attachment_index=0,binding=P2,set=B3)uniform lowp subpassInputMS j9;layout(location=0)out i fb;void main(){fb=(subpassLoad(j9,0)+subpassLoad(j9,1)+subpassLoad(j9,2)+subpassLoad(j9,3))*.25;}
#endif
