$input v_color0, v_dayFactor
#include <newb/config.h>
#if NL_CLOUD_TYPE >= 2
  $input v_color1, v_color2, v_fogColor
#endif

#include <bgfx_shader.sh>
#include <newb/main.sh>

uniform vec4 CameraPosition;

#define NL_CLOUD_PARAMS(x) NL_CLOUD2##x##STEPS, NL_CLOUD2##x##THICKNESS, NL_CLOUD2##x##RAIN_THICKNESS, NL_CLOUD2##x##VELOCITY, NL_CLOUD2##x##SCALE, NL_CLOUD2##x##DENSITY, NL_CLOUD2##x##SHAPE

void main() {
  // Default + EDITOR_CLOUDS dono me mesh off hai.
  // EDITOR_CLOUDS volumetric (shader-editor-clouds.txt) Sky dome me banta hai,
  // isliye mesh se double-draw nahi hona chahiye.
  gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}
