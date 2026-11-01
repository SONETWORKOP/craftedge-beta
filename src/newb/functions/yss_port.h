#ifndef YSS_PORT_H
#define YSS_PORT_H

// =====================================================================
// YSS port - sunrise/sunset sky colours, sun/moon lighting and water glint.
// Ported 1:1 from the YSS pack (src/newb/functions/yss.h):
//   - 5-band sky gradient (SBC -> SLBC -> SLC -> SMC -> SKY)
//   - YSS dawn/dusk/night time factors
//   - YSS sun / dusk / moon direct light (already multiplied with PI)
//   - YSS GGX water glint + animated wave normal
// Only the sunrise/sunset pieces are used in CraftEdge; stars, comets,
// aurora, discs, clouds and fog stay as they were.
// =====================================================================

#include "utils.h"
#include "detection.h"

// linear smoothstep (works with a > b)
float nlYssLinestep(float a, float b, float x) {
  return clamp((x-a)/(b-a), 0.0, 1.0);
}

// smoothstep safe for a > b
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

// body that lights the world right now
vec3 nlYssMainDir(nl_environment env) {
  return mix(env.sunDir, env.moonDir, step(env.dayFactor, 0.0));
}

// direct light of sun / dusk sun / moon (already multiplied with PI like YSS)
vec3 nlYssSunMoonLight(nl_environment env) {
  float du = nlYssDusk(env.dayFactor);
  float ni = nlYssNight(env.dayFactor);
  vec3 sun = (NL_YSS_SUN_COL*NL_YSS_SUN_BOOST - NL_YSS_DAY_DIF)*PI;
  vec3 dusk = NL_YSS_DUSKLIGHT*PI;
  vec3 moon = NL_YSS_MOONLIGHT*PI;
  return nlYssTime3(sun, dusk, moon, du, ni);
}

// ---------- YSS noise (water waves) ----------

highp float nlYssRand(highp vec2 x) {
  return fract(cos(x.x + x.y*252.0)*75.0);
}

highp float nlYssNoise(highp vec2 x) {
  highp vec2 a = floor(x);
  highp vec2 b = fract(x);
  highp vec2 c = b*b*(3.0-2.0*b);
  highp float d = nlYssRand(a);
  highp float e = nlYssRand(a+vec2(1.0,0.0));
  highp float f = nlYssRand(a+vec2(0.0,1.0));
  highp float g = nlYssRand(a+vec2(1.0,1.0));
  return mix(mix(d, e, c.x), mix(f, g, c.x), c.y);
}

highp float nlYssWaterHeight(highp vec2 ps, highp float t) {
  ps *= 0.8;
  highp vec2 wp = vec2(ps.x + sin(ps.y + t)*0.5, ps.y + sin(ps.x)*0.5);

  highp float op = pow(nlYssNoise(wp*5.0 + vec2_splat(t*4.0)), 3.0);
  op *= 0.004;

  highp float op1 = nlYssNoise(vec2(wp.x*2.0, wp.y) - vec2_splat(t*1.2));
  op1 = 1.0 - abs(op1 - 0.7)*2.0;
  highp float op2 = nlYssNoise(vec2(wp.x, 0.0) + vec2(t*1.8, 0.0));
  op2 = 1.0 - abs(op2 - 0.7)*2.0;
  op += (op1 + op2)*0.01;
  return op;
}

// upward facing wave normal for a world xz position
vec3 nlYssWaterNormal(highp vec2 p, highp float t) {
  const float d = 0.2;
  float h0 = nlYssWaterHeight(p, t);
  float hx = nlYssWaterHeight(p + vec2(d, 0.0), t);
  float hz = nlYssWaterHeight(p + vec2(0.0, d), t);
  vec2 slope = vec2(hx - h0, hz - h0)*(NL_YSS_WATER_BUMP/d);
  return normalize(vec3(-slope.x, 1.0, -slope.y));
}

// ---------- PBR bits used for the water glint ----------

float nlYssGGX(float NdotH, float roughness) {
  float r2 = roughness*roughness;
  float a = r2*r2;
  float d = (NdotH*a - NdotH)*NdotH + 1.0;
  return a/(PI*d*d);
}

float nlYssGeoSchlick(float x, float roughness) {
  float r = roughness + 1.0;
  float k = (r*r)/8.0;
  return x/(x*(1.0-k) + k);
}

float nlYssFresnel(float cosTheta, float f0, float roughness) {
  return f0 + max(1.0-roughness, f0)*pow(1.0-cosTheta, 5.0);
}

#endif
