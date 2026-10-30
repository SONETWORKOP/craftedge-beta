$input v_color0, v_fog, v_light, v_texcoord0, v_edgemap
#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/ActorUtil.dragonh>
#include <newb/main.sh>
uniform vec4 ChangeColor;
uniform vec4 OverlayColor;
uniform vec4 ColorBased;
uniform vec4 MatColor;
uniform vec4 MultiplicativeTintColor;
SAMPLER2D_AUTOREG(s_MatTexture);
void main() {
  #if defined(DEPTH_ONLY) || defined(INSTANCING)
    gl_FragColor = vec4_splat(0.0);
    return;
  #endif
  vec4 albedo = MatColor * texture2D(s_MatTexture, v_texcoord0);
  #ifdef ALPHA_TEST
    if (albedo.a < 0.5) {
      discard;
    }
  #endif
  #ifdef MULTI_COLOR_TINT
    albedo = applyMultiColorChange(albedo, ChangeColor.rgb, MultiplicativeTintColor.rgb);
  #else
    albedo = applyColorChange(albedo, ChangeColor, albedo.a);
  #endif
  albedo.rgb *= mix(vec3_splat(1.0), v_color0.rgb, ColorBased.x);
  albedo = applyOverlayColor(albedo, OverlayColor);
  // ✅ 修正后：仅 252(0.9882) ~ 253(0.9922) 发光
  float alphaTex = albedo.a;
  float lower = smoothstep(0.9881, 0.9882, alphaTex);
  float upper = 1.0 - smoothstep(0.9922, 0.9923, alphaTex);
  float mask = lower * upper;
  vec3 baseRaw = albedo.rgb;
  vec3 emissivePath = baseRaw * 1.0;
  vec3 litPath = baseRaw;
  litPath *= litPath * v_light.rgb;
  litPath *= nlEntityEdgeHighlight(v_edgemap);
  litPath = mix(litPath, v_fog.rgb, v_fog.a);
  litPath = colorCorrection(litPath);
  albedo.rgb = mix(litPath, emissivePath, mask);
  gl_FragColor = albedo;
}
