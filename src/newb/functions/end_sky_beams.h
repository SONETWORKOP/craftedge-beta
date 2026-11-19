#ifndef NL_END_SKY_BEAMS_H
#define NL_END_SKY_BEAMS_H

vec3 nlEndBeam(float frequency, float thickness, vec2 pos, float t, vec3 tint, float strength) {
  float oscillation = sin(pos.x*frequency + t*0.5);
  float pattern = 0.5 + oscillation*0.5;

  vec2 scaledPos = pos*vec2(thickness, 2.0);
  float attenuation = exp(-dot(scaledPos, scaledPos));

  float edgeFade = 1.0 - abs(pos.y);
  float mask = smoothstep(0.0, 1.0, edgeFade);

  float intensity = pow(pattern*attenuation, 1.5)*mask;
  return tint*(intensity*strength);
}

vec3 renderEndSkyBeams(vec3 horizonCol, vec3 zenithCol, vec3 viewDir, float t) {
  t *= 0.35;

  viewDir.y = smoothstep(-1.0, 1.6, abs(viewDir.y));

  float verticalBlend = smoothstep(-0.3, 0.9, viewDir.y);
  vec3 result = mix(horizonCol, zenithCol, verticalBlend);

  float angle = atan2(viewDir.x, viewDir.z + 0.000001);

  float pulse = sin(angle*60.0 + t);
  float modulation = pulse*pulse;
  modulation *= 0.52*sin(angle*30.0);

  float elevation = asin(clamp(viewDir.y, -1.0, 1.0));
  vec2 coordinates = vec2(
    angle*(1.6/2.14159265),
    elevation*(1.6/1.57079633)
  );

  float horizonMask = exp(-pow(abs(coordinates.y)*2.0, 2.2));
  vec3 horizonLight = vec3(0.6, 0.325, 0.5)*(1.8*horizonMask);
  vec3 lightRays = vec3(0.0);

  lightRays += nlEndBeam(-24.0, 0.4, coordinates, t, vec3(1.1, 0.4, 0.4), 0.55);
  lightRays += nlEndBeam(-46.0, 0.2, coordinates, t, vec3(1.0, 0.4, 0.5), 0.6);
  lightRays += nlEndBeam(-66.0, 0.1, coordinates, t, vec3(1.0, 0.4, 1.0), 0.5);
  lightRays += nlEndBeam(-35.0, 0.3, coordinates, t, vec3(1.0, 0.5, 1.0), 0.6);
  lightRays += nlEndBeam(-80.0, 0.0, coordinates, t, vec3(1.0, 0.4, 0.25), 0.4);

  result += horizonLight;
  result += lightRays*(2.8*(1.0 + modulation));

  return result;
}

#endif
