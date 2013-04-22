/*
 * ray.h
 *
 *  Created on: 20/04/2013
 *      Author: clinton
 */

#ifndef RAY_H_
#define RAY_H_

#include "types.h"
#include "vec3.h"

typedef struct {
	Vec3 origin;
	Vec3 direction;
}Ray;

static inline Ray ray_init(const Vec3* origin, const Vec3* direction) {
	return (Ray){*origin, *direction};
}

static inline Ray ray_set_origin(const Ray* ray, const Vec3* origin) {
	return ray_init(origin, &ray->direction);
}

static inline Ray ray_set_direction(const Ray* ray, const Vec3* direction) {
	return ray_init(&ray->origin, direction);
}

static inline Vec3 ray_point(const Ray* ray, FPType time) {
	Vec3 tmp = vec3_scale(&ray->direction, time);
	return vec3_add(&ray->origin, &tmp);
}

#endif /* RAY_H_ */
