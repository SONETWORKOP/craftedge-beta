#ifndef HSPE_BLOCK_H
#define HSPE_BLOCK_H

#include "utils.h"
#include "detection.h"
#include "sky.h"
#include "clouds.h"
#include "water.h"

// texture height
float nlHspeTexHeight(sampler2D tex, highp vec2 uv, vec2 texel) {
  #ifdef NL_HSPE_TEX_NORMAL_BLUR
    // smooth sample
    uv -= texel*0.5;
    vec2 a = uv - texel*fract(uv/texel);
    vec2 b = fract(uv/texel);
    b = b*b*(3.0-2.0*b);
    vec3 s1 = texture2D(tex, a).rgb;
    vec3 s2 = texture2D(tex, a + vec2(texel.x, 0.0)).rgb;
    vec3 s3 = texture2D(tex, a + vec2(0.0, texel.y)).rgb;
    vec3 s4 = texture2D(tex, a + texel).rgb;
    return luminance(mix(mix(s1, s2, b.x), mix(s3, s4, b.x), b.y));
  #else
    return luminance(texture2D(tex, uv).rgb);
  #endif
}

// texture normal (x, up, z)
vec3 nlHspeTexNormal(sampler2D tex, highp vec2 uv) {
  vec2 texel = 1.0/vec2(textureSize(tex, 0));
  vec2 wave = vec2(NL_HSPE_TEX_NORMAL_OFFSET, NL_HSPE_TEX_NORMAL_OFFSET)*texel;
  float a = nlHspeTexHeight(tex, uv, texel);
  float b = nlHspeTexHeight(tex, uv + vec2(wave.x, 0.0), texel);
  float c = nlHspeTexHeight(tex, uv + vec2(0.0, wave.y), texel);
  return normalize(vec3((b-a)*0.5, 1.0/NL_HSPE_TEX_HEIGHT, (c-a)*0.5));
}

// texture aurora mirror - same curtain as the Sky dome, added on top of the
// night water/metal reflection. r = reflected ray in the negated convention
// used by nlRenderSky (r.y < 0), so flip it back for the sky sampler.
vec3 nlHspeAuroraMirror(nl_environment env, vec3 r, vec3 CAMERA_POS, highp float t) {
  #if defined(NL_AURORA_REFLECTION) && defined(NL_AURORA)
    vec3 aurV = normalize(-r);
    float aurUp = clamp(aurV.y, 0.0, 1.0);
    float aurNight = 1.0 - smoothstep(-0.02, 0.32, env.dayFactor);
    if (aurNight > 0.001 && aurUp > 0.15) {
      float dither = fract(sin(dot(aurV.xy, vec2(12.9898, 78.233))) * 43758.5453);
      vec3 aur = NL_AURORA_TEX*aurNight*nlAuroraBorealis(
        aurV, aurUp, dither, env.rainFactor, CAMERA_POS.xz, t
      );
      #ifdef NL_WATER_AURORA_MIRROR
        aur *= NL_WATER_AURORA_MIRROR;
      #endif
      return aur;
    }
  #endif
  return vec3_splat(0.0);
}

// reflection (r = negated true direction, same as nlRenderSky)
vec3 nlHspeReflection(nl_skycolor skycol, nl_environment env, vec3 r, vec3 relPos, vec3 CAMERA_POS, highp float t) {
  vec3 col = nlRenderSky(skycol, env, r, t, false);
  #ifdef NL_CLOUD_AURORA_REFLECTION
    if (r.y < 0.0) {
      vec4 cr = nlCloudAuroraReflection(skycol, env, r, relPos, CAMERA_POS, t, 1.0);
      col = mix(col, cr.rgb, cr.a);
    }
  #endif
  return col;
}

// hspe water per pixel, returns vec4(reflection * fresnel, alpha)
vec4 nlHspeWaterPixel(nl_environment env, vec3 gPos, vec3 CAMERA_POS, vec3 VIEW_POS, vec2 uv1, highp float t) {
  vec3 relPos = gPos - CAMERA_POS;
  vec3 vdir = normalize(VIEW_POS - relPos);
  vec2 lit = uv1*uv1;

  nl_skycolor skycol = nlSkyColors(env);
  vec3 nrm = normalize(nlHspeWaterNormal(gPos.xz, t*NL_HSPE_WATER_SPEED));

  float cosR = dot(nrm, vdir);
  vec3 r = vdir - 2.0*cosR*nrm;

  vec3 refl = nlHspeReflection(skycol, env, r, relPos, CAMERA_POS, t);

  // torch reflection
  float tc = 0.5 + 0.5*sin(16.0*r.x)*sin(16.0*r.z);
  refl += NL_OVERWORLD_TORCH_COL*NL_TORCHLIGHT_INTENSITY*lit.x*tc*tc;

  // mask under shade
  refl *= 0.05 + lit.y*1.14;

  // aurora mirror is exempt from the night lightmap shade mask so the
  // curtain stays visible on water after sunset (beta look)
  refl += nlHspeAuroraMirror(env, r, CAMERA_POS, t);

  cosR = abs(cosR);
  float fresnel = calculateFresnel(cosR, NL_HSPE_WATER_F0);
  float op = 1.0 - cosR;
  float op2 = op*op;
  float alpha = NL_HSPE_WATER_ALPHA + (1.0-NL_HSPE_WATER_ALPHA)*op2*op2;
  return vec4(refl*fresnel, alpha);
}

// hspe metal block reflection
// dpx, dpy = dFdx/dFdy of position, st1, st2 = dFdx/dFdy of uv (computed in fragment shader)
vec3 nlHspeMetal(nl_environment env, vec3 diffuse, vec3 albedo, vec3 texN, vec3 dpx, vec3 dpy, vec2 st1, vec2 st2, vec2 uv1, vec3 gPos, vec3 CAMERA_POS, highp float t) {
  vec3 wp = gPos - CAMERA_POS;
  vec3 N = normalize(cross(dpx, dpy));
  if (dot(N, wp) > 0.0) {
    N = -N;
  }

  vec3 cn = N;
  #ifdef NL_HSPE_TEX_NORMAL
    vec3 T = normalize(dpx*st2.y - dpy*st1.y);
    vec3 B = -normalize(cross(N, T));
    cn = normalize(T*texN.x + B*texN.z + N*texN.y);
  #endif

  vec3 vd = normalize(wp);
  vec3 R = reflect(vd, cn);

  nl_skycolor skycol = nlSkyColors(env);
  vec3 rc = nlHspeReflection(skycol, env, -R, wp, CAMERA_POS, t);

  float cosine = clamp(abs(dot(vd, cn)), 0.0, 1.0);
  vec3 F0 = mix(vec3_splat(0.04), albedo, NL_HSPE_METALLIC);
  float f1 = 1.0 - cosine;
  float f2 = f1*f1;
  vec3 F = F0 + (1.0 - F0)*(f2*f2*f1);

  float hres = mix(0.35, 0.8, clamp((texN.x + texN.z)*1.5, 0.0, 1.0));
  rc += NL_OVERWORLD_TORCH_COL*NL_TORCHLIGHT_INTENSITY*smoothstep(hres, 1.0, uv1.x);
  rc += nlHspeAuroraMirror(env, -R, CAMERA_POS, t);

  return mix(diffuse, F*rc, smoothstep(0.625, 0.875, uv1.y));
}

#endif
