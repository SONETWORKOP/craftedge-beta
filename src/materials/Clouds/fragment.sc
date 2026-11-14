$input v_color0, v_dayFactor
#include <newb/config.h>
#if NL_CLOUD_TYPE >= 2
  $input v_color1, v_color2, v_fogColor
#endif

#include <bgfx_shader.sh>
#include <newb/main.sh>

#ifdef NL_PIXEL_CLOUDS
SAMPLER2D_AUTOREG(s_vanilla);

highp float nlPixelHash(highp vec2 p) {
  vec2 uv = p / 256.0;
  return texture2D(s_vanilla, uv).r;
}

vec2 nlPixelSmoothBlend(vec2 x) {
  vec2 v = x * x * (3.0 - 2.0 * x);
  return smoothstep(0.1, 0.95, v);
}

float nlPixelGridSDF(vec2 uv) {
  vec2 f = fract(uv);
  vec2 i = floor(uv);

  float bl = step(0.55, nlPixelHash(i + vec2(0.0, 0.0)));
  float br = step(0.55, nlPixelHash(i + vec2(1.0, 0.0)));
  float tl = step(0.55, nlPixelHash(i + vec2(0.0, 1.0)));
  float tr = step(0.55, nlPixelHash(i + vec2(1.0, 1.0)));

  vec2 fade = nlPixelSmoothBlend(f);

  float mixBottom = mix(bl, br, fade.x);
  float mixTop = mix(tl, tr, fade.x);
  return mix(mixBottom, mixTop, fade.y);
}

vec3 nlPixelCloudShape(vec2 uv, float timeVal) {
  float a = 0.0;
  float b = 0.0;
  float isTime = -timeVal * 0.02;

  vec2 shadeDirection = vec2(0.05, 0.03);
  float shadeSize = 0.2;

  uv *= 13.5;
  uv.y *= 1.5;

  for (int i = 0; i < NL_PIXEL_CLOUD_LAYERS; i++) {
    uv /= 1.01;
    float r = nlPixelGridSDF(uv + isTime);
    a = max(a, r);
  }

  vec2 shadeSampleUv = uv + isTime - shadeDirection;
  float shadeShape = nlPixelGridSDF(shadeSampleUv);

  b = smoothstep(shadeSize, shadeSize + 0.05, shadeShape);
  a = smoothstep(0.1, 0.15, a);

  a -= b * a * 0.25;
  return vec3_splat(clamp(a, 0.0, 1.0));
}
#endif

void main() {
  gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);

  #ifdef NL_PIXEL_CLOUDS
    #if NL_CLOUD_TYPE == 2
      vec3 worldPos = v_color0.xyz;
      float fadeFactor = v_color0.w;
      float timeVal = v_color2.w;
      float rainFactor = v_color1.w;

      vec2 cloudPlane = worldPos.xz * 0.005;
      vec3 cloudNoise = nlPixelCloudShape(cloudPlane, timeVal);

      // soft edges: hard pixel threshold ko thoda wide smooth karo
      float cloudAlpha = smoothstep(0.0, 0.55, cloudNoise.r);
      float internalShadow = smoothstep(0.0, 1.0, cloudNoise.r);

      vec3 dayColor = vec3(2.0, 2.0, 2.0);
      vec3 peachColor = vec3(1.5, 0.75, 0.65);

      // Night clouds sky se hi rang lete hain (zenith + horizonEdge).
      vec3 nightSky = mix(v_color1.rgb, v_color2.rgb, 0.50);
      vec3 nightColor = nightSky * 2.0 + vec3_splat(0.008);
      float nightLum = dot(nightColor, vec3(0.299, 0.587, 0.114));
      nightColor = mix(nightColor, vec3_splat(nightLum), 0.22);

      float nightFactor = 1.0 - smoothstep(-0.08, 0.12, v_dayFactor);
      float peachFactor = 1.0 - smoothstep(0.0, 0.22, abs(v_dayFactor));
      peachFactor *= peachFactor * peachFactor;

      vec3 cloudCol = mix(dayColor, nightColor, nightFactor);
      cloudCol = mix(cloudCol, peachColor, peachFactor);

      float shadowStrength = mix(0.82, 0.90, nightFactor);
      cloudCol *= mix(shadowStrength, 1.0, internalShadow);

      // barish me thoda grey, taaki pixel clouds sky se alag na lage
      vec3 rainGray = vec3_splat(luminance(cloudCol)) * vec3(0.80, 0.85, 0.92);
      cloudCol = mix(cloudCol, rainGray, rainFactor * 0.55);
      cloudCol *= 1.0 - 0.20 * rainFactor;

      float tightFade = smoothstep(0.0, 0.4, fadeFactor);
      cloudCol = colorCorrection(cloudCol);
      gl_FragColor = vec4(cloudCol, cloudAlpha * tightFade);
    #endif
  #endif
}
