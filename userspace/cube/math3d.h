#pragma once

typedef struct {
    double x, y, z;
} vec3;

vec3 rotate_x(vec3 input, double angle);
vec3 rotate_y(vec3 input, double angle);
vec3 scale_vec(vec3 input, double factor);