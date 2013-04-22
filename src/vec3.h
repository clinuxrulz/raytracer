/*
 * vec3.h
 *
 *  Created on: 20/04/2013
 *      Author: clinton
 */

#ifndef VEC3_H_
#define VEC3_H_

#include <math.h>
#include "types.h"

typedef struct {
	FPType x, y, z;
}Vec3;

static const Vec3 vec3_zero = {0, 0, 0};
static const Vec3 vec3_unit_x = {1, 0, 0};
static const Vec3 vec3_unit_y = {0, 1, 0};
static const Vec3 vec3_unit_z = {0, 0, 1};

static inline Vec3 vec3_add(const Vec3* lhs, const Vec3* rhs) {
	return (Vec3){
		lhs->x + rhs->x,
		lhs->y + rhs->y,
		lhs->z + rhs->z
	};
}

static inline Vec3 vec3_sub(const Vec3* lhs, const Vec3* rhs) {
	return (Vec3){
		lhs->x - rhs->x,
		lhs->y - rhs->y,
		lhs->z - rhs->z
	};
}

static inline Vec3 vec3_cross(const Vec3* lhs, const Vec3* rhs) {
	return (Vec3){
		lhs->y * rhs->z - lhs->z * rhs->y,
		lhs->z * rhs->x - lhs->x * rhs->z,
		lhs->x * rhs->y - lhs->y * rhs->x
	};
}

static inline double vec3_dot(const Vec3* lhs, const Vec3* rhs) {
	return lhs->x * rhs->x + lhs->y * rhs->y + lhs->z * rhs->z;
}

static inline FPType vec3_length_squared(const Vec3* v) {
	return vec3_dot(v, v);
}

static inline FPType vec3_length(const Vec3* v) {
	return sqrt(vec3_length_squared(v));
}

static inline Vec3 vec3_scale(const Vec3* lhs, double rhs) {
	return (Vec3){lhs->x * rhs, lhs->y * rhs, lhs->z * rhs};
}

static __attribute__((unused)) inline Vec3 vec3_normalize(const Vec3* v) {
	return vec3_scale(v, (FPType)1.0 / vec3_length(v));
}

static __attribute__((unused)) inline Vec3 vec3_reflect(const Vec3* v, const Vec3* n) {
	// u = v - 2*(v.n/n.n).n
	Vec3 tmp = vec3_scale(n, (FPType)2.0 * vec3_dot(v, n) / vec3_dot(n, n));
	return vec3_sub(v, &tmp);
}

#endif /* VEC3_H_ */
