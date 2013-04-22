/*
 * plane.h
 *
 *  Created on: 20/04/2013
 *      Author: clinton
 */

#ifndef PLANE_H_
#define PLANE_H_

#include "types.h"
#include "vec3.h"

typedef struct {
	Vec3 n;
	FPType d;
}Plane;

static inline Plane plane_init(const Vec3* n, FPType d) {
	return (Plane){*n, d};
}

#endif /* PLANE_H_ */
