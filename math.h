#pragma once
#include <math.h>

static const float PI = 3.14159265358979323846f;

typedef struct {
	float x, y;
} Vec2;

static float
vec2_len(Vec2 v)
{
        return sqrtf(v.x*v.x + v.y*v.y);
}

static Vec2
vec2_scale(Vec2 v, float scale)
{
        return (Vec2){ v.x * scale, v.y * scale };
}

static float
vec2_get_angle(Vec2 v)
{
        return atan2f(v.y, v.x);
}

static Vec2
vec2_rotate(Vec2 v, float angle)
{
        float cos_a = cosf(angle);
        float sin_a = sinf(angle);
        return (Vec2){ v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a };
}

static Vec2
vec2_norm(Vec2 v)
{
        return vec2_scale(v, 1.0 / vec2_len(v));
}

static Vec2
vec2_sub(Vec2 v0, Vec2 v1)
{
	return (Vec2){ v0.x - v1.x, v0.y - v1.y };
}

static Vec2
vec2_add(Vec2 v0, Vec2 v1)
{
	return (Vec2){ v0.x + v1.x, v0.y + v1.y };
}

