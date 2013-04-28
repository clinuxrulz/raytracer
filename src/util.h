/*
 * util.h
 *
 *  Created on: 20/04/2013
 *      Author: clinton
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <math.h>
#include "types.h"

static const FPType _180_over_PI  = ((FPType)180.0) / ((FPType)M_PI);
static const FPType PI_over_180 = ((FPType)M_PI) / ((FPType)180.0);

static inline FPType deg2rad(FPType deg) {
	return deg * PI_over_180;
}

static inline FPType rad2deg(FPType rad) {
	return rad * _180_over_PI;
}

static inline FPType cos_deg(FPType deg) {
	return cos(deg2rad(deg));
}

static inline FPType sin_deg(FPType deg) {
	return sin(deg2rad(deg));
}

static inline FPType tan_deg(FPType deg) {
	return tan(deg2rad(deg));
}

#endif /* UTIL_H_ */
