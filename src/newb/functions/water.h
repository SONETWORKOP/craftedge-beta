#ifndef WATER_H
#define WATER_H

#include "utils.h"
#include "detection.h"
#include "sky.h"
#include "clouds.h"
#include "noise.h"

// fresnel - Schlick's approximation
float calculateFresnel(float cosR, float r0) {
  float a = 1.0-cosR;
  float a2 = a*a;
  return r0 + (1.0-r0)*a2*a2*a;
}

#ifdef NL_HSPE_WATER
// hspe wave height
float nlHspeWaterHeight(highp vec2 c, highp float t) {
  c /= vec2(0.8, 0.7);
  c += c.yx*0.3;
  return abs(dot(sin(nlHspeNoise(c - t)*2.5 + c + t), vec2(1.75, 0.75)))*NL_HSPE_WATER_HEIGHT/7.5;
}

// hspe wave slope (xz)
vec3 nlHspeWaterNormal(highp vec2 c, highp float t) {
  float d = NL_HSPE_WATER_NORMAL_OFFSET;
  float h0 = nlHspeWaterHeight(c, t);
  float h1 = nlHspeWaterHeight(c + vec2(d, 0.0), t);
  float h3 = nlHspeWaterHeight(c + vec2(0.0, d), t);
  float sx = (h1 - h0)/d*2.0;
  float sz = (h3 - h0)/d*2.0;
  float z = abs(sx + sz);
  return vec3(sx, -max(1.0 - z*z, 0.2), sz);
}
#endif

vec4 nlWater(
  inout vec4 color, inout vec3 wPos, nl_skycolor skycol, nl_environment env, vec4 COLOR, vec3 viewDir,
  vec3 cPos, vec3 tiledCpos, vec3 gPos, vec3 CAMERA_POS, vec3 light, vec3 torchColor, vec2 lit,
  float fractCposY, float camDist, highp float t
) {

  vec2 bump = vec2_splat(movingNoise2D(gPos.xz + gPos.yy, NL_WATER_WAVE_SPEED*t, 0.6));

  vec3 nrm;
  if (fractCposY > 0.0) { // top plane
    #ifdef NL_HSPE_WATER
      // hspe waves
      nrm = nlHspeWaterNormal(gPos.xz, t*NL_HSPE_WATER_SPEED);
    #else
      nrm.xz = bump*NL_WATER_BUMP;
      nrm.y = -1.0;
    #endif
    /*if (fractCposY>0.8 || fractCposY<0.9) { // flat plane
    } else { // slanted plane and highly slanted plane
    }*/
  } else { // reflection for side plane
    bump *= 0.5 + 0.5*sin(3.0*t*NL_WATER_WAVE_SPEED + cPos.y*PI_HALF);
    nrm.xz = normalize(viewDir.xz) + bump.y*(1.0-viewDir.xz*viewDir.xz)*NL_WATER_BUMP;
    nrm.y = bump.x*NL_WATER_BUMP;
  }
  nrm = normalize(nrm);

  float cosR = dot(nrm, viewDir);
  viewDir = viewDir - 2.0*cosR*nrm ; // reflect(viewDir, nrm)

  vec3 waterRefl = nlRenderSky(skycol, env, viewDir, t, false);

  #if defined(NL_CLOUD_AURORA_REFLECTION)
    if (viewDir.y < 0.0) {
      vec4 cloudRefl = nlCloudAuroraReflection(skycol, env, viewDir, wPos, CAMERA_POS, t);
      waterRefl = mix(waterRefl, cloudRefl.rgb, cloudRefl.a);
    }
  #endif

  // torch light reflection
  float tc = 0.5+0.5*sin(16.0*viewDir.x)*sin(16.0*viewDir.z);
  waterRefl += torchColor*NL_TORCHLIGHT_INTENSITY*lit.x*tc*tc;

  // mask sky reflection under shade
  if (!env.end) {
    waterRefl *= 0.05 + lit.y*1.14;
  }

  #ifdef NL_WATER_REFL_MASK
    float mask = 0.05+0.05*sin(viewDir.x*12.0)*sin(viewDir.z*6.0);
    waterRefl *= smoothstep(mask-0.2,mask+0.13,viewDir.y*viewDir.y);
  #endif

  cosR = abs(cosR);
  #ifdef NL_HSPE_WATER
    // hspe fresnel, base color and alpha
    float fresnel = calculateFresnel(cosR, NL_HSPE_WATER_F0);
    float opacity = 1.0-cosR;
    float op2 = opacity*opacity;

    color.rgb = vec3_splat(NL_HSPE_WATER_BASE*NL_HSPE_WATER_BASE)*(1.0-0.8*fresnel);
    color.a = NL_HSPE_WATER_ALPHA + (1.0-NL_HSPE_WATER_ALPHA)*op2*op2;
  #else
    float fresnel = calculateFresnel(cosR, 0.07);
    float opacity = 1.0-cosR;

    color.rgb *= 0.22*NL_WATER_TINT*(1.0-0.8*fresnel);
    color.a = mix(COLOR.a*NL_WATER_TRANSPARENCY, 1.0, opacity*opacity);
  #endif

  #if defined(NL_WATER_WAVE) && !defined(NL_HSPE_WATER)
    if (camDist < 14.0) {
      wPos.y -= 0.5*(bump.x+0.5)*NL_WATER_BUMP;
    }
  #endif

  return vec4(waterRefl, fresnel);
}

#endif
