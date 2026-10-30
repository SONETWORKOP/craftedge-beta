$input v_color0, v_color1, v_fog, v_refl, v_texcoord0, v_lightmapUV, v_position, v_extra, v_reflPbr, v_reflSun, v_sunMoon

#include <bgfx_shader.sh>
// noise texture driving the aurora curtain - shared with the Sky dome via
// newb/functions/clouds.h so the water mirror draws the same aurora shape
SAMPLER2D_AUTOREG(s_NoiseVoxel);
#define NL_AURORA_REFLECTION
#include <newb/main.sh>

uniform vec4 ViewPositionAndTime;
uniform vec4 FogColor;
uniform vec4 FogAndDistanceControl;
uniform vec4 TimeOfDay;
uniform vec4 CameraPosition;
uniform vec4 MoonPhase;

SAMPLER2D_AUTOREG(s_MatTexture);
SAMPLER2D_AUTOREG(s_SeasonsTexture);
SAMPLER2D_AUTOREG(s_LightMapTexture);
SAMPLER2D_AUTOREG(s_SunTexture);
SAMPLER2D_AUTOREG(s_MoonTexture);

/*
  Real textured sun/moon mirror on water.

  Projects the reflected view ray into the celestial body's local plane (built
  from its direction), samples the vanilla sun or moon texture there, and keeps
  only the bright pixels (luminance mask) inside a soft circular falloff (dist
  mask). `quadTan` is half the body's angular size; `cellScale`/`cellOffset`
  select a sub-rectangle of the texture (the moon_phases.png 4x2 grid).
*/
vec3 celestialTextureMovement(
  sampler2D tex, vec3 bodyDir, vec3 rayDir, float quadTan,
  vec2 cellScale, vec2 cellOffset, out float mask
) {
  vec3 forward = normalize(bodyDir);
  vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), forward));
  vec3 up = cross(forward, right);

  vec2 uv = vec2(dot(rayDir, right), dot(rayDir, up));
  uv = uv / quadTan * 0.5 + 0.5;

  float distMask = smoothstep(0.5, 0.48, length(uv - 0.5));
  vec2 texUV = uv*cellScale + cellOffset;

  vec3 bodyColor = texture2D(tex, texUV).rgb;
  float lum = dot(bodyColor, vec3(0.299, 0.587, 0.114));
  float maskLum = smoothstep(0.0, 0.9, lum);
  mask = maskLum * distMask;
  return bodyColor * mask;
}

void main() {
  #if defined(DEPTH_ONLY_OPAQUE) || defined(DEPTH_ONLY) || defined(INSTANCING)
    gl_FragColor = vec4(1.0,1.0,1.0,1.0);
    return;
  #endif

  vec4 diffuse = texture2D(s_MatTexture, v_texcoord0);
  vec4 color = v_color0;

  // metal marker (texture alpha 0.97 ~ 0.98)
  float texAlpha = diffuse.a;

  #ifdef ALPHA_TEST
    if (diffuse.a < 0.6) {
      discard;
    }
  #endif

  #if defined(SEASONS) && (defined(OPAQUE) || defined(ALPHA_TEST))
    diffuse.rgb *= mix(vec3(1.0,1.0,1.0), texture2D(s_SeasonsTexture, v_color1.xy).rgb * 2.0, v_color1.z);
  #endif

  vec3 glow = nlGlow(s_MatTexture, v_texcoord0, v_extra.a);

  diffuse.rgb *= diffuse.rgb;
  vec3 albedoLin = diffuse.rgb;

  // block depth
  vec3 texN = vec3(0.0, 1.0, 0.0);
  #if defined(NL_HSPE_TEX_NORMAL) && !defined(RENDER_AS_BILLBOARDS)
    if (v_extra.b < 0.9) {
      texN = nlHspeTexNormal(s_MatTexture, v_texcoord0);
      vec3 un = normalize(texN*vec3(1.0, 0.1, 1.0));
      diffuse.rgb *= max(un.x + un.z, 0.0)*NL_HSPE_TEX_STRENGTH + 1.0;
    }
  #endif

  #if defined(TRANSPARENT) && !(defined(SEASONS) || defined(RENDER_AS_BILLBOARDS))
    if (v_extra.b > 0.9) {
      diffuse.rgb = vec3_splat(1.0 - NL_WATER_TEX_OPACITY*(1.0 - diffuse.b*1.8));
      diffuse.a = color.a;
    }
  #else
    diffuse.a = 1.0;
  #endif

  diffuse.rgb *= color.rgb;
  diffuse.rgb += glow;

  if (v_extra.b > 0.9) {
    bool hspeWaterDone = false;

    // water
    #if defined(NL_HSPE_WATER) && defined(TRANSPARENT) && !(defined(SEASONS) || defined(RENDER_AS_BILLBOARDS))
      float waterFlat = abs(dFdx(v_extra.g)) + abs(dFdy(v_extra.g));
      if (waterFlat < 0.0004) {
        nl_environment wenv = nlDetectEnvironment(TimeOfDay.x, FogColor.rgb, FogAndDistanceControl.xyz);
        if (!wenv.underwater && !wenv.nether) {
          vec4 wr = nlHspeWaterPixel(wenv, v_position.xyz, CameraPosition.xyz, ViewPositionAndTime.xyz, v_lightmapUV, ViewPositionAndTime.w);
          diffuse.rgb += wr.rgb;
          diffuse.a = wr.a;
          hspeWaterDone = true;
        }
      }
    #endif

    if (!hspeWaterDone) {
      diffuse.rgb += v_refl.rgb*v_refl.a;

      #ifndef NL_NO_WATER_CLOUD_REFL
        // rebuild the sky palette so the mirrored aurora is shaded with the
        // same horizon tint the Sky material uses (cheap: only mix() ops)
        nl_environment wenv;
        wenv.end = false;
        wenv.nether = false;
        wenv.underwater = false;
        wenv.rainFactor = v_reflPbr.w;
        wenv.dayFactor = v_reflSun.w;
        wenv.sunDir = v_reflSun.xyz;
        wenv.moonDir = v_sunMoon.xyz;
        wenv.fogCol = FogColor.rgb;

        // beta: paani me clouds reflection nahi - sirf AURORA ka aks +
        // sun/moon disc mirror. (Medium subpack me ye bhi band.)
        #ifndef NL_NO_WATER_CLOUD_AURORA_REFL
        #ifdef NL_AURORA_REFLECTION
          {
            vec3 aurV = normalize(v_reflPbr.xyz);
            vec3 aurReflDir = vec3(-aurV.x, aurV.y, -aurV.z);
            if (aurReflDir.y > 0.004) {
              float aurVdotU = clamp(aurReflDir.y, 0.0, 1.0);
              float aurNight = 1.0 - smoothstep(-0.02, 0.32, wenv.dayFactor);
              if (aurNight > 0.001 && aurVdotU > 0.15) {
                float aurDither = fract(sin(dot(aurReflDir.xy, vec2(12.9898, 78.233))) * 43758.5453);
                vec3 aurora = NL_AURORA_TEX*aurNight*nlAuroraBorealis(
                  aurReflDir, aurVdotU, aurDither, wenv.rainFactor, CameraPosition.xz, ViewPositionAndTime.w
                );
                #ifdef NL_WATER_AURORA_MIRROR
                  float auroraAmt = NL_WATER_AURORA_MIRROR;
                #else
                  float auroraAmt = 0.55;
                #endif
                diffuse.rgb += aurora*auroraAmt;
              }
            }
          }
        #endif
        #endif

        // real textured sun/moon mirror on water (same flat-mirror ray as the
        // clouds). v_reflSun.xyz is the real sun dir, v_sunMoon.xyz the real
        // moon dir. Which body shows is decided by the sun's actual height
        // (v_reflSun.y) - not dayFactor - so at sunset the sun leaves the
        // water exactly when it visually sets and the moon takes over.
        vec3 sunV = normalize(v_reflPbr.xyz);
        vec3 sunReflDir = vec3(-sunV.x, sunV.y, -sunV.z);
        if (sunReflDir.y > 0.004) {
          float sunMask;
          vec3 sunTex;
          float sunHeight = v_reflSun.y;
          float sunFade = smoothstep(-0.03, 0.03, sunHeight);
          if (sunHeight > 0.0) {
            sunTex = celestialTextureMovement(
              s_SunTexture, wenv.sunDir, sunReflDir, NL_WATER_SUN_QUAD_TAN,
              vec2_splat(1.0), vec2_splat(0.0), sunMask
            );
          } else {
            // moon_phases.png is a 4x2 grid (8 phases, row-major)
            float phase = mod(floor(MoonPhase.x + 0.5), 8.0);
            vec2 moonCell = vec2(mod(phase, 4.0), floor(phase / 4.0));
            sunTex = celestialTextureMovement(
              s_MoonTexture, wenv.moonDir, sunReflDir, NL_WATER_MOON_QUAD_TAN,
              vec2(0.25, 0.5), moonCell*vec2(0.25, 0.5), sunMask
            );
          }
          float bodyVisible = sunHeight > 0.0 ? sunFade : (1.0 - sunFade);
          bodyVisible *= (1.0 - wenv.rainFactor);
          vec3 sunCol = sunLightTint(wenv.dayFactor, wenv.rainFactor);
          sunCol *= NL_SUNLIGHT_INTENSITY;
          diffuse.rgb += sunTex*sunCol*NL_WATER_SUN_DISC*bodyVisible;
        }
      #endif
    }
  } else if (v_refl.a > 0.0) {
    // reflective effect - only on xz plane
    float dy = abs(dFdy(v_extra.g));
    if (dy < 0.0002) {
      float mask = v_refl.a*(clamp(v_extra.r*10.0,8.2,8.8)-7.8);
      diffuse.rgb *= 1.0 - 0.6*mask;
      diffuse.rgb += v_refl.rgb*mask;
    }
  }

  // rain drop ripples - only on flat upward faces (not water/plants)
  #if defined(NL_RAIN_RIPPLE) && !defined(ALPHA_TEST) && !defined(RENDER_AS_BILLBOARDS)
    if (v_position.w > 0.0 && v_extra.b < 0.9 && v_extra.r > 0.95) {
      float flatTest = abs(dFdx(v_extra.g)) + abs(dFdy(v_extra.g));
      if (flatTest < 0.0004) {
        // day light (sky brightness) or torch light
        float dayLight = clamp(dot(v_fog.rgb, vec3(0.5,0.5,0.5)), 0.0, 1.0);
        float brightness = max(dayLight, pow(v_lightmapUV.x, 1.4)*0.8);
        vec4 ripple = nlRainRippleShade(v_position.xz, ViewPositionAndTime.w, v_position.w, v_fog.a, brightness);
        diffuse.rgb *= 1.0 - ripple.a;
        diffuse.rgb += ripple.rgb;
      }
    }
  #endif

  // metal reflection
  #if defined(NL_HSPE_METAL) && defined(OPAQUE) && !defined(SEASONS)
    bool metalFlag = texAlpha >= 0.97 && texAlpha < 0.98 && v_color1.a != 0.0;
    if (metalFlag) {
      nl_environment menv = nlDetectEnvironment(TimeOfDay.x, FogColor.rgb, FogAndDistanceControl.xyz);
      if (!menv.nether && !menv.underwater) {
        vec3 mRel = v_position.xyz - CameraPosition.xyz;
        diffuse.rgb = nlHspeMetal(menv, diffuse.rgb, albedoLin, texN, dFdx(mRel), dFdy(mRel), dFdx(v_texcoord0), dFdy(v_texcoord0), v_lightmapUV, v_position.xyz, CameraPosition.xyz, ViewPositionAndTime.w);
      }
    }
  #endif

  diffuse.rgb = mix(diffuse.rgb, v_fog.rgb, v_fog.a);

  // water.txt: underwater extinction, tonemap se pehle.
  // v_sunMoon.w = camera underwater flag (vertex packs it), v_extra.b = water pixels.
  if (v_sunMoon.w > 0.5) {
    float uwWaterFlag = step(0.9, v_extra.b);
    diffuse.rgb = nlUnderwaterScatter(diffuse.rgb, normalize(v_reflSun.xyz), uwWaterFlag);
  }

  diffuse.rgb = colorCorrection(diffuse.rgb);

  gl_FragColor = diffuse;
}
