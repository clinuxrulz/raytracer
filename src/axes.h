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

#endif /* AXES_H_ */
