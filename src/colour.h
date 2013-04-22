/*
 * colour.h
 *
 *  Created on: 21/04/2013
 *      Author: clinton
 */

#ifndef COLOUR_H_
#define COLOUR_H_

#include "types.h"

typedef struct {
	FPType red,green,blue;
}Colour;

static inline Colour colour_init(FPType red, FPType green, FPType blue) {
	return (Colour){red,green,blue};
}

static inline Colour colour_mix(const Colour* colour1, const Colour* colour2, FPType amount) {
	FPType one_minus_amount = (FPType)1 - amount;
	return (Colour){
		colour1->red * one_minus_amount + colour2->red * amount,
		colour1->green * one_minus_amount + colour2->green * amount,
		colour1->blue * one_minus_amount + colour2->blue * amount
	};
}

#endif /* COLOUR_H_ */
