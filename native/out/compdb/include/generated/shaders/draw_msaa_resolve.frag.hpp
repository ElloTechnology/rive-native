#pragma once

#include "draw_msaa_resolve.frag.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char draw_msaa_resolve_frag[] = R"===(#ifdef EB
layout(input_attachment_index=0,binding=P2,set=B3)uniform lowp subpassInputMS j9;layout(location=0)out i fb;void main(){fb=(subpassLoad(j9,0)+subpassLoad(j9,1)+subpassLoad(j9,2)+subpassLoad(j9,3))*.25;}
#endif
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive