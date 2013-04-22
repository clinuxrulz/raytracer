/*
 * sphere.h
 *
 *  Created on: 21/04/2013
 *      Author: clinton
 */

#ifndef SPHERE_H_
#define SPHERE_H_

#include "types.h"
#include "vec3.h"

typedef struct {
	Vec3 centre;
	FPType radius;
}Sphere;

static __attribute__((unused)) Sphere sphere_init(const Vec3* centre, FPType radius) {
	return (Sphere){.centre = *centre, .radius = radius};
}

#endif /* SPHERE_H_ */
