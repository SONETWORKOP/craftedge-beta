#ifndef NL_NIGHT_BAND_H
#define NL_NIGHT_BAND_H

vec3 nlRenderNightBand(vec3 d, float t) {
  const vec3 bandN = vec3(0.6027, 0.1507, 0.7836);
  const vec3 bandA = vec3(-0.6324, -0.5086, 0.5843);

  float along = dot(d, bandA);
  float across = dot(d, bandN);
  across += 0.030*sin(along*3.1 + t*0.008);
  across += 0.012*sin(along*7.7 - t*0.005);

  float x = abs(across)*(1.0/NL_NIGHT_BAND_WIDTH);
  float core = exp(-x*x*18.0);
  float glow = exp(-x*x*4.0);

  vec3 col = pow(NL_NIGHT_BAND_GLOW_COL, vec3_splat(2.2))*glow;
  col += pow(NL_NIGHT_BAND_COL, vec3_splat(2.2))*core;
  return col;
}

#endif
