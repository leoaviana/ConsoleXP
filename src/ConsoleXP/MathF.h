#pragma once
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "pch.h"
#include <cmath>

namespace MathF {

    static float Lerp(float a, float b, float f) {
        return a * (1.0f - f) + (b * f);
    }

    static float Clamp(float value, float min, float max) {
        if (value < min)
            value = min;
        else if (value > max)
            value = max;
        return value;
    }

    static float LinearRemap(float value, float oldmin, float oldmax, float newmin, float newmax) {
        return (((value - oldmin) * (newmax - newmin)) / (oldmax - oldmin)) + newmin;
    }

    static float LerpAngle(float a, float b, float t) {
        float delta = Clamp((b - a) - std::floor((b - a) / 360.0f) * 360.0f, 0.0f, 360.0f);
        if (delta > 180.0f)
            delta -= 360.0f;
        return a + delta * Clamp(t, 0.0f, 1.0f);
    }

    static float LerpRadians(float a, float b, float t) {
        float flta = (a * (180.0f / M_PI));
        float fltb = (b * (180.0f / M_PI));

        return LerpAngle(flta, fltb, t) * (M_PI / 180.0f);
    }

    static float SmoothDamp(float current, float target, float deltaTime, float dampRate) {
        float lerpFactor = Clamp(dampRate * deltaTime, 0.0f, 1.0f);
        return Lerp(current, target, lerpFactor);
    }

    static float ShortestAngleBetween(float from, float to)
    {
        float difference = fmodf(to - from, 2.0f * M_PI);

        if (difference < -M_PI)
            difference += 2.0f * M_PI;
        else if (difference > M_PI)
            difference -= 2.0f * M_PI;

        return difference;
    }

    static float NormalizeRadian(float value) {
        float radian = value;

        while (radian < 0) {
            radian = (2.0f * M_PI) + radian;
        }

        while (radian >= (2.0f * M_PI)) {
            radian -= (2.0f * M_PI);
        }

        return radian;
    }
}