$input v_color0, v_color1, v_fog, v_refl, v_texcoord0, v_lightmapUV, v_position, v_extra

#include <bgfx_shader.sh>
#include <newb/main.sh>

uniform vec4 ViewPositionAndTime;
uniform vec4 FogColor;
uniform vec4 FogAndDistanceControl;
uniform vec4 TimeOfDay;
uniform vec4 CameraPosition;

SAMPLER2D_AUTOREG(s_MatTexture);
SAMPLER2D_AUTOREG(s_SeasonsTexture);
SAMPLER2D_AUTOREG(s_LightMapTexture);

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

  diffuse.rgb = colorCorrection(diffuse.rgb);

  gl_FragColor = diffuse;
}
