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

// ---- Clear pretty water: smooth swell + fine chop (naya wave code) ----
// badi smooth lehren (2 direction) + chhoti chop, sasta (sirf sin/cos)
vec2 nlWaterSwell(vec2 p, float t) {
  float s = t*NL_WATER_WAVE_SPEED;
  vec2 g = vec2(
    sin(p.x*0.9 + s*1.4)*0.45 + sin((p.x + p.y)*0.55 - s*1.0)*0.30,
    cos(p.y*1.0 - s*1.2)*0.45 + cos((p.x - p.y)*0.5 + s*0.9)*0.30
  );
  g.x += sin(p.y*2.7 + s*2.3)*0.18 + sin((p.x - p.y)*3.1 + s*2.0)*0.12;
  g.y += cos(p.x*2.4 - s*2.2)*0.18 + cos((p.x + p.y)*3.3 - s*1.8)*0.12;
  return g;
}

float nlWaterHeight(vec2 p, float t) {
  float s = t*NL_WATER_WAVE_SPEED;
  return sin(p.x*0.9 + s*1.4)*0.35
       + sin((p.x + p.y)*0.55 - s*1.0)*0.25
       + cos(p.y*1.0 - s*1.2)*0.35
       + sin(p.y*2.7 + s*2.3)*0.12;
}

vec4 nlWater(
  inout vec4 color, inout vec3 wPos, nl_skycolor skycol, nl_environment env, vec4 COLOR, vec3 viewDir,
  vec3 cPos, vec3 tiledCpos, vec3 gPos, vec3 CAMERA_POS, vec3 light, vec3 torchColor, vec2 lit,
  float fractCposY, float camDist, highp float t
) {

  vec2 bump = vec2_splat(movingNoise2D(gPos.xz + gPos.yy, NL_WATER_WAVE_SPEED*t, 0.6));
  // naya smooth swell (bump ke saath mix taaki reflection sundar toote)
  vec2 swell = nlWaterSwell(gPos.xz, t);

  vec3 nrm;
  if (fractCposY > 0.0) { // top plane
    nrm.xz = (bump*0.55 + swell*0.65)*NL_WATER_BUMP*1.15;
    nrm.y = -1.0;
    /*if (fractCposY>0.8 || fractCposY<0.9) { // flat plane
    } else { // slanted plane and highly slanted plane
    }*/
  } else { // reflection for side plane
    bump *= 0.5 + 0.5*sin(3.0*t*NL_WATER_WAVE_SPEED + cPos.y*PI_HALF);
    float viewDirXZLengthSq = dot(viewDir.xz, viewDir.xz);
    vec2 sideDir = viewDirXZLengthSq > 0.000001 ? viewDir.xz/sqrt(viewDirXZLengthSq) : vec2(1.0,0.0);
    nrm.xz = sideDir + bump.y*(1.0-viewDir.xz*viewDir.xz)*NL_WATER_BUMP;
    nrm.y = bump.x*NL_WATER_BUMP;
  }
  nrm = normalize(nrm);

  float cosR = dot(nrm, viewDir);
  vec3 reflDir = viewDir - 2.0*cosR*nrm ; // reflect(viewDir, nrm)

  vec3 waterRefl = nlRenderSky(skycol, env, reflDir, t, false);

  // Vertex-path cloud+aurora fallback (sirf jab fragment mirror off ho).
  // Beta me fragment cloud-mirror hataya hua hai, ye fallback wahi deta hai.
  #if defined(NL_CLOUD_AURORA_REFLECTION) && defined(NL_NO_WATER_CLOUD_REFL)
    if (reflDir.y < 0.0) {
      vec4 cloudRefl = nlCloudAuroraReflection(skycol, env, reflDir, wPos, CAMERA_POS, t, 1.0);
      waterRefl = mix(waterRefl, cloudRefl.rgb, cloudRefl.a);
    }
  #endif

  // torch light reflection (soft - tez freq se vertex aliasing hota tha)
  float tc = 0.5+0.5*sin(6.0*reflDir.x)*sin(6.0*reflDir.z);
  waterRefl += torchColor*NL_TORCHLIGHT_INTENSITY*lit.x*tc;

  // sun glitter (soft single lobe - pow 600*2.0 se blocky white pixels aate the)
  #if defined(NL_SUNLIGHT_INTENSITY)
    vec3 sunDir = env.sunDir.y > 0.0 ? env.sunDir : env.moonDir;
    vec3 halfVector = sunDir + viewDir;
    float halfLengthSq = dot(halfVector, halfVector);
    vec3 halfDir = halfLengthSq > 0.000001 ? halfVector/sqrt(halfLengthSq) : nrm;
    float specAngle = max(dot(nrm, halfDir), 0.0);
    #ifdef NL_WATER_GLITTER
      float specHighlight = pow(specAngle, 220.0)*0.9;
      specHighlight *= NL_WATER_GLITTER;
    #else
      float specHighlight = pow(specAngle, 256.0);
    #endif
    specHighlight *= lit.y;
    waterRefl += specHighlight*NL_SUNLIGHT_INTENSITY*sunLightTint(env.dayFactor, env.rainFactor);
    // blowout rok: paani me hi clamp taaki safed pixel na phatein
    waterRefl = min(waterRefl, vec3_splat(2.5));
  #endif

  // mask sky reflection under shade
  if (!env.end) {
    waterRefl *= 0.08 + lit.y*1.05;
  }

  #ifdef NL_WATER_REFL_MASK
    float mask = 0.05+0.05*sin(reflDir.x*12.0)*sin(reflDir.z*6.0);
    waterRefl *= smoothstep(mask-0.2,mask+0.13,reflDir.y*reflDir.y);
  #endif

  cosR = abs(cosR);
  // clear water: fresnel base 0.02, milky base hataya (0.22->0.16), edge halka
  float fresnel = calculateFresnel(cosR, 0.02);
  float opacity = 1.0-cosR;

  color.rgb *= 0.16*NL_WATER_TINT*(1.0-0.65*fresnel);
  color.a = mix(COLOR.a*NL_WATER_TRANSPARENCY, 1.0, opacity*opacity*0.85);

  #ifdef NL_WATER_WAVE
    if (camDist < 16.0) {
      wPos.y += nlWaterHeight(gPos.xz, t)*NL_WATER_BUMP*0.55;
    }
  #endif

  return vec4(waterRefl, fresnel);
}

// ---- water.txt port (Download/water.txt): underwater extinction ----
// litColor = diffuse after lighting (+fog mix). waterFlag 1.0 = water pixels.
// NOTE: snippet ke fogfactor/scatter/uwFogColor lines usme dead hain (kahin
// apply nahi hote), isliye live lines (extinction + moon scale) port ki hain.
// saturate() -> clamp() (bgfx-safe, same cheez).
vec3 nlUnderwaterScatter(vec3 litColor, vec3 sunDir, float waterFlag) {
  float moonVisibility = clamp(-sunDir.y, 0.0, 1.0);

  // red jaldi absorb, blue bachta hai (murky depth tint, non-water pixels par)
  const vec3 extinctionCoeffs = vec3(0.15, 0.08, 0.01);
  litColor *= mix(exp(-extinctionCoeffs * 8.0), vec3(1.0, 1.0, 1.0), waterFlag);
  litColor *= mix(2.0 - 0.5 * moonVisibility, 1.0, waterFlag);

  return litColor;
}

#endif
