$input v_color0, v_fog, v_light, v_glintuv
#include <bgfx_shader.sh>
#include <MinecraftRenderer.Materials/ActorUtil.dragonh>
#include <newb/main.sh>
uniform vec4 ChangeColor;
uniform vec4 OverlayColor;
uniform vec4 GlintColor;
uniform vec4 MatColor;
uniform vec4 MultiplicativeTintColor;
uniform vec4 TileLightColor;
uniform vec4 ColorBased;
SAMPLER2D_AUTOREG(s_GlintTexture);
void main() {
  #if defined(DEPTH_ONLY) || defined(INSTANCING)
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    return;
  #endif
  vec4 albedo = vec4(mix(vec3(1.0, 1.0, 1.0), v_color0.rgb, ColorBased.x), 1.0);
  #ifdef MULTI_COLOR_TINT
    albedo = applyMultiColorChange(albedo, ChangeColor.rgb, MultiplicativeTintColor.rgb);
  #else
    albedo = applyColorChange(albedo, ChangeColor, albedo.a);
    albedo.a *= ChangeColor.a;
  #endif
  albedo = applyOverlayColor(albedo, OverlayColor);
  #ifdef ALPHA_TEST
    if (albedo.a < 0.5) {
      discard;
    }
  #endif
  // 保存原始基础色，用于自发光层
  vec3 baseRaw = albedo.rgb;
  // 原版完整附魔+光照流程（所有像素都会计算附魔条纹）
  vec4 light = nlGlint(v_light, v_glintuv, s_GlintTexture, GlintColor, TileLightColor, albedo);
  albedo.rgb *= albedo.rgb * light.rgb;
  albedo.rgb = mix(albedo.rgb, v_fog.rgb, v_fog.a);
  albedo.rgb = colorCorrection(albedo.rgb);
  // 发光判定（沿用稳定版逻辑）
  float diff = v_color0.a - 0.99;
  float mask = 1.0 - smoothstep(-0.0001, 0.0001, diff);
  // 固定亮度自发光层，加法叠加不覆盖原有附魔
  vec3 emissive = baseRaw * 0.8 * mask;
  albedo.rgb += emissive;
  gl_FragColor = albedo;
}
