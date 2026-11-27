#ifndef YSS_CLOUDS_H
#define YSS_CLOUDS_H

#include "yss_port.h"
#include "yss_stars.h"

highp float yssNoise(highp vec2 x) {
  highp vec2 a = floor(x);
  highp vec2 b = fract(x);
  highp vec2 c = b*b*(3.0-2.0*b);
  highp float d = yssRand(a);
  highp float e = yssRand(a+vec2(1.0,0.0));
  highp float f = yssRand(a+vec2(0.0,1.0));
  highp float g = yssRand(a+vec2(1.0,1.0));
  return mix(mix(d, e, c.x), mix(f, g, c.x), c.y);
}

highp float yssChunkNoise(highp vec2 x) {
  highp vec2 a = floor(x);
  highp vec2 b = fract(x);
  highp vec2 c = vec2(yssSmooth(0.2,0.6,b.x), yssSmooth(0.2,0.6,b.y));
  highp float d = yssRand(a);
  highp float e = yssRand(a+vec2(1.0,0.0));
  highp float f = yssRand(a+vec2(0.0,1.0));
  highp float g = yssRand(a+vec2(1.0,1.0));
  return mix(mix(d, e, c.x), mix(f, g, c.x), c.y);
}

vec3 yssCloudDiffuse(float du, float ni, float rain) {
  vec3 c = nlYssTime3(NL_YSS_DAY_CLOUD_DIF, NL_YSS_DUSK_CLOUD_DIF, NL_YSS_NIGHT_CLOUD_DIF, du, ni);
  return mix(c, NL_YSS_RAIN_CLOUD_DIF, rain);
}

vec3 yssCloudDirect(float du, float ni, float rain) {
  vec3 day = NL_YSS_DAY_CLOUD_DIR - NL_YSS_DAY_CLOUD_DIF;
  vec3 dusk = NL_YSS_DUSK_CLOUD_DIR - NL_YSS_DUSK_CLOUD_DIF;
  vec3 c = nlYssTime3(day, dusk, NL_YSS_NIGHT_CLOUD_DIR, du, ni);
  return mix(c, NL_YSS_RAIN_CLOUD_DIR, rain);
}

highp float yssClouds2D(highp vec2 px, highp float t) {
  highp float a = 2.0;
  highp float b = 0.0;
  px += vec2(1.0,2.0)*t*0.1;
  px *= 0.09;
  b += 0.3*nlYssLinestep(0.2, 1.0, yssNoise(vec2_splat(px.x+px.y)));
  for (int i=0; i<5; i++) {
    b += yssNoise(px)/a;
    a *= 2.2;
    px *= 3.5;
    px -= t*0.1*pow(a, 0.01);
  }
  return pow(abs(b), 5.0);
}

float yssACloud(highp vec3 x, highp float t) {
  highp vec2 p = vec2(x.x + yssNoise(vec2_splat(x.z*0.6))*0.7, x.z*0.4);
  p *= 0.9;
  float j = nlYssLinestep(0.05, 1.2, yssClouds2D(p*5.0, t));
  return min(j, 0.8);
}

highp float yssBlockCloudMap(highp vec2 px, highp float t) {
  px *= 0.1;
  float a = 4.0;
  float b = 1.0;
  highp vec2 ps = px - vec2_splat(t*0.05);
  float m = pow(yssChunkNoise(ps), 2.0);
  m += pow(yssChunkNoise(ps*1.3), 2.0);
  for (int i=0; i<10; i++) {
    ps *= a;
    b *= 0.4;
    m += max(yssNoise(ps + vec2_splat(t*0.5))*b, 0.0);
  }
  return pow(m, 3.0);
}

highp float yssBlockCloudMapSim(highp vec2 px, highp float t) {
  px *= 0.1;
  float a = 4.0;
  float b = 1.0;
  highp vec2 ps = px - vec2_splat(t*0.05);
  float m = pow(yssChunkNoise(ps), 2.0);
  m += pow(yssChunkNoise(ps*1.3), 2.0);
  for (int i=0; i<5; i++) {
    ps *= a;
    b *= 0.4;
    m += max(yssNoise(ps + vec2_splat(t*0.5))*b, 0.0);
  }
  return pow(m, 3.0);
}

vec4 yssBlockClouds(highp vec3 x, highp vec3 dir, highp float t) {
  float bottom = yssBlockCloudMapSim(x.xz, t);
  x += dir*0.5/max(dir.y, 0.001);
  float shape = yssBlockCloudMap(x.xz, t);
  vec4 c;
  c.a = smoothstep(0.5, 1.5, shape);
  c.rgb = mix(vec3_splat(0.35), vec3_splat(1.0), yssSmooth(5.0, 0.0, bottom));
  return c;
}

float yssVolNc(vec2 p) {
  return floor(yssRand(floor(p))*0.4 + yssRand(floor(p*0.4))*0.7 + yssRand(floor(p*0.2))*0.4);
}

float yssVolClo(vec3 p, float bottom, float height, highp float t) {
  float inRange = step(bottom, p.y)*step(p.y, height);
  return inRange*yssVolNc(p.xz*0.5 + vec2_splat(t*0.05));
}

vec4 yssVolClouds(highp vec3 dir, highp vec3 start, float bottom, float height, float stepHeight, highp float t) {
  vec3 hit = vec3_splat(0.0);
  float value = 0.0;
  vec3 stepv = dir*stepHeight/max(dir.y, 0.001);
  for (int i=0; i<NL_YSS_BLV_STEPS; i++) {
    start += stepv;
    value += yssVolClo(start, bottom, height, t)*0.05;
    if (value > 0.0 && value <= 0.05) {
      hit = start;
    }
    if (value >= 1.0 || start.y > height) {
      break;
    }
  }
  vec4 r;
  r.rgb = vec3_splat(mix(1.0, 0.2, pow(yssSmooth(height, bottom, hit.y), 2.0)));
  r.a = clamp(value, 0.0, 1.0);
  return r;
}

vec4 yssRenderClouds(highp vec3 vd, highp float t, float dayFactor, float rain) {
  if (vd.y <= 0.0) {
    return vec4_splat(0.0);
  }
  float du = nlYssDusk(dayFactor);
  float ni = nlYssNight(dayFactor);
  vec3 ldir = yssCloudDirect(du, ni, rain);
  vec3 ldif = yssCloudDiffuse(du, ni, rain);

  float y = max(vd.y, 0.001);
  vec3 fp = vd*3.0/y;
  float stp = nlYssLinestep(40.0, 20.0, length(fp.xz));

  vec3 acc = vec3_splat(0.0);
  float accA = 0.0;

  #ifdef NL_YSS_N2DCLOUD
    {
      float a = yssACloud(vd*4.0/y*0.8, t)*stp;
      acc = acc*(1.0-a) + (ldir + ldif)*a;
      accA = accA*(1.0-a) + a;
    }
  #endif

  #ifdef NL_YSS_BL2DCLOUD
    {
      vec3 fc = vd*8.0/y;
      float stpC = nlYssLinestep(60.0, 40.0, length(fc.xz));
      vec4 h = yssBlockClouds(fc, vd, t);
      float a = h.a*stpC;
      acc = acc*(1.0-a) + (h.rgb*ldir + ldif)*a;
      accA = accA*(1.0-a) + a;
    }
  #endif

  #ifdef NL_YSS_BLVCLOUD
    {
      vec3 fa = vd*5.0/y;
      vec4 h = yssVolClouds(vd, fa, 5.0, 5.0 + NL_YSS_CLOUDTHICK, 0.01, t);
      float a = h.a*stp;
      acc = acc*(1.0-a) + (h.rgb*ldir + ldif)*a;
      accA = accA*(1.0-a) + a;
    }
  #endif

  return vec4(nlYssToLinear(acc/max(accA, 0.0001)), accA);
}

#endif
