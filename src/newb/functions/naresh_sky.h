#ifndef NARESH_SKY_H
#define NARESH_SKY_H

#include "detection.h"

// Naresh-style dusk sky/fog. Day and night stay on the existing CraftEdge path.

float nlNrsDayAmt(float sy) {
  return smoothstep(-0.12, 0.28, sy);
}

float nlNrsDuskAmt(float sy) {
  float x = (sy - 0.02)*4.5;
  return exp(-x*x);
}

float nlNrsSkyCurve(float uvy) {
  return pow(saturate(uvy*(16.0/15.0)), 2.3);
}

float nlNrsLuma(vec3 c) {
  return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

float nlNrsHash(vec2 p) {
  vec3 p3 = fract(vec3(p.x, p.y, p.x)*vec3(0.1139, 0.1071, 0.0953));
  p3 += dot(p3, p3.yzx + 31.7);
  return fract((p3.x + p3.y)*p3.z);
}

float nlNrsNoise(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
  f = f*f*(3.0 - 2.0*f);
  float a = nlNrsHash(i);
  float b = nlNrsHash(i + vec2(1.0, 0.0));
  float c = nlNrsHash(i + vec2(0.0, 1.0));
  float d = nlNrsHash(i + vec2_splat(1.0));
  return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float nlNrsFbm(vec2 p, float octaves) {
  float s = 0.0;
  float a = 0.5;
  float w = 0.0;
  for (float i = 0.0; i < 5.0; i++) {
    if (i >= octaves) break;
    s += a*nlNrsNoise(p);
    w += a;
    p = vec2(0.8*p.x - 0.6*p.y, 0.6*p.x + 0.8*p.y)*2.02 + vec2(7.3, 3.1);
    a *= 0.5;
  }
  return s*(0.9375/max(w, 0.0001));
}

vec3 nlNrsSkyBase(vec3 d, vec3 S, float rain) {
  float day = nlNrsDayAmt(S.y);
  float dusk = nlNrsDuskAmt(S.y);

  vec3 zen = mix(vec3(0.003, 0.006, 0.022), vec3(0.16, 0.36, 0.85), day);
  vec3 hor = mix(vec3(0.012, 0.022, 0.055), vec3(0.62, 0.78, 1.0)*1.05, day);
  zen = mix(zen, vec3(0.07, 0.10, 0.30), dusk*0.85);
  hor = mix(hor, vec3(1.25, 0.50, 0.20), dusk*0.85);

  float h = saturate(d.y);
  vec3 col = mix(zen, hor, pow(1.0 - h, 3.2));

  vec2 dxz = d.xz/max(length(d.xz), 0.0001);
  vec2 sxz = S.xz/max(length(S.xz), 0.0001);
  float side = saturate(dot(dxz, sxz)*0.5 + 0.5);
  side *= side;
  float f = dusk*(0.25 + 0.75*side)*pow(1.0 - h, 1.2);
  col = mix(col, vec3(1.3, 0.45, 0.28), f*0.55);

  vec3 grey = mix(vec3(0.26, 0.29, 0.34), vec3(0.50, 0.53, 0.57), pow(1.0 - h, 2.0))*(0.10 + 0.90*day);
  col = mix(col, grey, rain*0.85);

  col = mix(col, col*0.55, saturate(-d.y*3.0));
  return col;
}

vec3 nlNrsTerrainAmbient(vec3 pos, vec3 S, float rain) {
  vec3 d = normalize(vec3(pos.x, 0.03, pos.z));
  vec3 amb = nlNrsSkyBase(d, S, rain) * vec3(1.104, 1.15, 1.219);
  amb = mix(amb, vec3_splat(nlNrsLuma(amb))*vec3(0.96, 1.0, 1.06), rain*0.55);
  amb *= (1.0 + rain)*(1.0 - 0.20*rain);
  return amb;
}

vec3 nlNrsSunLight(vec3 S, float rain) {
  float up = smoothstep(-0.04, 0.16, S.y);
  float warm = smoothstep(0.0, 0.5, S.y);
  vec3 c = mix(vec3(1.0, 0.38, 0.12), vec3(1.1, 0.98, 0.84), warm);
  return c*(3.4*up)*(1.0 - 0.92*rain);
}

vec3 nlNrsSunGlow(vec3 d, vec3 S) {
  float sd = max(dot(d, S), 0.0);
  float up = smoothstep(-0.10, 0.04, S.y);
  float g = pow(sd, 6.0)*0.20 + pow(sd, 48.0)*0.7;
  return vec3(1.0, 0.82, 0.55)*(g*up);
}

vec3 nlNrsSunLayer(vec3 d, vec3 S, float rain) {
  float sd = max(dot(d, S), 0.0);
  float up = smoothstep(-0.10, 0.04, S.y);
  float cr = cos(NL_NARESH_SUN_SIZE);
  float disc = smoothstep(cr - 0.0010, cr + 0.0003, sd);
  return (nlNrsSunGlow(d, S) + vec3(1.0, 0.93, 0.8)*(disc*NL_NARESH_SUN_POWER*up))*(1.0 - rain);
}

vec3 nlNrsMoonLayer(vec3 d, vec3 S, float q, float rain) {
  vec3 M = -S;
  float up = smoothstep(-0.12, 0.10, M.y)*(1.0 - rain);
  if (up <= 0.0) return vec3_splat(0.0);

  float dayfade = 1.0 - 0.85*nlNrsDayAmt(S.y);
  float md = dot(d, M);
  float ang = 1.0 - md;

  float g = exp(-ang/0.0045)*0.9 + exp(-ang/0.03)*0.35 + exp(-ang/0.18)*0.10;
  vec3 col = vec3(0.55, 0.68, 1.0)*(g*NL_NARESH_MOON_GLOW);

  float r = NL_NARESH_MOON_SIZE;
  if (md > cos(r*1.2)) {
    vec3 T = abs(M.y) > 0.999 ? vec3(1.0, 0.0, 0.0) : normalize(cross(vec3(0.0, 1.0, 0.0), M));
    vec3 B = cross(M, T);
    float u = dot(d, T)/r;
    float v = dot(d, B)/r;
    float rr = sqrt(u*u + v*v);
    float inside = 1.0 - smoothstep(0.95, 1.0, rr);

    float mar = nlNrsFbm(vec2(u, v)*2.2 + vec2(3.0, 1.0), q > 0.75 ? 3.0 : 2.0);
    float dark = smoothstep(0.42, 0.62, mar)*0.38;
    float cr = nlNrsFbm(vec2(u, v)*7.0, 2.0);
    dark += smoothstep(0.55, 0.70, cr)*0.10;

    float nz = sqrt(max(1.0 - rr*rr, 0.0));
    float shade = (0.78 + 0.22*nz)*(1.0 - dark);
    col += vec3(0.95, 0.97, 1.05)*(NL_NARESH_MOON_POWER*shade*inside);
  }

  return col*(up*dayfade);
}

vec3 nlNrsHorizonColor(vec3 dir, vec3 S, float rain) {
  vec3 d = normalize(vec3(dir.x, 0.03, dir.z));
  return nlNrsSkyBase(d, S, rain) + nlNrsSunGlow(d, S)*(1.0 - rain);
}

float nlNrsDuskStrength(nl_environment env) {
  return nlNrsDuskAmt(env.dayFactor)*(1.0 - env.rainFactor);
}

#endif
