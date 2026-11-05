#ifndef YSS_PORT_H
#define YSS_PORT_H

// YSS fog only: time-of-day fog colour and density.
// YSS sky colours, sun/moon disc and sky overlay are intentionally not used.

#include "utils.h"
#include "detection.h"

float nlYssLinestep(float a, float b, float x) {
  return clamp((x-a)/(b-a), 0.0, 1.0);
}

// d = env.dayFactor (1 = noon, -1 = midnight)
float nlYssDusk(float d) {
  return nlYssLinestep(1.0, 0.3, d)*smoothstep(-0.3, 0.1, d);
}

float nlYssNight(float d) {
  return nlYssLinestep(0.1, -0.1, d);
}

vec3 nlYssTime3(vec3 day, vec3 dusk, vec3 night, float du, float ni) {
  return mix(mix(day, dusk, du), night, ni);
}

vec4 nlYssFog(nl_environment env, float relDist) {
  float du = nlYssDusk(env.dayFactor);
  float ni = nlYssNight(env.dayFactor);
  float rain = env.rainFactor;

  float fogEnd = 1.8;
  fogEnd = mix(fogEnd, 1.0, du);
  fogEnd = mix(fogEnd, 0.9, ni);
  fogEnd = mix(fogEnd, 0.7, rain);

  float fogAmt = 0.04;
  fogAmt = mix(fogAmt, 0.15, du);
  fogAmt = mix(fogAmt, 0.1, ni);
  fogAmt = mix(fogAmt, 0.6, rain);

  vec3 fogc = nlYssTime3(NL_YSS_DAY_FOG, NL_YSS_DUSK_FOG, NL_YSS_NIGHT_FOG, du, ni);
  fogc = mix(fogc, NL_YSS_RAIN_FOG, rain);

  return vec4(fogc, fogAmt*nlYssLinestep(0.0, fogEnd, relDist));
}

#endif
