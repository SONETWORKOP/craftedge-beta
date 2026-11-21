#ifndef SUN_GLOW_H
#define SUN_GLOW_H

vec3 nlApplyTerrainSunGlow(
  vec3 color, vec3 rd, vec3 sunDir, vec3 glowColor,
  float sunsetFactor, float rainFactor, float strength
) {
  float hl = length(rd.xz);
  float sunHz = length(sunDir.xz);
  vec2 sunDirNormXZ = sunHz > 1e-4 ? sunDir.xz/sunHz : vec2(1.0, 0.0);
  float az = hl > 1e-4 ? dot(rd.xz/hl, sunDirNormXZ) : 0.0;
  float sunAmt = max(dot(rd, sunDir), 0.0);
  float sAmt = max(sunAmt, 1e-4);
  float s2 = sAmt*sAmt;
  float s4 = s2*s2;
  float s16 = s4*s4;
  float s64 = s16*s16*s16*s16;
  float azPos = max(az*0.5 + 0.5, 1e-4);
  float band = exp(-abs(rd.y)*6.0)*pow(azPos, 1.5)*sunsetFactor;
  float weather = 1.0 - 0.85*rainFactor;
  float sunUp = smoothstep(-0.10, 0.03, sunDir.y);
  float glowWeight = (band*0.40 + s4*0.18 + s16*0.32 + s64*0.45)*weather*sunUp*strength;
  float horFade = smoothstep(-0.20, 0.05, rd.y);
  float finalGlow = glowWeight*horFade;
  float extinction = clamp(glowWeight*0.45, 0.0, 0.30)*horFade;
  return color*(1.0 - extinction) + glowColor*finalGlow;
}

#endif
