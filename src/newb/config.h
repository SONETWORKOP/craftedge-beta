#ifndef NL_CONFIG_H
#define NL_CONFIG_H

/*
  NEWB SHADER MAIN CONFIG
  This part contains base configuration options for the shader.

  TOGGLES
  - If [toggle] is mentioned, then
    options can be commented to disable (by adding '//')
  - eg: #define PLANTS_WAVE    -> this is ON
        //#define PLANTS_WAVE  -> this is OFF

  COLOR VALUES
  - Color format: vec3(red,green,blue)
  - 1.0 means 100%, 0.0 means 0%
  - eg: vec3(1.0,1.0,0.0) = yellow

  VALUES
  - Values must be decimal
  - eg. 32 is wrong, 32.0 is correct

  TYPES
  - Should be integer. options to choose will be mentioned there
  - eg: #define NL_CLOUD_TYPE 2

  Remember to rebuild the shader after making changes.
*/

/* Color correction */
#define NL_TONEMAP_TYPE 5              // 1:Exponential, 2:Reinhard, 3:Extended Reinhard, 4:ACES, 5:HSPE ACES, 6:HSPE Filmic
#define NL_GAMMA 2.0                   // 0.3 low ~ 2.0 high (HSPE uses 2.0)
//#define NL_EXPOSURE 1.3              // [toggle] 0.5 dark ~ 3.0 bright
#define NL_SATURATION 1.125            // [toggle] 0.0 grayscale ~ 4.0 super saturated (HSPE block saturation)
//#define NL_TINT                      // [toggle] enable light/dark tone tinting
#define NL_TINT_LOW  vec3(0.3,0.5,1.4) // color tint for dark tone
#define NL_TINT_HIGH vec3(1.4,0.7,0.3) // color tint for light tone

/* Lighting */
#define NL_SUNLIGHT_INTENSITY   3.3  // 1.0 weak ~ 5.0 bright
#define NL_TORCHLIGHT_INTENSITY 1.0  // 0.5 weak ~ 3.0 bright
#define NL_SHADOW_INTENSITY     0.7  // 0.0 no shadow ~ 1.0 strong shadow
#define NL_MIN_LIGHTING_BOOST   1.5  // 1.0 minimal lighting boost for dark areas ~ 3.0 brighter dark areas
//#define NL_BLINKING_TORCH  // [toggle] flickering light
#define NL_CLOUD_SHADOW      // [toggle] cloud shadow (simple clouds only)

/* Ambient light for nether/end */
#define NL_NETHER_AMBIENT vec3(3.0,2.16,1.89)
#define NL_END_AMBIENT    vec3(1.98,1.25,2.3)

/* Sun/moon light color */
#define NL_DAWN_SUNLIGHT_COL   vec3(1.0,0.4,0.1)
#define NL_NOON_SUNLIGHT_COL   vec3(1.0,0.75,0.57)
#define NL_NIGHT_MOONLIGHT_COL vec3(0.01,0.03,0.2)

/* Torch colors */
#define NL_OVERWORLD_TORCH_COL  vec3(1.0,0.52,0.18)
#define NL_UNDERWATER_TORCH_COL vec3(1.0,0.52,0.18)
#define NL_NETHER_TORCH_COL     vec3(1.0,0.52,0.18)
#define NL_END_TORCH_COL        vec3(1.0,0.52,0.18)

/* HSPE world light (ported from HSPE shader) */
#define NL_HSPE_LIGHT                // [toggle] use HSPE light/shadow/torch colors

// sun light color
#define NL_HSPE_LIGHT_DAY    vec3(1.4,1.2,1.1)
#define NL_HSPE_LIGHT_NIGHT  vec3(0.25,0.5,0.8)
#define NL_HSPE_LIGHT_DUSK   vec3(1.3,1.1,0.6)
#define NL_HSPE_LIGHT_DRAIN  vec3(0.8,0.8,0.8)
#define NL_HSPE_LIGHT_NRAIN  vec3(0.4,0.4,0.4)

// shadow color
#define NL_HSPE_SHADOW_DAY   vec3(0.5,0.65,0.82)
#define NL_HSPE_SHADOW_NIGHT vec3(0.2,0.2,0.35)
#define NL_HSPE_SHADOW_DUSK  vec3(0.45,0.75,1.0)
#define NL_HSPE_SHADOW_DRAIN vec3(0.6,0.6,0.6)
#define NL_HSPE_SHADOW_NRAIN vec3(0.3,0.3,0.3)
#define NL_HSPE_SHADOW_CAVE  vec3(0.15,0.25,0.35)

// torch color
#define NL_HSPE_TORCH_DAY    vec3(0.0,0.0,0.0)
#define NL_HSPE_TORCH_NIGHT  vec3(2.0,1.3,0.0)
#define NL_HSPE_TORCH_CAVE   vec3(3.0,1.35,0.0)

/* Fog - realistic atmospheric depth (beta) */
#define NL_FOG 1.3                // [toggle] 0.1 subtle ~ 1.0 blend with sky completely
#define NL_MIST_DENSITY 0.38      // 0.0 no mist ~ 1.0 misty
#define NL_END_FOG 1.6            // End-only extra fog thickness (1.0 = same as overworld)
#define NL_RAIN_MIST_OPACITY 0.4  // [toggle] 0.04 very subtle ~ 0.5 thick rain mist blow
#define NL_CLOUDY_FOG 0.25        // [toggle] 0.0 subtle - 0.8 dense fog clouds

/* Height fog - ground-hugging mist that thins with altitude */
#define NL_HEIGHT_FOG 0.5
#define NL_HEIGHT_FOG_START 56.0
#define NL_HEIGHT_FOG_RANGE 55.0

/* Sky */
#define NL_SKY_VOID_FACTOR     0.5
#define NL_SKY_VOID_DARKNESS   0.3
#define NL_SKY_RAIN_MIX_FACTOR 0.95

/* ATMO sunset palette (ESTN jaisa): dawn/dusk kohra-rang, 0.0 off */
#define NL_ATMO 0.5
#define NL_ATMO_SUNSET vec3(1.21,0.9025,0.49)  // HSPE DUSK_EDGE*c2 sunset haze

/* Sky colors - zenith=top, horizon=bottom */
#define NL_DAWN_ZENITH_COL   vec3(0.1,0.4,0.7)
#define NL_DAWN_HORIZON_COL  vec3(3.0,0.4,0.4)
#define NL_DAWN_EDGE_COL     vec3(2.0,0.8,0.8)
#define NL_DAY_ZENITH_COL    vec3(0.3,0.9,2.0)
#define NL_DAY_HORIZON_COL   vec3(1.0,1.6,1.8)
#define NL_DAY_EDGE_COL      vec3(1.44,1.56,1.62)
#define NL_NIGHT_ZENITH_COL  vec3(0.04,0.20,0.38)    // deep cyan zenith (beta)
#define NL_NIGHT_HORIZON_COL vec3(0.06,0.30,0.48)    // cyan horizon (beta)
#define NL_NIGHT_EDGE_COL    vec3(0.08,0.38,0.55)    // bright cyan edge (beta)
/* Raat ke clouds ka fixed rang - halka cyan (din me asar nahi) */
#define NL_NIGHT_CLOUD_COL   vec3(0.55,0.75,0.88)    // halka cyan night clouds
#define NL_NIGHT_CLOUD_BLEND 0.65  // raat me kitna cyan mix ho
#define NL_NIGHT_SKY_BRIGHTNESS 1.0
#define NL_RAIN_ZENITH_COL   vec3(0.47,0.51,0.56)
#define NL_RAIN_HORIZON_COL  vec3(0.6,0.6,0.6)

#define NL_END_ZENITH_COL    vec3(0.20,0.003,0.26)
#define NL_END_HORIZON_COL   vec3(0.78,0.025,0.85)

/* End black hole */
#define NL_END_BLACK_HOLE
#define NL_BH_COL_LOW  vec3(0.1,0.1,1.0)
#define NL_BH_COL_HIGH vec3(2.8,0.1,0.6)
#define NL_BH_DIST  1.8
#define NL_BH_SPEED 0.3

/* HSPE sky (ported from HSPE shader) */
#define NL_HSPE_SKY                 // [toggle] use HSPE sky colors + gradient
#define NL_HSPE_SUN_GLOW 0.5        // 0.0 no glow ~ 1.5 strong sun/moon glow

// day
#define NL_HSPE_DAY_EDGE      vec3(0.8,0.9,1.0)
#define NL_HSPE_DAY_HORIZON   vec3(0.2,0.65,0.95)
#define NL_HSPE_DAY_ZENITH    vec3(0.2,0.45,0.75)
// night (beta cyan - HSPE gradient ke saath)
#define NL_HSPE_NIGHT_EDGE    vec3(0.08,0.38,0.55)
#define NL_HSPE_NIGHT_HORIZON vec3(0.06,0.30,0.48)
#define NL_HSPE_NIGHT_ZENITH  vec3(0.04,0.20,0.38)
// dusk / dawn
#define NL_HSPE_DUSK_EDGE     vec3(1.1,0.95,0.7)
#define NL_HSPE_DUSK_HORIZON  vec3(1.0,1.0,1.0)
#define NL_HSPE_DUSK_ZENITH   vec3(0.2,0.7,0.9)
// day rain
#define NL_HSPE_DRAIN_EDGE    vec3(1.0,1.0,1.0)
#define NL_HSPE_DRAIN_HORIZON vec3(0.8,0.8,0.8)
#define NL_HSPE_DRAIN_ZENITH  vec3(0.5,0.5,0.5)
// night rain
#define NL_HSPE_NRAIN_EDGE    vec3(0.0,0.0,0.0)
#define NL_HSPE_NRAIN_HORIZON vec3(0.1,0.1,0.1)
#define NL_HSPE_NRAIN_ZENITH  vec3(0.3,0.3,0.3)


/* Rainbow */
#define NL_RAINBOW           // [toggle] enable rainbow in sky
#define NL_RAINBOW_CLEAR 0.0 // 0.3 subtle ~ 1.0 bright during clear weather
#define NL_RAINBOW_RAIN  0.45 // 0.3 subtle ~ 1.0 bright during rain weather

/* Ore glow intensity */
#define NL_GLOW_TEX 2.3           // 0.4 weak ~ 8.0 bright
#define NL_GLOW_SHIMMER 0.8       // [toggle] 0.1 subtle ~ 1.0 100% shimmer
#define NL_GLOW_SHIMMER_SPEED 0.9 // 0.5 slow - 2.0 fast
//#define NL_GLOW_LEAK 0.6        // [toggle] 0.08 subtle ~ 1.0 100% brightness of NL_GLOW_TEX

/* Waving */
#define NL_PLANTS_WAVE 0.055   // [toggle] 0.02 gentle ~ 0.4 violent
#define NL_LANTERN_WAVE 0.16   // [toggle] 0.05 subtle ~ 0.4 large swing
#define NL_WAVE_SPEED 2.8      // 0.5 slow wave ~ 5.0 very fast wave
//#define NL_EXTRA_PLANTS_WAVE // [toggle] !dont use! wave using texture coords (1.21.0 vanilla 2048x1024)
#define NL_WAVE_RANGE 14.0     // 6.0 upto 6 blocks ~ 32.0 upto 32 blocks away (slower)

/* Water */
#define NL_WATER_TRANSPARENCY 0.9 // 0.0 transparent ~ 1.0 normal
#define NL_WATER_BUMP 0.09        // 0.001 plain ~ 0.2 bumpy water
#define NL_WATER_WAVE_SPEED  0.8  // 0.2 calm ~ 2.0 turbulent
#define NL_WATER_TEX_OPACITY 0.3  // 0.0 plain water ~ 1.0 vanilla water texture
#define NL_WATER_SUN_DISC    0.35       // per-pixel sun disc mirror strength on water (0 = off)
#define NL_WATER_SUN_QUAD_TAN 0.1283    // tan of half the sun quad angular size
#define NL_WATER_MOON_QUAD_TAN 0.1167   // tan of half the moon quad angular size
#define NL_WATER_CLOUD_MIRROR 0.42      // soft cloud mirror on water (clouds reflection off hai)
#define NL_WATER_CLOUD_HEIGHT 192.0     // cloud height used by legacy samplers
#define NL_WATER_CLOUD_REFLECTION_DEPTH 2.0
#define NL_WATER_CLOUD_REFL_RIPPLE 0.012
#define NL_WATER_AURORA_MIRROR 0.55     // paani me aurora aks kitna tez
//#define NL_WATER_WAVE               // OFF: water wave band (flat paani)
//#define NL_WATER_REFL_MASK          // OFF: full reflection instead of patchy masked reflection
#define NL_WATER_TINT vec3(0.30,0.78,0.92)  // clear tropical blue
#define NL_WATER_GLITTER 1.25           // sun glitter tez

/* HSPE water (ported from HSPE shader) */
#define NL_HSPE_WATER                  // [toggle] use HSPE waves + reflection
#define NL_HSPE_WATER_HEIGHT 0.05      // 0.01 calm ~ 0.2 high waves
#define NL_HSPE_WATER_NORMAL_OFFSET 0.2 // wave sample distance
#define NL_HSPE_WATER_SPEED 1.0        // 0.2 slow ~ 3.0 fast
#define NL_HSPE_WATER_BASE 0.15        // 0.0 black water ~ 0.5 bright water
#define NL_HSPE_WATER_F0 0.25          // 0.04 less reflection ~ 0.5 mirror
#define NL_HSPE_WATER_ALPHA 0.7        // 0.3 transparent ~ 1.0 opaque

/* HSPE block depth + metal reflection (ported from HSPE shader) */
#define NL_HSPE_TEX_NORMAL             // [toggle] fake block depth from texture
#define NL_HSPE_TEX_NORMAL_BLUR        // [toggle] smooth depth (slower)
#define NL_HSPE_TEX_HEIGHT 2.0         // 1.0 deep ~ 5.0 flat
#define NL_HSPE_TEX_NORMAL_OFFSET 0.5  // sample distance in pixels
#define NL_HSPE_TEX_STRENGTH 0.35      // 0.1 subtle ~ 0.8 strong depth shading
#define NL_HSPE_METAL                  // [toggle] iron/gold/emerald block reflection
#define NL_HSPE_METALLIC 0.8           // 0.0 plain ~ 1.0 full metal tint

/* Underwater */
#define NL_UNDERWATER_BRIGHTNESS 0.85        // 0.0 dark ~ 3.0 bright
#define NL_CAUSTIC_INTENSITY 2.1             // 0.5 weak ~ 5.0 bright
#define NL_UNDERWATER_WAVE 0.11              // [toggle] 0.02 subtle ~ 0.6 trippy
#define NL_UNDERWATER_STREAKS 1.1            // [toggle] 0.8 subtle - 2.0 bright streaks from top
#define NL_UNDERWATER_TINT vec3(0.8,0.95,1.0) // fog tint color when underwater

/* Cloud type - mesh clouds off, sky-dome dome clouds draw karte hain (beta) */
#define NL_CLOUD_TYPE 0 // 0:vanilla, 1:soft, 2:rounded, 3:realistic, 4:HSPE (needs NL_HSPE_SKY)

/* Vanilla cloud settings - make sure to remove clouds.png when using this */
#define NL_CLOUD0_THICKNESS 2.1      // 0.5 slim ~ 8.0 fat
#define NL_CLOUD0_RAIN_THICKNESS 4.0 // 0.5 slim ~ 8.0 fat
#define NL_CLOUD0_OPACITY 0.0        // disabled: sky-dome clouds draw karte hain
//#define NL_CLOUD0_MULTILAYER       // [toggle] extra cloud layer

/* Soft cloud settings */
#define NL_CLOUD1_SCALE vec2(0.016, 0.022) // 0.003 large ~ 0.2 tiny
#define NL_CLOUD1_DEPTH 1.3                // 0.0 no bump ~ 10.0 large bumps
#define NL_CLOUD1_SPEED 0.04               // 0.0 static ~ 0.4 fast moving
#define NL_CLOUD1_DENSITY 0.54             // 0.1 less clouds ~ 0.8 more clouds
#define NL_CLOUD1_OPACITY 0.9              // 0.0 invisible ~ 1.0 opaque

/* Rounded cloud settings */
#define NL_CLOUD2_THICKNESS 2.1            // 0.5 slim ~ 5.0 fat
#define NL_CLOUD2_RAIN_THICKNESS 2.5       // 0.5 slim ~ 5.0 fat
#define NL_CLOUD2_STEPS 5                  // 3 low quality ~ 16 high quality
#define NL_CLOUD2_SCALE vec2(0.033, 0.033) // 0.003 large ~ 0.3 tiny
#define NL_CLOUD2_SHAPE vec2(0.5, 0.4)     // 0.0 round ~ 1.0 box vec2(horizontal shape, vertical shape)
#define NL_CLOUD2_DENSITY 25.0             // 1.0 blurry ~ 100.0 sharp
#define NL_CLOUD2_VELOCITY 0.8             // 0.0 static ~ 4.0 very fast
//#define NL_CLOUD2_LAYER2                      // [toggle] extra cloud layer
#define NL_CLOUD2_LAYER2_OFFSET 143.0           // 30.0 near ~ 300.0 very high
#define NL_CLOUD2_LAYER2_THICKNESS 2.5          // 0.7 slim ~ 5.0 fat
#define NL_CLOUD2_LAYER2_RAIN_THICKNESS 3.0     // 0.7 slim ~ 5.0 fat
#define NL_CLOUD2_LAYER2_STEPS 3                // 3 low quality ~ 16 high quality
#define NL_CLOUD2_LAYER2_SCALE vec2(0.03, 0.03) // 0.003 large ~ 0.3 tiny
#define NL_CLOUD2_LAYER2_SHAPE vec2(0.5, 0.4)   // 0.0 round ~ 1.0 box vec2(horizontal shape, vertical shape)
#define NL_CLOUD2_LAYER2_DENSITY 25.0           // 1.0 blurry ~ 100.0 sharp
#define NL_CLOUD2_LAYER2_VELOCITY 0.8           // 0.0 static ~ 4.0 very fast

/* Realistic cloud settings */
#define NL_CLOUD3_SCALE vec2(0.03, 0.03) // 0.003 large ~ 1.0 tiny
#define NL_CLOUD3_SPEED 0.005            // 0.0 static ~ 4.0 fast moving
#define NL_CLOUD3_SHADOW 0.9             // 0.1 subtle ~ 1.0 dark
#define NL_CLOUD3_SHADOW_OFFSET 0.3      // 0.05 minimal ~ 1.0 large

/* Realistic texture clouds (y.png) */
#define NL_REAL_CLOUD_SCALE 0.5       // plane project scale

/* Sky-following clouds - din/sunset/raat/barish rang (Sky dome + mirror) */
#define NL_CLOUD_SKY_TINT
#define NL_CLOUD_DUSK_STRENGTH 0.8     // sunset narangi kitni tez (0 = band)

/* Sky-dome procedural clouds (vibrant) - NO_REFLECTIONS/Low me on */
//#define NL_SKY_CLOUDS
#define NL_SKY_CLOUD_SPEED 0.09    // drift speed (cell units/sec)
#define NL_SKY_CLOUD_DIR vec2(1.0, 0.35) // wind direction
#define NL_SKY_CLOUD_OPACITY 0.9   // max cloud opacity

/* HSPE cloud settings */
#define NL_HSPE_CLOUD_QUALITY 6        // 3 fast ~ 7 best quality
#define NL_HSPE_CLOUD_SCALE 1.0        // 0.5 large clouds ~ 2.0 small clouds
#define NL_HSPE_CLOUD_DENSITY_CLEAR 2.2 // higher = less clouds (clear weather)
#define NL_HSPE_CLOUD_DENSITY_RAIN 1.7  // higher = less clouds (rain)
#define NL_HSPE_CLOUD_SOFTNESS 3.0     // cloud softness
#define NL_HSPE_CLOUD_NOISE 3.0        // noise amplitude
#define NL_HSPE_CLOUD_SPEED 0.1        // 0.0 static ~ 0.4 fast

// cloud light color
#define NL_HSPE_CLOUD_DAY    vec3(1.1,1.1,1.1)
#define NL_HSPE_CLOUD_NIGHT  vec3(0.15,0.3,0.5)
#define NL_HSPE_CLOUD_DUSK   vec3(1.0,0.85,0.5)
#define NL_HSPE_CLOUD_DRAIN  vec3(0.7,0.7,0.7)
#define NL_HSPE_CLOUD_NRAIN  vec3(0.3,0.3,0.3)

// cloud shadow color
#define NL_HSPE_CSHADOW_DAY    vec3(0.8,0.8,0.825)
#define NL_HSPE_CSHADOW_NIGHT  vec3(0.1,0.2,0.3)
#define NL_HSPE_CSHADOW_DUSK   vec3(0.5,0.65,0.8)
#define NL_HSPE_CSHADOW_DRAIN  vec3(0.5,0.5,0.5)
#define NL_HSPE_CSHADOW_NRAIN  vec3(0.2,0.2,0.2)

/* Aurora settings (beta - texture curtain + water mirror) */
#define NL_AURORA 1.3           // [toggle] 0.4 dim ~ 4.0 very bright
#define NL_AURORA_TEX 1.0          // original
#define NL_AURORA_TEX_LAYERS 10    // curtain layers - main cost knob (2 taps each)
#define NL_AURORA_TEX_COL1 vec3(0.6,7.5,9.5)  // original cyan
#define NL_AURORA_TEX_COL2 vec3(0.2,4.2,8.0)  // original teal-blue
#define NL_AURORA_VELOCITY 0.03 // 0.0 static ~ 0.3 very fast
#define NL_AURORA_SCALE 0.04    // 0.002 large ~ 0.4 tiny
#define NL_AURORA_WIDTH 0.18    // 0.04 thin line ~ 0.4 thick lines
#define NL_AURORA_COL1 vec3(0.2,0.6,1.0)   // original sky blue
#define NL_AURORA_COL2 vec3(0.35,0.85,1.0)  // original cyan-blue

#define NL_CLOUD_AURORA_REFLECTION // [toggle] simple clouds/aurora reflection

/* Shooting star - 4 streaks fire together every NL_SHOOTING_STAR_PERIOD seconds */
#define NL_SHOOTING_STAR 1.0        // [toggle] 0.2 dim ~ 1.0 bright
#define NL_SHOOTING_STAR_PERIOD 5.0 // 0.4 fast ~ 12.0 slow (12 secs)
#define NL_SHOOTING_STAR_DELAY 0.0  // 0.0 no delay ~ 100.0 long time (100 secs)

/* Galaxy */
//#define NL_GALAXY_STARS 2.0           // [toggle] 0.1 dim - 4.0 bright
#define NL_GALAXY_VIBRANCE 0.7          // 0.0 white - 1.0 colorful
#define NL_GALAXY_SPEED 0.03            // 0.01 slow motion - 0.2 fast motion
#define NL_GALAXY_DAY_VISIBILITY 0.0    // 0.0 invisible - 1.0 visible

/* Chunk loading slide in animation */
//#define NL_CHUNK_LOAD_ANIM 100.0 // [toggle] -600.0 fall from top ~ 600.0 rise from bottom

/* Sun/Moon (beta) */
#define NL_SUN_SIZE  2.75          // 0.3 tiny ~ 4.0 massive
#define NL_MOON_SIZE 1.7           // 0.3 tiny ~ 4.0 massive
#define NL_SUN_PATH_YAW    0.0     // seedha rasta (centered)
#define NL_MOON_PATH_YAW   17.0
#define NL_SUN_PATH_TILT   0.0     // 0.0 = suraj seedha upar-neeche
#define NL_MOON_PATH_TILT -28.0
#define NL_SUN_TILT        0.0     // 0.0 = no tilt
#define NL_MOON_TILT       90.0    // 0.0 no tilt ~ 90.0 tilt of 90 degrees

/* Godrays - strong volumetric light shafts (beta) */
#define NL_GODRAY 0.25 // [toggle] 0.1 subtle ~ 0.8 strong
#define NL_GODRAY_MULTIPLICATOR 2.0 // final tez (1.0 normal)
#define NL_GODRAY_SENSITIVITY   0.03 // halke rays threshold (kam = zyada rays)

/* PBR block reflection (from "block reflection V3") - fragment-stage
   normal-mapped, TBN-distorted, Cook-Torrance mirror on smooth blocks (beta) */
#define NL_PBR_BLOCK_REFL              // ON: enable V3-style PBR block reflection
#define NL_PBR_ROUGHNESS 0.15          // tez mirror
#define NL_PBR_METALLIC 0.0            // 0 keeps texture bright
#define NL_PBR_F0 vec3(0.35,0.35,0.36) // strong reflectance
#define NL_PBR_SUNCOLOR vec3(1.0,0.95,0.85) // neutral white glint, not golden
#define NL_PBR_SPEC_INTENSITY 1.0      // tez glint
#define NL_PBR_SPEC_CLAMP 1.5          // glint cap khula
#define NL_PBR_NORMAL_STRENGTH 0.7     // bump strength
#define NL_PBR_RAIN_BOOST 1.4          // extra mirror strength when raining (wet ground)
#define NL_PBR_ATLAS_TEXEL vec2(0.0009765625, 0.001953125) // 1024x512 terrain atlas

/* Ground reflection - reflective wet ground (beta) */
#define NL_GROUND_REFL 1.2              // ON: strong mirror-like reflection
#define NL_GROUND_RAIN_WETNESS 1.6     // strong puddle reflections while raining
#define NL_GROUND_RAIN_PUDDLES 0.9     // more scattered, natural puddle shapes

/* Rain reflection - strong wet/puddle mirror only while raining (beta) */
#define NL_RAIN_REFL_STRENGTH 1.5      // 0 = off, higher = stronger wet ground mirror

/* End reflection boost - End me mirror aur tez (beta) */
#define NL_END_REFL_BOOST 2.0          // 1.0 = no change, zyada = tez pink mirror

/* Rain drop ripples on block surfaces (top faces, only while raining) */
#define NL_RAIN_RIPPLE 1.0          // [toggle] 0.3 subtle ~ 1.5 strong ripples
#define NL_RAIN_RIPPLE_SCALE 3.0    // 1.5 big rings ~ 6.0 tiny rings
#define NL_RAIN_RIPPLE_SPEED 0.3    // 0.1 slow ~ 0.8 fast
#define NL_RAIN_RIPPLE_RADIUS 1     // 1 = 9 neighbour cells (fast), 2 = 25 cells (denser, slower)
#define NL_RAIN_RIPPLE_PATCHY 1.0   // 0.0 ripples everywhere ~ 1.0 patchy wet areas

/* Entity (Actor, ItemInHand) */
#define NL_ENTITY_BRIGHTNESS     0.68 // 0.1 dark ~ 1.6 bright
#define NL_ENTITY_EDGE_HIGHLIGHT 0.42 // [toggle] 0.0 no highlight ~ 1.6 bright highlight

/* Weather particles */
#define NL_WEATHER_SPECK 0.5         // [toggle] 0.0 vanilla texture ~ 1.0 soft speck
#define NL_WEATHER_RAIN_SLANT 5.0    // 1.0 minimal ~ 8.0 violent
#define NL_WEATHER_PARTICLE_SIZE 0.85 // 0.5 tiny ~ 4.0 large

/* Lava effects */
#define NL_LAVA_NOISE            // [toggle] darken lava in certain regions
//#define NL_LAVA_NOISE_BUMP 0.2 // [toggle] 0.1 subtle ~ 0.8 massive waves
#define NL_LAVA_NOISE_SPEED 0.2  // 0.0 still ~ 0.8 fast

/*
  NEWB SHADER SUBPACK CONFIG
  This part contains custom configuration options for each subpack.

  If a value is already defined,
  then you must undefine it before modifying:
  eg: #undef OPTION_NAME

  Subpack names and flags are inside `pack_config.toml`.
  Build tool will enable corresponding flags when compiling.
*/

#ifdef LITE
  #define NO_WAVE
  #undef NL_GLOW_SHIMMER
  #undef NL_LAVA_NOISE
  #undef NL_WEATHER_SPECK
  #undef NL_SHOOTING_STAR
  #undef NL_CLOUD_AURORA_REFLECTION
  #undef NL_UNDERWATER_STREAKS
  #undef NL_RAIN_MIST_OPACITY
  #undef NL_CLOUDY_FOG
  #undef NL_ENTITY_EDGE_HIGHLIGHT
  #undef NL_PBR_BLOCK_REFL
  // halve the aurora curtain: 2 texture taps per layer per sky pixel
  #undef NL_AURORA_TEX_LAYERS
  #define NL_AURORA_TEX_LAYERS 5
  #define NL_NO_WATER_CLOUD_REFL
#endif

#ifdef NO_WAVE_NO_FOG
  #define NO_WAVE
  #define NO_FOG
#endif

#ifdef NO_FOG
  #undef NL_FOG
#endif

#ifdef NO_WAVE
  #undef NL_PLANTS_WAVE
  #undef NL_LANTERN_WAVE
  #undef NL_UNDERWATER_WAVE
  #undef NL_WATER_WAVE
  #undef NL_RAIN_MIST_OPACITY
#endif

#ifdef CHUNK_ANIM
  #define NL_CHUNK_LOAD_ANIM 100.0
#endif

/* ---- Beta subpacks ---- */
#ifdef MEDIUM
  // cloud + aurora mirror on water band (heaviest two reflection layers)
  #define NL_NO_WATER_CLOUD_AURORA_REFL
#endif

#if defined(VIBRANT_CLOUD) || defined(NO_REFLECTIONS)
  #undef NL_CLOUD_TYPE
  #define NL_CLOUD_TYPE 0
  #define NL_SKY_CLOUDS
#endif

#ifdef NO_REFLECTIONS
  #undef NL_GROUND_REFL
  #undef NL_PBR_BLOCK_REFL
  #undef NL_RAIN_REFL_STRENGTH
  #define NL_NO_GROUND_REFL
  #define NL_NO_WATER_CLOUD_REFL
#endif

#ifdef EDITOR_CLOUDS
  #undef NL_CLOUD_TYPE
  #define NL_CLOUD_TYPE 2
#endif

#endif
