#ifndef YSS_PORT_H
#define YSS_PORT_H

// =====================================================================
// YSS port - sunrise/sunset sky colours.
// Ported 1:1 from the YSS pack (src/newb/functions/yss.h):
//   - 5-band sky gradient (SBC -> SLBC -> SLC -> SMC -> SKY)
//   - YSS dawn/dusk/night time factors
// Only the sky palette is used in CraftEdge; lighting, water glint,
// stars, comets, aurora, discs, clouds and fog stay as they were.
// =====================================================================

#include "utils.h"
#include "detection.h"

// linear smoothstep (works with a > b)
float nlYssLinestep(float a, float b, float x) {
  return clamp((x-a)/(b-a), 0.0, 1.0);
}

// YSS time factors. d = env.dayFactor (1 = noon, -1 = midnight)
float nlYssDusk(float d) {
  return nlYssLinestep(1.0, 0.3, d)*smoothstep(-0.3, 0.1, d);
}

float nlYssNight(float d) {
  return nlYssLinestep(0.1, -0.1, d);
}

vec3 nlYssTime3(vec3 day, vec3 dusk, vec3 night, float du, float ni) {
  return mix(mix(day, dusk, du), night, ni);
}

// YSS colours are authored in gamma space
vec3 nlYssToLinear(vec3 c) {
  return pow(max(c, vec3_splat(0.0)), vec3_splat(NL_YSS_GAMMA));
}

// 5-band sky gradient. h = view height (-1..1) in the sky shader frame.
vec3 nlYssSkyGradient(nl_environment env, float h) {
  float du = nlYssDusk(env.dayFactor);
  float ni = nlYssNight(env.dayFactor);

  vec3 c = nlYssTime3(NL_YSS_DAY_SBC, NL_YSS_DUSK_SBC, NL_YSS_NIGHT_SBC, du, ni);
  c = mix(c, nlYssTime3(NL_YSS_DAY_SLBC, NL_YSS_DUSK_SLBC, NL_YSS_NIGHT_SLBC, du, ni), smoothstep(NL_YSS_SKYLINE_P1, NL_YSS_SKYLINE_P2, h));
  c = mix(c, nlYssTime3(NL_YSS_DAY_SLC, NL_YSS_DUSK_SLC, NL_YSS_NIGHT_SLC, du, ni), smoothstep(NL_YSS_SKYLINE_P3, NL_YSS_SKYLINE_P4, h));
  c = mix(c, nlYssTime3(NL_YSS_DAY_SMC, NL_YSS_DUSK_SMC, NL_YSS_NIGHT_SMC, du, ni), smoothstep(NL_YSS_SKYLINE_P5, NL_YSS_SKYLINE_P6, h));

  vec3 top = nlYssTime3(NL_YSS_DAY_SKY, NL_YSS_DUSK_SKY, NL_YSS_NIGHT_SKY, du, ni);
  top = mix(top, NL_YSS_RAIN_SKY, env.rainFactor);
  c = mix(c, top, smoothstep(NL_YSS_SKYLINE_P7, NL_YSS_SKYLINE_P8, h));
  return c;
}

// gradient -> linear with YSS rain desaturation
vec3 nlYssSkyLinear(nl_environment env, float h) {
  vec3 c = nlYssSkyGradient(env, h);
  c = mix(vec3_splat((c.r+c.g+c.b)/3.0), c, 1.0-env.rainFactor);
  return nlYssToLinear(c);
}

#endif
