#pragma once

#include "flush_uniforms.glsl.exports.h"

namespace rive {
namespace gpu {
namespace glsl {
const char flush_uniforms[] = R"===(#ifndef I2
#define I2(X3) float X3;
#endif
#ifndef Y2
#define Y2(X3) uint X3;
#endif
#ifndef Yc
#define Yc(X3) Z5 X3;
#endif
#ifndef q7
#define q7(X3) c X3;
#endif
#ifndef Ca
#define Ca(X3) g X3;
#endif
#ifndef Zc
#define Zc MB
#endif
l6(k3,Zc)I2(Qb)I2(ad)I2(Te)I2(Ue)Y2(m6)Y2(mg)Y2(Fe)Y2(Ge)Yc(S7)q7(jg)q7(bd)Y2(W1)I2(ng)Y2(Y5)I2(O2)I2(cd)Y2(Ae)I2(v3)I2(w3)I2(dd)Y2(gg)r7(k)
)===";
} // namespace glsl
} // namespace gpu
} // namespace rive