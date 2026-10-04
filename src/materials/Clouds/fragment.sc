$input v_color0, v_dayFactor
#include <newb/config.h>
#if NL_CLOUD_TYPE >= 2
  $input v_color1, v_color2, v_fogColor
#endif

#include <bgfx_shader.sh>
#include <newb/main.sh>

uniform vec4 CameraPosition;

#define NL_CLOUD_PARAMS(x) NL_CLOUD2##x##STEPS, NL_CLOUD2##x##THICKNESS, NL_CLOUD2##x##RAIN_THICKNESS, NL_CLOUD2##x##VELOCITY, NL_CLOUD2##x##SCALE, NL_CLOUD2##x##DENSITY, NL_CLOUD2##x##SHAPE

#ifdef EDITOR_CLOUDS
// Shader-editor clouds (Download/clouds/fragment.sc) - texture-free version.
// Original hash() s_vanilla texture use karta tha, yahan rand() (noise.h)
// use hota hai taaki extra vanilla.png/uniform ki zarurat na pade.
highp float editorHash(highp vec2 p) {
  return rand(p);
}
vec2 editorBlend(vec2 x) {
  vec2 v = x * x * (3.0 - 2.0 * x);
  return smoothstep(0.1, 0.95, v);
}
float editorGrid(vec2 uv) {
  vec2 f = fract(uv);
  vec2 i = floor(uv);
  float bl = step(0.55, editorHash(i + vec2(0.0, 0.0)));
  float br = step(0.55, editorHash(i + vec2(1.0, 0.0)));
  float tl = step(0.55, editorHash(i + vec2(0.0, 1.0)));
  float tr = step(0.55, editorHash(i + vec2(1.0, 1.0)));
  vec2 fade = editorBlend(f);
  float mixBottom = mix(bl, br, fade.x);
  float mixTop = mix(tl, tr, fade.x);
  return mix(mixBottom, mixTop, fade.y);
}
vec3 editorPixelated(vec2 uv, float timeVal) {
  float a = 0.0;
  float b = 0.0;
  float isTime = -timeVal * 0.02;
  vec2 shadeDirection = vec2(0.05, 0.03);
  uv *= 13.5;
  uv.y *= 1.5;
  for (int i = 0; i < 3; i++) {
    uv /= 1.02;
    float r = editorGrid(uv + isTime);
    a = max(a, r);
  }
  vec2 shadeSampleUv = uv + isTime - shadeDirection;
  float shadeShape = editorGrid(shadeSampleUv);
  b = smoothstep(0.05, 0.9, shadeShape);
  a = smoothstep(0.1, 0.15, a);
  a -= b * a * 0.25;
  return vec3(clamp(a, 0.0, 1.0));
}
#endif

void main() {
#ifdef EDITOR_CLOUDS
  // Shader-editor pixelated clouds (Download/clouds, NL_CLOUD_TYPE 2 path).
  // Vertex se: v_color0.xyz = worldPos, v_color0.w = fade,
  // v_color2.w = time, v_dayFactor = day/night.
  // NOTE: texture-free hash (rand from noise.h) - s_vanilla ki zarurat nahi.
  vec3 worldPos = v_color0.xyz;
  float fadeFactor = v_color0.w;
  float timeVal = v_color2.w;

  vec2 cp = worldPos.xz * 0.005;
  vec3 cloudNoise = editorPixelated(cp, timeVal);
  float cloudAlpha = clamp(length(cloudNoise.r), 0.0, 1.0);

  vec3 dayColor   = vec3(2.0, 2.0, 2.0);
  vec3 peachColor = vec3(1.5, 0.75, 0.65);
  vec3 nightColor = vec3(0.03, 0.04, 0.09);

  float nightFactor = step(v_dayFactor, 0.0);

  float peachFactor = 1.0 - smoothstep(0.0, 0.22, abs(v_dayFactor));
  peachFactor *= peachFactor * peachFactor;

  vec3 cloudcol = mix(dayColor, nightColor, nightFactor);
  cloudcol = mix(cloudcol, peachColor, peachFactor);

  float internalShadow = smoothstep(0.0, 1.0, cloudNoise.r);
  float shadowStrength = mix(0.75, 0.92, nightFactor);
  cloudcol *= mix(shadowStrength, 1.0, internalShadow);

  float tightFade = smoothstep(0.0, 0.4, fadeFactor);
  vec4 color = vec4(cloudcol, cloudAlpha * tightFade);
  color.rgb = colorCorrection(color.rgb);
  gl_FragColor = color;
#else
  // SKY-ONLY CLOUDS (default): mesh path fully off - saare clouds
  // Sky dome se aate hain. Koi mesh contribution possible nahi.
  gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
#endif
}
