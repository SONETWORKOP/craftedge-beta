#ifndef YSS_STARS_H
#define YSS_STARS_H

highp float yssRand(highp vec2 x) {
  return fract(cos(x.x + x.y*252.0)*75.0);
}

float yssSmooth(float a, float b, float x) {
  float t = clamp((x-a)/(b-a), 0.0, 1.0);
  return t*t*(3.0-2.0*t);
}

highp vec2 yssStarPos(highp vec2 x) {
  highp vec2 a = floor(x);
  vec2 mid = vec2(yssRand(a+vec2(0.0,1.0)), yssRand(a+vec2(1.0,0.0)));
  mid = mid*0.8 + 0.1;
  mid += a;
  return x - mid;
}

float yssSpecialStar(vec2 pos) {
  float a = 0.0;
  if (pos.x > 0.0) {
    a = max(1.0-length(pos), 0.0);
  } else {
    a = clamp(1.0-abs(pos.y)+pos.x*0.1, 0.0, 1.0);
  }
  return pow(a, 5.0);
}

float yssStarMove(highp vec2 x, highp float t) {
  x.x -= t*3.0;
  x *= 0.1;
  return yssSpecialStar(yssStarPos(x)*30.0);
}

#endif
