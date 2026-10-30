#ifndef CLOUDS_H
#define CLOUDS_H

#include "detection.h"
#include "noise.h"
#include "sky.h"

// simple clouds 2D noise
float cloudNoise2D(vec2 p, highp float t, float rain) {
  t *= NL_CLOUD1_SPEED;
  p += t;
  p.y += 3.0*sin(0.3*p.x + 0.1*t);

  vec2 p0 = floor(p);
  vec2 u = p-p0;
  u *= u*(3.0-2.0*u);
  vec2 v = 1.0-u;

  float n = mix(
    mix(rand(p0),rand(p0+vec2(1.0,0.0)), u.x),
    mix(rand(p0+vec2(0.0,1.0)),rand(p0+vec2(1.0,1.0)), u.x),
    u.y
  );
  n *= 0.5 + 0.5*sin(p.x*0.6 - 0.5*t)*sin(p.y*0.6 + 0.8*t);
  n = min(n*(1.0+rain), 1.0);
  return n*n;
}

// simple clouds
vec4 renderCloudsSimple(nl_skycolor skycol, vec3 pos, highp float t, float rain) {
  pos.xz *= NL_CLOUD1_SCALE;
  float d = cloudNoise2D(pos.xz, t, rain);
  vec4 col = vec4(skycol.horizonEdge + skycol.zenith, smoothstep(0.1,0.6,d));
  col.rgb += 1.5*dot(col.rgb, vec3(0.3,0.4,0.3))*smoothstep(0.6,0.2,d)*col.a;
  col.rgb *= 1.0 - 0.8*rain;
  return col;
}

// rounded clouds
// rounded clouds 3D density map
float cloudDf(vec3 pos, float rain, vec2 boxiness) {
  boxiness *= 0.999;
  vec2 p0 = floor(pos.xz);
  vec2 u = max((pos.xz-p0-boxiness.x)/(1.0-boxiness.x), 0.0);
  u *= u*(3.0 - 2.0*u);

  vec4 r = vec4(rand(p0), rand(p0+vec2(1.0,0.0)), rand(p0+vec2(1.0,1.0)), rand(p0+vec2(0.0,1.0)));
  r = smoothstep(0.1001+0.2*rain, 0.1+0.2*rain*rain, r); // rain transition

  float n = mix(mix(r.x,r.y,u.x), mix(r.w,r.z,u.x), u.y);

  // round y
  n *= 1.0 - 1.5*smoothstep(boxiness.y, 2.0 - boxiness.y, 2.0*abs(pos.y-0.5));


  n = max(1.25*(n-0.2), 0.0); // smoothstep(0.2, 1.0, n)
  n *= n*(3.0 - 2.0*n);
  return n;
}

vec4 renderCloudsRounded(
    vec3 vDir, vec3 vPos, float rain, float time, vec3 horizonCol, vec3 zenithCol,
    const int steps, const float thickness, const float thickness_rain, const float speed,
    const vec2 scale, const float density, const vec2 boxiness
) {
  float height = 7.0*mix(thickness, thickness_rain, rain);
  float stepsf = float(steps);

  // scaled ray offset
  vec3 deltaP;
  deltaP.y = 1.0;
  deltaP.xz = height*scale*vDir.xz/(0.02+0.98*abs(vDir.y));

  // local cloud pos
  vec3 pos;
  pos.y = 0.0;
  pos.xz = scale*(vPos.xz + vec2(1.0,0.5)*(time*speed));
  pos += deltaP;

  deltaP /= -stepsf;

  // alpha, gradient
  vec2 d = vec2(0.0,1.0);
  for (int i=1; i<=steps; i++) {
    float m = cloudDf(pos, rain, boxiness);
    d.x += m;
    d.y = mix(d.y, pos.y, m);
    pos += deltaP;
  }
  d.x *= smoothstep(0.03, 0.1, d.x);
  d.x /= (stepsf/density) + d.x;

  if (vPos.y < 0.0) { // view from top
    d.y = 1.0 - d.y;
  }

  vec4 col = vec4(zenithCol + horizonCol, d.x);
  col.rgb += dot(col.rgb, vec3(0.3,0.4,0.3))*d.y*d.y;
  col.rgb *= 1.0 - 0.8*rain;
  return col;
}

float cloudsNoiseVr(vec2 p, float t) {
  float n = fastVoronoi2(p + t, 1.8);
  n *= fastVoronoi2(3.0*p + t, 1.5);
  n *= fastVoronoi2(9.0*p + t, 0.4);
  n *= fastVoronoi2(27.0*p + t, 0.1);
  //n *= fastVoronoi2(82.0*pos + t, 0.02); // more quality
  return n*n;
}

vec4 renderClouds(vec2 p, float t, float rain, vec3 horizonCol, vec3 zenithCol, const vec2 scale, const float velocity, const float shadow) {
  p *= scale;
  t *= velocity;

  // layer 1
  float a = cloudsNoiseVr(p, t);
  float b = cloudsNoiseVr(p + NL_CLOUD3_SHADOW_OFFSET*scale, t);

  // layer 2
  p = 1.4 * p.yx + vec2(7.8, 9.2);
  t *= 0.5;
  float c = cloudsNoiseVr(p, t);
  float d = cloudsNoiseVr(p + NL_CLOUD3_SHADOW_OFFSET*scale, t);

  // higher = less clouds thickness
  // lower separation betwen x & y = sharper
  vec2 tr = vec2(0.6, 0.7) - 0.12*rain;
  a = smoothstep(tr.x, tr.y, a);
  c = smoothstep(tr.x, tr.y, c);

  // shadow
  b *= smoothstep(0.2, 0.8, b);
  d *= smoothstep(0.2, 0.8, d);

  vec4 col;
  col.a = a + c*(1.0-a);
  col.rgb = horizonCol + horizonCol.ggg;
  col.rgb = mix(col.rgb, 0.5*(zenithCol + zenithCol.ggg), shadow*mix(b, d, c));
  col.rgb *= 1.0-0.7*rain;

  return col;
}

// hspe cloud colors
void nlHspeCloudColors(nl_environment env, out vec3 light, out vec3 shadow) {
  float night, dusk;
  nlHspeTime(env, night, dusk);
  float wet = env.rainFactor;
  light = mix(mix(mix(NL_HSPE_CLOUD_DAY, NL_HSPE_CLOUD_NIGHT, night), NL_HSPE_CLOUD_DUSK, dusk), mix(NL_HSPE_CLOUD_DRAIN, NL_HSPE_CLOUD_NRAIN, night), wet);
  shadow = mix(mix(mix(NL_HSPE_CSHADOW_DAY, NL_HSPE_CSHADOW_NIGHT, night), NL_HSPE_CSHADOW_DUSK, dusk), mix(NL_HSPE_CSHADOW_DRAIN, NL_HSPE_CSHADOW_NRAIN, night), wet);
  light = nlHspeLin(light);
  shadow = nlHspeLin(shadow);
}

// hspe value noise
float nlHspeNoise(highp vec2 x) {
  highp vec2 a = floor(x);
  highp vec2 b = fract(x);
  b = b*b*(3.0-2.0*b);
  float h1 = rand(a);
  float h2 = rand(a + vec2(1.0, 0.0));
  float h3 = rand(a + vec2(0.0, 1.0));
  float h4 = rand(a + vec2(1.0, 1.0));
  return mix(mix(h1, h2, b.x), mix(h3, h4, b.x), b.y);
}

// hspe cloud map
float nlHspeCloudMap(highp vec2 p, highp float t, float wet, const int depth) {
  float f = 0.0;
  float a = mix(NL_HSPE_CLOUD_DENSITY_CLEAR, NL_HSPE_CLOUD_DENSITY_RAIN, wet);
  float b = 2.0;
  float m = 0.0;
  mat2 rot = mat2(0.8, 0.6, -0.6, 0.8);
  p += t*0.05;
  for (int i = 0; i < depth; i++) {
    f += nlHspeNoise(p)/a;
    m += 1.0/b;
    p = mul(rot, p)*NL_HSPE_CLOUD_NOISE;
    p += vec2(100.0, 100.0);
    a *= NL_HSPE_CLOUD_SOFTNESS;
    b *= NL_HSPE_CLOUD_SOFTNESS;
    p -= t*NL_HSPE_CLOUD_SPEED*sqrt(float(i));
  }
  return f/m;
}

// hspe clouds (vDir = real view direction, y up)
vec4 nlHspeClouds(vec3 vDir, highp float t, float rain, vec3 light, vec3 shadow) {
  highp vec2 fc = vDir.xz/(max(vDir.y, 0.0) + 0.1);
  fc /= length(fc)*0.15 + 1.0;
  fc *= 1.75;

  float fade = clamp((length(fc) - 10.0)/(5.0 - 10.0), 0.0, 1.0);
  fade *= smoothstep(0.0, 0.08, vDir.y);
  fc *= NL_HSPE_CLOUD_SCALE;

  float c = nlHspeCloudMap(fc, t, rain, NL_HSPE_CLOUD_QUALITY);
  c = clamp((c - 0.4)/0.12, 0.0, 1.0);

  float b = nlHspeCloudMap(fc/1.0435, t, rain, 4)*0.6666 + nlHspeCloudMap(fc/1.07, t, rain, 3)*0.3333;
  b = clamp((b - 0.4)/(mix(0.7, 1.0, rain) - 0.4), 0.0, 1.0);

  return vec4(mix(light, shadow, b), c*fade);
}

// aurora is rendered on clouds layer
#ifdef NL_AURORA
vec4 renderAurora(vec3 p, float t, float rain, float dayFactor) {
  t *= NL_AURORA_VELOCITY;
  p.xz *= NL_AURORA_SCALE;
  p.xz += 0.05*sin(p.x*4.0 + 20.0*t);

  float d0 = sin(p.x*0.1 + t + sin(p.z*0.2));
  float d1 = sin(p.z*0.1 - t + sin(p.x*0.2));
  float d2 = sin(p.z*0.1 + 1.0*sin(d0 + d1*2.0) + d1*2.0 + d0*1.0);
  d0 *= d0; d1 *= d1; d2 *= d2;
  d2 = d0/(1.0 + d2/NL_AURORA_WIDTH);

  float mask = max(-dayFactor, 0.0);
  mask *= mask*(1.0-0.8*rain);
  return vec4(NL_AURORA*mix(NL_AURORA_COL1,NL_AURORA_COL2,d1),1.0)*d2*mask;
}
#endif

vec4 nlCloudAuroraReflection(nl_skycolor skycol, nl_environment env, vec3 viewDir, vec3 wPos, vec3 CAMERA_POS, highp float t) {
  vec2 cloudPos = wPos.xz;
  cloudPos += (187.0-(wPos.y+CAMERA_POS.y))*viewDir.xz/viewDir.y;
  float fade = clamp(2.0 - 0.005*length(cloudPos), 0.0, 1.0);
  cloudPos += CAMERA_POS.xz;

  vec4 refl = vec4_splat(0.0);

  #ifdef NL_AURORA
    vec4 aurora = renderAurora(cloudPos.xyy, t, env.rainFactor, smoothstep(0.2, -0.2, env.dayFactor));
    aurora.a *= fade;
    refl = vec4(2.0*aurora.rgb*aurora.a, aurora.a);
  #endif

  #if NL_CLOUD_TYPE == 1
    vec4 clouds = renderCloudsSimple(skycol, cloudPos.xyy, t, env.rainFactor);
    clouds.a *= fade;
    refl = vec4(mix(refl.rgb, clouds.rgb, clouds.a), min(refl.a + clouds.a, 1.0));
  #elif NL_CLOUD_TYPE == 4
    // hspe clouds reflection
    vec3 hl, hs;
    nlHspeCloudColors(env, hl, hs);
    vec4 hc = nlHspeClouds(vec3(viewDir.x, -viewDir.y, viewDir.z), t, env.rainFactor, hl, hs);
    refl = vec4(mix(refl.rgb, hc.rgb, hc.a), min(refl.a + hc.a, 1.0));
  #endif

  return refl;
}

#endif