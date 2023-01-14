#include "math3d.h"

#include "math.h"

// rx = x;
// ry = 0*x + c*y - s*z
// rz = 0*x + s*y + c*z
vec3 rotate_x(vec3 i, double angle) {
    double s = sin(angle);
    double c = sin(angle + M_PI / 2.0);
    vec3 result = {
        i.x,
        c * i.y - s * i.z,
        s * i.y + c * i.z
    };
    return result;
}

// rx = c*x + s*z
// ry = y
// rz = -s*x + c*z
vec3 rotate_y(vec3 i, double angle) {
    double s = sin(angle);
    double c = sin(angle + M_PI / 2.0);
    vec3 result = {
        c * i.x + s * i.z,
        i.y,
        -s * i.x + c * i.z
    };
    return result;
}

vec3 scale_vec(vec3 i, double f) {
    vec3 result = {
        i.x * f,
        i.y * f,
        i.z * f
    };
    return result;
}

