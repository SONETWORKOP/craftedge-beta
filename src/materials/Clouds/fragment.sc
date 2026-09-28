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
  // SKY-ONLY CLOUDS TEST (check.txt): mesh path fully off - saare clouds
  // Sky dome se aate hain. Koi mesh contribution possible nahi.
  gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}
