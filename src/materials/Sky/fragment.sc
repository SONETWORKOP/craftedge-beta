#ifndef INSTANCING
  $input v_worldPos, v_underwaterRainTimeDay
#endif

#include <bgfx_shader.sh>

#ifndef INSTANCING
  SAMPLER2D_AUTOREG(s_NoiseTexture);
  // noise texture driving the aurora curtain - declared before main.sh so the
  // shared aurora function in newb/functions/clouds.h (included by main.sh)
  // can sample it in both the Sky dome and the RenderChunk water mirror.
  SAMPLER2D_AUTOREG(s_NoiseVoxel);
  #define NL_ROUNDED_CLOUDS
  #define NL_AURORA_REFLECTION
  #include <newb/main.sh>
  uniform vec4 TimeOfDay;
  uniform vec4 FogColor;
  uniform vec4 FogAndDistanceControl;
  uniform vec4 CameraPosition;

  // the aurora borealis curtain lives in newb/functions/clouds.h
  // (nlAuroraBorealis) so the water mirror in RenderChunk draws the exact
  // same shape.
  #ifdef EDITOR_CLOUDS
  // ok.txt FULL CODE PORT (Download/ok.txt 102 lines, STEPS 5) - Sky-dome version.
  // ok.txt Shadertoy original: resolution/time/touch + gl_FragCoord + sky-gradient + sun + 5-step volumetric.
  // Yahan 1:1 port: hash13/vnoise/density/raymarch/light/horizon-fade same, sirf rd=viewDir, t=dome-time,
  // sky=nlRenderSky (Minecraft din/sunset/raat/barish), jitter=rd.xy (dome me fragCoord nahi).
  #define EDITOR_STEPS 5
  #define EDITOR_CB 80.0
  #define EDITOR_CT 140.0
  #define EDITOR_COV 0.5
  #define EDITOR_SPEED 2.0
  #define EDITOR_H0 0.06
  #define EDITOR_H1 0.32
  float editorHash13(vec3 p) {
    p = mod(p, 50.0);
    p = fract(p * 0.3183099 + vec3(0.1, 0.2, 0.3));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
  }
  float editorVNoise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    float n000 = editorHash13(p);
    float n100 = editorHash13(p + vec3(1.0, 0.0, 0.0));
    float n010 = editorHash13(p + vec3(0.0, 1.0, 0.0));
    float n110 = editorHash13(p + vec3(1.0, 1.0, 0.0));
    float n001 = editorHash13(p + vec3(0.0, 0.0, 1.0));
    float n101 = editorHash13(p + vec3(1.0, 0.0, 1.0));
    float n011 = editorHash13(p + vec3(0.0, 1.0, 1.0));
    float n111 = editorHash13(p + vec3(1.0, 1.0, 1.0));
    return mix(mix(mix(n000, n100, f.x), mix(n010, n110, f.x), f.y),
               mix(mix(n001, n101, f.x), mix(n011, n111, f.x), f.y), f.z);
  }
  float editorDensity(vec3 p, float t) {
    float h = (p.y - EDITOR_CB) / (EDITOR_CT - EDITOR_CB);
    float shape = smoothstep(0.0, 0.2, h) * (1.0 - smoothstep(0.5, 1.0, h));
    vec3 q = p * 0.02 + vec3(t * EDITOR_SPEED * 0.01, 0.0, 0.0);
    float n = editorVNoise(q) * 0.68 + editorVNoise(q * 2.4 + vec3(3.1, 1.7, 5.3)) * 0.32;
    return clamp((n - (1.0 - EDITOR_COV)) * shape * 4.0, 0.0, 1.0);
  }
  #endif
#endif

void main() {
  #ifndef INSTANCING
    vec3 viewDir = normalize(v_worldPos);

    nl_environment env;
    env.end = false;
    env.nether = false;
    env.underwater = v_underwaterRainTimeDay.x > 0.5;
    env.rainFactor = v_underwaterRainTimeDay.y;
    env.dayFactor = v_underwaterRainTimeDay.w;
    env.fogCol = FogColor.rgb;
    env = calculateSunParams(env, TimeOfDay.x);

    nl_skycolor skycol = nlOverworldSkyColors(env);

    vec3 skyColor = nlRenderSky(skycol, env, -viewDir, v_underwaterRainTimeDay.z, true);
    #ifdef NL_SHOOTING_STAR
      skyColor += NL_SHOOTING_STAR*nlRenderShootingStar(viewDir, env.fogCol, v_underwaterRainTimeDay.z);
    #endif
    #ifdef NL_GALAXY_STARS
      skyColor += NL_GALAXY_STARS*nlRenderGalaxy(viewDir, env.fogCol, env, v_underwaterRainTimeDay.z);
    #endif

    // aurora borealis (night only, hidden by rain and underwater)
    if (!env.underwater) {
      float VdotU = clamp(viewDir.y, 0.0, 1.0);
      float nightFactor = 1.0 - smoothstep(-0.02, 0.32, env.dayFactor);
      if (nightFactor > 0.001 && VdotU > 0.15) {
        float dither = fract(sin(dot(viewDir.xy, vec2(12.9898, 78.233))) * 43758.5453);
        vec3 aurora = nlAuroraBorealis(
          viewDir, VdotU, dither, env.rainFactor,
          CameraPosition.xz, v_underwaterRainTimeDay.z
        );
        skyColor += NL_AURORA_TEX*aurora*nightFactor;
      }
    }

    #ifdef EDITOR_CLOUDS
    // ok.txt main() FULL PORT - 5-step raymarch + horizon-fade.
    // ok.txt: rd from uv, sun fixed, sky-gradient + pow(sd,32)*0.6, t0/t1 with max(rd.y,0.08), jitter fragCoord,
    // d>0.02, light=exp(-d*2.2), c=mix(dark,bright,light)+sun*pow(sd,6)*(0.25+0.35*light),
    // a=1-exp(-d*dt*0.08), T break 0.04, fade smoothstep(0.06,0.32), col=mix(col,acc+sky*T,fade).
    // Yahan: rd=viewDir, sun=sunDir/moonDir, t=dome-time, sky=nlRenderSky, jitter=rd.xy. Baaki 1:1.
    if (!env.underwater && viewDir.y > EDITOR_H0 * 0.5) {
      vec3 rd = viewDir;
      vec3 sunDir = env.sunDir.y > 0.0 ? env.sunDir : env.moonDir;
      float sd = max(dot(rd, sunDir), 0.0);
      float t = v_underwaterRainTimeDay.z;
      float t0 = EDITOR_CB / max(rd.y, 0.08);
      float t1 = EDITOR_CT / max(rd.y, 0.08);
      float dt = (t1 - t0) / float(EDITOR_STEPS);
      float jitter = fract(sin(dot(rd.xy, vec2(12.9898, 78.233))) * 43758.5453);
      float marchT = t0 + dt * jitter;
      float T = 1.0;
      vec3 acc = vec3(0.0);
      for (int i = 0; i < 5; i++) {
        if (i >= EDITOR_STEPS) break;
        vec3 p = rd * marchT;
        float d = editorDensity(p, t);
        if (d > 0.02) {
          float light = exp(-d * 2.2);
          vec3 c = mix(vec3(0.45, 0.52, 0.65), vec3(1.0, 0.97, 0.9), light);
          c += vec3(1.0, 0.9, 0.7) * pow(max(sd, 0.001), 6.0) * (0.25 + 0.35 * light);
          float a = 1.0 - exp(-d * dt * 0.08);
          acc += c * a * T;
          T *= 1.0 - a;
          if (T < 0.04) break;
        }
        marchT += dt;
      }
      // Minecraft sky-rang me dhalo: din/sunset/raat/barish
      acc = nlSkyCloudTint(acc, skycol.horizon, env.dayFactor, env.rainFactor);
      vec3 vol = acc + skyColor.rgb * T;
      float fade = smoothstep(EDITOR_H0, EDITOR_H1, rd.y);
      skyColor.rgb = mix(skyColor.rgb, vol, fade);
    }
    #else
    // EDITOR_CLOUDS off: neeche wale dome-clouds
    // REALISTIC_CLOUDS subpack: y.png noise-texture wale 8-layer badal.
    // Chunne par baaki dome-clouds nahi bante (double-draw nahi).
    #ifdef REALISTIC_CLOUDS
      if (!env.underwater && viewDir.y > 0.001) {
        vec3 rcSun = env.sunDir.y > 0.0 ? env.sunDir : env.moonDir;
        vec4 rc = nlRealClouds(viewDir, rcSun, v_underwaterRainTimeDay.z);
        rc.rgb = nlSkyCloudTint(rc.rgb, skycol.horizon, env.dayFactor, env.rainFactor);
        float rcOp = smoothstep(0.03, 0.3, viewDir.y);
        skyColor.rgb = mix(skyColor.rgb, rc.rgb, clamp(rc.a*rcOp, 0.0, 1.0));
      }
    #else
    // procedural vibrant clouds (cheap, no texture)
    #ifdef NL_SKY_CLOUDS
      if (!env.underwater && viewDir.y > 0.001) {
        float scale = 0.8 / viewDir.y;
        float cloudA = nlVibrantClouds(viewDir.xz*scale, 0.004*scale, v_underwaterRainTimeDay.z);
        cloudA *= smoothstep(0.05, 0.35, viewDir.y);   // horizon fade
        cloudA *= NL_SKY_CLOUD_OPACITY;

        // cloud color POORA sky-rang me: din safed, sunset narangi,
        // raat gehra neela, barish grey
        vec3 cloudCol = nlVibrantCloudColor(env.dayFactor, sunLightTint(env.dayFactor, env.rainFactor), skycol.horizon, env.rainFactor);
        skyColor.rgb = mix(skyColor.rgb, cloudCol, clamp(cloudA, 0.0, 1.0));
      }
      // Medium second layer (my-style top echo, ESTN DCLOUDS jaisa):
      // sparse bade puffs vibrant base ke upar.
      #ifdef MEDIUM
      if (!env.underwater && viewDir.y > 0.001) {
        float scale2 = 0.3 / viewDir.y;
        float cloudA2 = nlVibrantClouds(viewDir.xz*scale2 + vec2(3.7,9.1), 0.004*scale2, v_underwaterRainTimeDay.z + 40.0);
        cloudA2 *= smoothstep(0.05, 0.35, viewDir.y);
        cloudA2 = clamp((cloudA2-0.55)*1.6, 0.0, 1.0)*0.7;
        float dayLight2 = clamp(env.dayFactor*0.5 + 0.5, 0.0, 1.0);
        vec3 topCol = mix(vec3(0.60,0.66,0.75), vec3(1.08,1.06,1.02), clamp(cloudA2, 0.0, 1.0));
        topCol *= 0.12 + 0.88*dayLight2;
        // POORA sky-rang: horizon dye + sunset kiss + raat dark + barish
        topCol *= skycol.horizon*0.85 + vec3_splat(0.35);
        #ifdef NL_ATMO_SUNSET
          topCol = mix(topCol, topCol*(NL_ATMO_SUNSET*1.2 + vec3(0.35,0.12,0.10)), nlDuskF(env.dayFactor)*0.7);
        #endif
        topCol = mix(topCol, NL_NIGHT_CLOUD_COL*0.7, nlNightF(env.dayFactor));
        topCol *= 1.0 - 0.45*(1.0 - smoothstep(-0.08, 0.12, env.dayFactor));
        topCol *= 1.0 - 0.30*env.rainFactor;
        skyColor.rgb = mix(skyColor.rgb, topCol, clamp(cloudA2, 0.0, 1.0)*NL_SKY_CLOUD_OPACITY);
      }
      #endif
    #else
      // Low (NO_REFLECTIONS): dome clean - box-mesh ESTN clouds cover karte
      // hain (double-draw nahi). Baaki sab rounded dome.
      #ifndef NO_REFLECTIONS
      // MEDIUM: original newb rounded clouds (stock NL_CLOUD2_* settings) -
      // raymarched, sky-rang me range hue
      #ifdef MEDIUM
      if (!env.underwater && viewDir.y > 0.001) {
        vec4 medClouds = renderCloudsRounded(
          viewDir, CameraPosition.xyz, env.rainFactor, v_underwaterRainTimeDay.z,
          skycol.horizon, skycol.zenith,
          NL_CLOUD2_STEPS, NL_CLOUD2_THICKNESS, NL_CLOUD2_RAIN_THICKNESS,
          NL_CLOUD2_VELOCITY, NL_CLOUD2_SCALE, NL_CLOUD2_DENSITY, NL_CLOUD2_SHAPE
        );
        medClouds.rgb = nlSkyCloudTint(medClouds.rgb, skycol.horizon, env.dayFactor, env.rainFactor);
        float medOp = smoothstep(0.05, 0.3, viewDir.y);
        skyColor.rgb = mix(skyColor.rgb, medClouds.rgb, clamp(medClouds.a*medOp, 0.0, 1.0));
      }
      // default (High): texture rounded dome
      #else
      // raymarched rounded clouds (RoundedClouds from cloud.txt), the default
      // replacement for the old blocky box clouds
      if (!env.underwater && viewDir.y > 0.001) {
        float jitter = fract(sin(dot(viewDir.xy, vec2(12.9898, 78.233))) * 43758.5453);
        vec4 clouds = nlRoundedClouds(viewDir, v_underwaterRainTimeDay.z, jitter);
        // POORA sky-rang + dark: sunset narangi, raat gehra, barish grey
        clouds.rgb = nlSkyCloudTint(clouds.rgb, skycol.horizon, env.dayFactor, env.rainFactor);
        float opacity = smoothstep(0.1, 0.3, viewDir.y);
        float cloudMask = clouds.a * 0.5 * opacity;
        skyColor.rgb = mix(skyColor.rgb, clouds.rgb, cloudMask);
      }
      #endif // MEDIUM
      #endif // NO_REFLECTIONS
    #endif // NL_SKY_CLOUDS
    #endif // REALISTIC_CLOUDS
    #endif // EDITOR_CLOUDS

    skyColor = colorCorrection(skyColor);

    gl_FragColor = vec4(skyColor, 1.0);
  #else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  #endif
}
