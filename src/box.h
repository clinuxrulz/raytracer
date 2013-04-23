/*
 * box.h
 *
 *  Created on: 23/04/2013
 *      Author: clinton
 */

#ifndef BOX_H_
#define BOX_H_

#include "types.h"
#include "axes.h"

typedef struct {
	Axes axes;
	FPType lenX, lenY, lenZ;
}Box;

static inline Box box_init(const Axes* axes, FPType lenX, FPType lenY, FPType lenZ) {
	return (Box){*axes, lenX, lenY, lenZ};
}

#endif /* BOX_H_ */
