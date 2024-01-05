#include "vec.h"

// {{{ vec3
vec3_t vec3_add(vec3_t a, vec3_t b) {
    return (vec3_t) {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z,
    };
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return (vec3_t) {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z,
    };
}

vec3_t vec3_mul(vec3_t a, float f) {
    return (vec3_t) {
        .x = a.x * f,
        .y = a.y * f,
        .z = a.z * f,
    };
}
/// }}}

// {{{ vec2
vec2_t vec2_add(vec2_t a, vec2_t b) {
    return (vec2_t) {
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
}

vec2_t vec2_sub(vec2_t a, vec2_t b) {
    return (vec2_t) {
        .x = a.x - b.x,
        .y = a.y - b.y,
    };
}

vec2_t vec2_mul(vec2_t a, float f) {
    return (vec2_t) {
        .x = a.x * f,
        .y = a.y * f,
    };
}
/// }}}
