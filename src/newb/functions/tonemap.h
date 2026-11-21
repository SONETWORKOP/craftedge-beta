#ifndef TONEMAP_H
#define TONEMAP_H

#include "utils.h"

// Naresh hybrid tonemap (MIT): github.com/bamdaniyanaresh-png/naresh-renderdragon
vec3 nrsCurveSoft(vec3 c) {
  return vec3_splat(1.0) - exp(-0.95*c);
}

vec3 nrsCurveFilmic(vec3 c) {
  vec3 x = c*1.25;
  vec3 n = x*(0.22*x + 0.05) + 0.004;
  vec3 d = x*(0.22*x + 0.50) + 0.06;
  return (n/d - 0.004/0.06)*1.45;
}

vec3 nrsCurveCine(vec3 c) {
  vec3 y = c*0.9;
  return (y*(2.51*y + 0.03))/(y*(2.43*y + 0.59) + 0.14);
}

vec3 nrsCurveReinhard(vec3 c) {
  vec3 r = c*0.8;
  return r*(1.0 + r*(1.0/9.0))/(1.0 + r);
}

vec3 nrsToneMix(vec3 c) {
  c = max(c, vec3_splat(0.0));
  float total = NL_NRS_TM_SOFT + NL_NRS_TM_FILMIC + NL_NRS_TM_ACES + NL_NRS_TM_REINHARD;
  vec3 m = NL_NRS_TM_SOFT*nrsCurveSoft(c) + NL_NRS_TM_FILMIC*nrsCurveFilmic(c)
         + NL_NRS_TM_ACES*nrsCurveCine(c) + NL_NRS_TM_REINHARD*nrsCurveReinhard(c);
  return clamp(m/total, 0.0, 1.0);
}

vec3 nrsGrade(vec3 c) {
  float l = dot(c, vec3(0.299, 0.587, 0.114));
  c = clamp(mix(vec3_splat(l), c, NL_NRS_TM_SATURATION), 0.0, 1.0);
  c = mix(c, c*c*(3.0 - 2.0*c), NL_NRS_TM_CONTRAST);
  c *= vec3(1.0 + NL_NRS_TM_WARMTH, 1.0, 1.0 - NL_NRS_TM_WARMTH);
  return clamp(c, 0.0, 1.0);
}

vec3 colorCorrection(vec3 col) {
  #ifdef NL_EXPOSURE
    col *= NL_EXPOSURE;
  #endif

  #if NL_TONEMAP_TYPE == 7
    col = nrsToneMix(col);
    col = pow(max(col, vec3_splat(0.0)), vec3_splat(1.0/NL_NRS_GAMMA));
    col = nrsGrade(col);
  #else
    // ref - https://64.github.io/tonemapping/
    #if NL_TONEMAP_TYPE == 3
      // extended reinhard tonemap
      const float whiteScale = 0.063;
      col = col*(1.0+col*whiteScale)/(1.0+col);
    #elif NL_TONEMAP_TYPE == 4
      // aces tonemap
      const float a = 1.04;
      const float b = 0.03;
      const float c = 0.93;
      const float d = 0.56;
      const float e = 0.14;
      col *= 0.85;
      col = clamp((col*(a*col + b)) / (col*(c*col + d) + e), 0.0, 1.0);
    #elif NL_TONEMAP_TYPE == 5
      // hspe aces tonemap
      const float a = 2.51;
      const float b = 0.03;
      const float c = 2.43;
      const float d = 0.59;
      const float e = 0.14;
      col = clamp((col*(a*col + b)) / (col*(c*col + d) + e), 0.0, 1.0);
    #elif NL_TONEMAP_TYPE == 6
      // hspe filmic tonemap
      col *= 1.4;
      col = (col*(2.5*col + 0.1)) / (col*(2.52*col) + 0.9) - 0.1;
      col = max(col, 0.0);
    #elif NL_TONEMAP_TYPE == 2
      // simple reinhard tonemap
      col = col/(1.0+col);
    #elif NL_TONEMAP_TYPE == 1
      // exponential tonemap
      col = 1.0-exp(-col*0.8);
    #endif

    // gamma correction
    col = pow(col, vec3_splat(1.0/NL_GAMMA));

    #ifdef NL_SATURATION
      col = mix(vec3_splat(luminance(col)), col, NL_SATURATION);
    #endif

    #ifdef NL_TINT
      col *= mix(NL_TINT_LOW, NL_TINT_HIGH, col);
    #endif
  #endif

  return col;
}

// inv used in fogcolor for nether
vec3 colorCorrectionInv(vec3 col) {
  #ifdef NL_TINT
    col /= mix(NL_TINT_LOW, NL_TINT_HIGH, col); // not accurate inverse
  #endif

  #ifdef NL_SATURATION
    col = mix(vec3_splat(dot(col,vec3(0.21, 0.71, 0.08))), col, 1.0/NL_SATURATION);
  #endif

  // incomplete
  // extended reinhard only
  float ws = 0.7966;
  col = pow(col, vec3_splat(NL_GAMMA));
  col = col*(ws + col)/(ws + col*(1.0 - ws));

  #ifdef NL_EXPOSURE
    col /= NL_EXPOSURE;
  #endif

  return col;
}

#endif
