#ifndef YSS_PORT_H
#define YSS_PORT_H

// =====================================================================
// YSS port - sun/moon disc and fog only.
// The YSS 5-band sky gradient was removed; the sky is HSPE with fixed
// noon / sunrise / sunset accent colours (see sky.h + config.h).
//   - YSS dawn/dusk/night time factors
//   - procedural sun/moon disc + halo (also visible underwater)
//   - YSS time-of-day fog colours + sun glow
// Lighting, water glint, stars, comets, aurora and clouds stay as they were.
// =====================================================================

#include "utils.h"
#include "detection.h"

// linear smoothstep (works with a > b)
float nlYssLinestep(float a, float b, float x) {
  return clamp((x-a)/(b-a), 0.0, 1.0);
}

// smoothstep that is safe for a > b
float nlYssSmooth(float a, float b, float x) {
  float t = clamp((x-a)/(b-a), 0.0, 1.0);
  return t*t*(3.0-2.0*t);
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

// colour of the sun (day/dusk) or moon (night)
vec3 nlYssSunMoonColor(nl_environment env) {
  vec3 c = mix(NL_YSS_DAY_SUNC, NL_YSS_DUSK_SUNC, nlYssDusk(env.dayFactor));
  return mix(c, NL_YSS_MOONC, nlYssNight(env.dayFactor));
}

// the body that lights the world right now (same frame as env.sunDir)
vec3 nlYssMainDir(nl_environment env) {
  return mix(env.sunDir, env.moonDir, step(env.dayFactor, 0.0));
}

// YSS disc + inner halo + big soft glow. `c` must be linear, `vd` uses the
// nlRenderSky frame (y already flipped). YSS mixes these in gamma space.
vec3 nlYssSunMoonOverlay(vec3 c, nl_environment env, vec3 vd) {
  float rainInv = 1.0 - env.rainFactor;
  vec3 smc = nlYssSunMoonColor(env);
  vec3 g = pow(max(c, vec3_splat(0.0)), vec3_splat(1.0/NL_YSS_GAMMA));
  float len = length(vd - nlYssMainDir(env));

  #ifdef NL_YSS_SUNMOON
    g = mix(g, smc, nlYssLinestep(NL_YSS_SUNMOON_DISC_SIZE.x, NL_YSS_SUNMOON_DISC_SIZE.y, len)*rainInv);
  #endif
  g = mix(g, smc, pow(nlYssLinestep(0.45, -0.1, len), 10.0)*rainInv);
  g = mix(g, smc, rainInv*0.3*pow(nlYssLinestep(1.8, 0.0, len), 3.5));

  return nlYssToLinear(g);
}

// YSS fog colour + opacity. Colours are kept in YSS gamma space (YSS does
// the same); edge fade blends towards the linear sky in sky.h.
vec4 nlYssFog(nl_environment env, float relDist, vec3 vd) {
  float df = env.dayFactor;
  float du = nlYssDusk(df);
  float ni = nlYssNight(df);
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

  // glow around the sun
  float len = length(vd - nlYssMainDir(env));
  float g = nlYssSmooth(1.2, 0.0, len);
  fogc += nlYssSunMoonColor(env)*0.6*(1.0-rain)*g*g*g*0.5;

  return vec4(fogc, fogAmt*nlYssLinestep(0.0, fogEnd, relDist));
}

#endif
