/*
 * axes.h
 *
 *  Created on: 20/04/2013
 *      Author: clinton
 */

#ifndef AXES_H_
#define AXES_H_

#include <math.h>
#include "types.h"
#include "util.h"
#include "vec3.h"
#include "ray.h"

typedef struct {
	Vec3 u,v,w,o;
}Axes;

static inline Axes axes_identity() {
	return (Axes){(Vec3){1,0,0}, (Vec3){0,1,0}, (Vec3){0,0,1}, (Vec3){0,0,0}};
}

static inline Axes axes_translate(const Axes* axes, const Vec3* m) {
	return (Axes){
		axes->u,
		axes->v,
		axes->w,
		vec3_add(&axes->o, m)
	};
}

static inline Axes axes_rotate_u(const Axes* axes, FPType a) {
	FPType ca = cos_deg(a);
	FPType sa = sin_deg(a);
	Vec3 v_ca = vec3_scale(&axes->v, ca);
	Vec3 v_sa = vec3_scale(&axes->v, sa);
	Vec3 w_ca = vec3_scale(&axes->w, ca);
	Vec3 w_sa = vec3_scale(&axes->w, sa);
	return (Axes){
		axes->u,
		vec3_sub(&v_ca, &w_sa),
		vec3_add(&w_ca, &v_sa),
		axes->o
	};
}

static inline Axes axes_rotate_v(const Axes* axes, FPType a) {
	FPType ca = cos_deg(a);
	FPType sa = sin_deg(a);
	Vec3 w_ca = vec3_scale(&axes->w, ca);
	Vec3 w_sa = vec3_scale(&axes->w, sa);
	Vec3 u_ca = vec3_scale(&axes->u, ca);
	Vec3 u_sa = vec3_scale(&axes->u, sa);
	return (Axes){
		vec3_add(&u_ca, &w_sa),
		axes->v,
		vec3_sub(&w_ca, &u_sa),
		axes->o
	};
}

static inline Axes axes_rotate_w(const Axes* axes, FPType a) {
	FPType ca = cos_deg(a);
	FPType sa = sin_deg(a);
	Vec3 u_ca = vec3_scale(&axes->u, ca);
	Vec3 u_sa = vec3_scale(&axes->u, sa);
	Vec3 v_ca = vec3_scale(&axes->v, ca);
	Vec3 v_sa = vec3_scale(&axes->v, sa);
	return (Axes){
		vec3_sub(&u_ca, &v_sa),
		vec3_add(&v_ca, &u_sa),
		axes->w,
		axes->o
	};
}

static inline Vec3 vector_to_space(const Vec3* vector, const Axes* space) {
	return (Vec3){
		vec3_dot(vector, &space->u),
		vec3_dot(vector, &space->v),
		vec3_dot(vector, &space->w)
	};
}

static inline Vec3 vector_from_space(const Vec3* vector, const Axes* space) {
	return (Vec3){
		space->u.x * vector->x + space->v.x * vector->y + space->w.x * vector->z,
		space->u.y * vector->x + space->v.y * vector->y + space->w.y * vector->z,
		space->u.z * vector->x + space->v.z * vector->y + space->w.z * vector->z
	};
}

static inline Vec3 point_to_space(const Vec3* point, const Axes* space) {
	return (Vec3){
		vec3_dot(point, &space->u) - vec3_dot(&space->o, &space->u),
		vec3_dot(point, &space->v) - vec3_dot(&space->o, &space->v),
		vec3_dot(point, &space->w) - vec3_dot(&space->o, &space->w)
	};
}

static inline Vec3 point_from_space(const Vec3* point, const Axes* space) {
	return (Vec3){
		space->u.x * point->x + space->v.x * point->y + space->w.x * point->z + space->o.x,
		space->u.y * point->x + space->v.y * point->y + space->w.y * point->z + space->o.y,
		space->u.z * point->x + space->v.z * point->y + space->w.z * point->z + space->o.z
	};
}

static inline Ray ray_to_space(const Ray* ray, const Axes* space) {
	return (Ray){
		.origin = point_to_space(&ray->origin, space),
		.direction = vector_to_space(&ray->direction, space)
	};
}

static inline Ray ray_from_space(const Ray* ray, const Axes* space) {
	return (Ray){
		.origin = point_from_space(&ray->origin, space),
		.direction = vector_from_space(&ray->direction, space)
	};
}

static inline Axes axes_to_space(const Axes* axes, const Axes* space) {
	return (Axes){
		.u = vector_to_space(&axes->u, space),
		.v = vector_to_space(&axes->v, space),
		.w = vector_to_space(&axes->w, space),
		.o = point_to_space(&axes->o, space),
	};
}

static inline Axes axes_from_space(const Axes* axes, const Axes* space) {
	return (Axes){
		.u = vector_from_space(&axes->u, space),
		.v = vector_from_space(&axes->v, space),
		.w = vector_from_space(&axes->w, space),
		.o = point_from_space(&axes->o, space),
	};
}

#endif /* AXES_H_ */
