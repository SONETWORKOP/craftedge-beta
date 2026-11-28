#ifndef NARESH_CLOUDS_H
#define NARESH_CLOUDS_H

#include "sky.h"
#include "naresh_sky.h"

#ifdef NL_NARESH_CLOUDS

float nlNrsCloudDensity(vec2 p, float cover) {
  vec2 w = vec2(nlNrsNoise(p*0.7 + 11.0), nlNrsNoise(p*0.7 + 37.0)) - 0.5;
  p += w*0.55;
  float n = nlNrsFbm(p, NL_NARESH_CLOUD_OCTAVES);
  return smoothstep(cover - 0.04, cover + 0.30, n);
}

void nlNrsCloudLight(nl_environment env, out vec3 L, out vec3 Lcol, out vec3 amb) {
  vec3 S = env.sunDir;
  float rain = env.rainFactor;

  if (S.y > -0.04) {
    L = S;
    float t = smoothstep(0.0, 0.5, S.y);
    vec3 c = mix(vec3(1.0, 0.38, 0.12), vec3(1.10, 0.98, 0.84), t);
    Lcol = c*(NL_NARESH_CLOUD_SUN_POWER*smoothstep(-0.04, 0.16, S.y));
  } else {
    L = -S;
    Lcol = vec3(0.52, 0.66, 1.0)*(NL_NARESH_CLOUD_MOON_POWER*smoothstep(0.04, 0.24, -S.y));
  }
  Lcol *= 1.0 - 0.92*rain;

  float day = smoothstep(-0.12, 0.28, S.y);
  float duskX = (S.y - 0.02)*4.5;
  float dusk = exp(-duskX*duskX);
  vec3 a = mix(vec3(0.075, 0.115, 0.26), vec3(0.46, 0.62, 0.92)*0.85, day);
  a = mix(a, vec3(0.55, 0.36, 0.40)*0.55, dusk*0.6);
  a = mix(a, vec3_splat(nlNrsLuma(a))*vec3(0.96, 1.0, 1.06), rain*0.55);
  amb = a*((1.0 + rain)*(1.0 - 0.20*rain));
}

vec3 nlNrsCloudLayers(vec3 sky, vec3 d, nl_environment env, highp float time, float q) {
  float rain = env.rainFactor;
  float dy = d.y;
  float cover = NL_NARESH_CLOUD_COVER - 0.28*rain;
  vec3 outc = sky;

  vec3 L;
  vec3 Lcol;
  vec3 amb;
  nlNrsCloudLight(env, L, Lcol, amb);

  float t = 1.0/(max(dy, 0.0) + 0.22);
  vec2 p = d.xz*t*NL_NARESH_CLOUD_SCALE + time*NL_NARESH_CLOUD_SPEED;
  float d1 = nlNrsCloudDensity(p, cover);

  float cosl = saturate(dot(d, L));
  vec3 cloud = vec3_splat(0.0);
  float alpha = 0.0;

  if (d1 > 0.002) {
    float trans = 0.7;
    if (q > 0.75) {
      vec2 toL = L.xz/max(L.y, 0.3);
      float s1 = nlNrsCloudDensity(p + toL*0.22, cover);
      float s2 = nlNrsCloudDensity(p + toL*0.50, cover);
      trans = exp(-1.7*(s1 + s2*0.8));
    }

    float silver = pow(cosl, 5.0)*(1.0 - d1)*1.2;
    vec3 lit = Lcol*(trans*(0.85 + silver*1.6));
    vec3 ambc = amb*((0.55 + 0.45*(1.0 - d1))*0.95);
    cloud = ambc + lit*0.9;

    cloud = mix(cloud, vec3_splat(nlNrsLuma(cloud))*vec3(0.95, 0.98, 1.05), rain*0.6);

    vec3 horc = nlNrsSkyBase(normalize(vec3(d.x, 0.02, d.z)), env.sunDir, rain);
    float ap = pow(saturate(1.0 - dy*1.6), 3.0)*0.6;
    cloud = mix(cloud, horc*1.05, ap);
    cloud = nlCloudPaint(cloud, env.dayFactor, env.sunsetSide, rain);

    alpha = (1.0 - exp(-3.2*d1))*smoothstep(0.0, 0.14, dy)*0.95;
    cloud = nlCloudCyanEdge(cloud, alpha, env.dayFactor);
    outc = mix(outc, cloud, alpha);
  }

  #if NL_NARESH_CIRRUS
    if (q > 0.75) {
      float t2 = 1.0/(max(dy, 0.0) + 0.30);
      vec2 c2 = vec2(d.x*t2*0.35 + time*0.01, d.z*t2*1.2 + time*0.005);
      float n2 = nlNrsFbm(c2, 4.0);
      float ci = smoothstep(0.50, 0.78, n2)*0.38*smoothstep(0.02, 0.25, dy);
      vec3 cic = amb*0.9 + Lcol*(0.5*(0.6 + 0.8*cosl));
      cic = nlCloudPaint(cic, env.dayFactor, env.sunsetSide, rain);
      cic = nlCloudCyanEdge(cic, ci, env.dayFactor);
      outc = mix(outc, cic, ci*(1.0 - alpha*0.7));
    }
  #endif

  return outc;
}

#endif

#endif
