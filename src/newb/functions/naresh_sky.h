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

vec3 nlNrsSunGlow(vec3 d, vec3 S) {
  float sd = max(dot(d, S), 0.0);
  float up = smoothstep(-0.10, 0.04, S.y);
  float g = pow(sd, 6.0)*0.20 + pow(sd, 48.0)*0.7;
  return vec3(1.0, 0.82, 0.55)*(g*up);
}

vec3 nlNrsHorizonColor(vec3 dir, vec3 S, float rain) {
  vec3 d = normalize(vec3(dir.x, 0.03, dir.z));
  return nlNrsSkyBase(d, S, rain) + nlNrsSunGlow(d, S)*(1.0 - rain);
}

float nlNrsDuskStrength(nl_environment env) {
  return nlNrsDuskAmt(env.dayFactor)*(1.0 - env.rainFactor);
}

#endif
