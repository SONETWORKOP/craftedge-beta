#ifndef FOG_H
#define FOG_H

float nlRenderFogFade(float relativeDist, vec3 FOG_COLOR, vec2 FOG_CONTROL, bool isEnd) {
  #ifdef NL_FOG
    float fade = smoothstep(FOG_CONTROL.x, FOG_CONTROL.y, relativeDist);

    // exponential distance fog - closer to how real atmospheric haze builds up
    float expFade = 1.0 - exp(-relativeDist*relativeDist*0.9);
    fade = max(fade, expFade*0.35);

    // misty effect
    float density = NL_MIST_DENSITY*(19.0 - 18.0*FOG_COLOR.g);
    fade += (1.0-fade)*(0.3-0.3*exp(-relativeDist*relativeDist*density));

    float fog = NL_FOG * fade;
    #ifdef NL_END_FOG
      fog *= isEnd ? NL_END_FOG : 1.0;
    #endif

    return clamp(fog, 0.0, 1.0);
  #else
    return 0.0;
  #endif
}

float nlRenderHeightFog(float fade, float worldHeight, float relativeDist) {
  #if defined(NL_FOG) && defined(NL_HEIGHT_FOG)
    // thicker fog near ground, thins out with altitude (valley mist look)
    float heightFactor = 1.0 - smoothstep(NL_HEIGHT_FOG_START, NL_HEIGHT_FOG_START+NL_HEIGHT_FOG_RANGE, worldHeight);
    fade += (1.0-fade)*heightFactor*NL_HEIGHT_FOG*relativeDist;
    return clamp(fade, 0.0, 1.0);
  #else
    return fade;
  #endif
}

float nlRenderGodRayIntensity(vec3 cPos, vec3 worldPos, float t, vec2 uv1, float relativeDist, vec3 FOG_COLOR) {
  vec3 offset = cPos - 16.0*fract(worldPos*0.0625);
  offset = abs(2.0*fract(offset*0.0625)-1.0);
  offset = offset*offset*(3.0-2.0*offset);

  vec3 nrmof = normalize(worldPos);
  float u = nrmof.z/length(nrmof.zy);
  float diff = dot(offset,vec3(0.1,0.2,1.0)) + 0.07*t;
  float mask = nrmof.x*nrmof.x;

  float vol = sin(7.0*u + 1.5*diff)*sin(3.0*u + diff);
  vol *= vol*mask*uv1.y*(1.0-mask*mask);
  vol *= relativeDist*relativeDist;

  // dawn/dusk only - back to original Newb Shader behavior
  vol *= clamp(3.0*(FOG_COLOR.r-FOG_COLOR.b), 0.0, 1.0);

  vol = smoothstep(0.0, 0.1, vol);
  return vol;
}

vec3 nlGodRayTint(vec3 FOG_COLOR) {
  // warm yellow-gold tint for light shafts, blending toward deeper orange
  // at dawn/dusk when FOG_COLOR itself is already warm (higher red-blue diff)
  float dawnDusk = clamp(3.0*(FOG_COLOR.r-FOG_COLOR.b), 0.0, 1.0);
  vec3 dayRayTint = vec3(1.0, 0.92, 0.55);
  vec3 dawnRayTint = vec3(1.0, 0.75, 0.35);
  return mix(dayRayTint, dawnRayTint, dawnDusk);
}

// ---- Fragment godrays (realistic sun shafts, beta) ----
// viewDirS = surface->camera, sunDir = real suraj dir, sunUp = suraj upar (0/1).
// cone: suraj ki taraf dekhne par tez; bands: animated dhaariyan;
// barish me band; door kohre me tez. Underwater gate caller kare.
float nlGodRayFrag(
  vec3 viewDirS, vec3 sunDir, float sunUp, highp float t,
  float rainFactor, float dayFactor, float fogAmt
) {
  float cone = pow(clamp(dot(-viewDirS, sunDir), 0.0, 1.0), 4.0);
  if (cone < 0.003) return 0.0;
  // suraj-axis basis taaki dhaariyan ghumein nahi
  vec3 upRef = abs(sunDir.y) > 0.9 ? vec3(1.0,0.0,0.0) : vec3(0.0,1.0,0.0);
  vec3 bu = normalize(cross(sunDir, upRef));
  vec3 bv = cross(sunDir, bu);
  float u = dot(viewDirS, bu);
  float v = dot(viewDirS, bv);
  float bands = 0.5 + 0.5*sin(u*46.0 + v*39.0 + t*0.7)*sin(u*31.0 - v*44.0 - t*0.5);
  bands = smoothstep(0.30, 0.92, bands);
  float dawnDusk = clamp(1.0 - abs(dayFactor)*2.0, 0.0, 1.0);
  dawnDusk *= dawnDusk;
  float timeAmt = (0.22 + 0.78*dawnDusk)*sunUp;
  float distFade = smoothstep(0.03, 0.45, fogAmt);
  float rainOff = 1.0 - clamp(rainFactor, 0.0, 1.0);
  return cone*(0.30 + 0.70*bands)*timeAmt*distFade*rainOff;
}

// sunset/sunrise tint: yellow <-> pink-purple animated, din me warm white
vec3 nlGodRayTintFrag(vec3 viewDirS, float dayFactor, highp float t) {
  float dawnDusk = clamp(1.0 - abs(dayFactor)*2.0, 0.0, 1.0);
  dawnDusk *= dawnDusk;
  vec3 dayTint = vec3(1.0, 0.95, 0.78);
  float hueShift = 0.5 + 0.5*sin(t*0.35 + viewDirS.x*9.0 + viewDirS.y*7.0);
  vec3 duskTint = mix(vec3(1.0, 0.70, 0.28), vec3(0.88, 0.45, 0.80), hueShift);
  return mix(dayTint, duskTint, dawnDusk);
}

// ---- fog.txt port (Download/fog.txt) ----
// Sunset glow cue 0..1 from fog color (godrays wali same trick).
float nlSunsetGlow(vec3 fogColor) {
  return clamp(3.0*(fogColor.r - fogColor.b), 0.0, 1.0);
}
// fog.txt getFog(): linear+quadratic exp fog with sunset/rain/nether/
// underwater rules. Engine-scale numbers pack space me map kiye:
//   fogDensity = relativeDist (0..1) space, *15.0 -> *6.0 (clear din me
//   pack fade jeette, sunset/rain me snippet jeete - change dikhega),
//   nether 30.0 -> 3.0 (30.0 pack me sab kuch 100% fog kar deta).
// Underwater exp(-dist*12.0) exact rakha (self-normalizing hai).
float nlGetFog(float dist, vec2 fogDensity, float rain, float sunsetSunrise, bool underwater, bool nether) {
  float fogStrength = mix(0.4, 0.6, sunsetSunrise);
  fogStrength = mix(fogStrength, 1.2, rain);
  if (nether) fogStrength = 3.0;

  float q1 = dist * fogDensity.x * fogStrength;
  float q2 = dist * dist * fogDensity.y * fogStrength * fogStrength;
  float fogFactor = 1.0 - exp(-(q1 + q2) * 6.0);

  if (underwater) {
    fogFactor = 1.0 - exp(-dist * 12.0);
  }

  return clamp(fogFactor, 0.0, 1.0);
}

#endif
