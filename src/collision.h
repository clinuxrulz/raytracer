/*
 * collision.h
 *
 *  Created on: 20/04/2013
 *      Author: clinton
 */

#ifndef COLLISION_H_
#define COLLISION_H_

#include <math.h>
#include "types.h"
#include "vec3.h"
#include "ray.h"
#include "plane.h"
#include "sphere.h"
#include "colour.h"
#include "text.h"

typedef enum {
	None,
	Enter,
	Exit
}CollisionType;

typedef struct {
	CollisionType type;
	FPType time;
	Vec3 normal;
	Colour colour;
	FPType reflectiveness;
}CollisionResult;

static __attribute__((unused)) CollisionResult collision_ray_plane(const Ray* ray, const Plane* plane) {
	// (ro + rd.t).n + d = 0
	// ro.n + rd.n.t + d = 0
	// rd.n.t = -(d + ro.n)
	// t = -(d + ro.n) / (rd.n)
	FPType time = -(plane->d + vec3_dot(&ray->origin, &plane->n)) / vec3_dot(&ray->direction, &plane->n);
	if (isnan(time) || isinf(time) || time < (FPType)0) {
		return (CollisionResult){
			.type = None,
			.time = 0,
			.normal = (Vec3){0,0,0},
			.colour = (Colour){0,0,0},
			.reflectiveness = 0
		};
	}
	FPType side = vec3_dot(&ray->origin, &plane->n) + plane->d;
	return (CollisionResult){
		.type = side > 0 ? Enter : Exit,
		.time = time,
		.normal = plane->n,
		.colour = (Colour){1,1,1},
		.reflectiveness = 0
	};
}

static __attribute__((unused)) Text* collision_ray_plane_func_glsl_code() {
	return text(
		"void collision_ray_plane(in vec3 ro, in vec3 rd, in vec3 n, in float d, out int type, out float time, out vec3 normal, out vec3 colour, out float reflectiveness) {\n"
		"	float epsilon = 0.001;\n"
		"	float rd_dot_n = dot(rd, n);\n"
		"	if (-epsilon < rd_dot_n && rd_dot_n < epsilon) {\n"
		"		type = 0;\n"
		"		normal = vec3(0.0, 0.0, 0.0);\n"
		"		colour = vec3(0.0, 0.0, 0.0);\n"
		"		reflectiveness = 0.0;\n"
		"	} else {\n"
		"		float t = -(d + dot(ro, n)) / rd_dot_n;\n"
		"		if (t < 0.0) {\n"
		"			type = 0;\n"
		"			normal = vec3(0.0, 0.0, 0.0);\n"
		"			colour = vec3(0.0, 0.0, 0.0);\n"
		"			reflectiveness = 0.0;\n"
		"		} else {\n"
		"			float side = dot(ro, n) + d;\n"
		"			if (side > 0.0) {\n"
		"				type = 1;\n"
		"			} else {\n"
		"				type = 2;\n"
		"			}\n"
		"			time = t;\n"
		"			normal = n;\n"
		"			colour = vec3(1.0,1.0,1.0);\n"
		"			reflectiveness = 0.0;\n"
		"		}\n"
		"	}\n"
		"}\n"
	);
}

static __attribute__((unused)) CollisionResult collision_ray_sphere(const Ray* ray, const Sphere* sphere) {
	// (ro + rd.t - c).(ro + rd.t - c) = r.r
	// ((ro - c) + rd.t).((ro - c) + rd.t) = r.r
	// (ro - c).(ro - c) + 2.(ro - c).rd.t + rd.rd.t.t = r.r
	// rd.rd.t.t + 2.(ro - c).rd.t = r.r - (ro - c).(ro - c)
	// t.t + ((2.(ro - c).rd)/(rd.rd)).t = (r.r - (ro - c).(ro - c)) / (rd.rd)
	// (t + ((ro - c).rd)/(rd.rd))^2 - (((ro - c).rd)^2) / ((rd.rd)^2) = (r.r - (ro - c).(ro - c)) / (rd.rd)
	// (t + ((ro - c).rd)/(rd.rd))^2 - (((ro - c).rd)^2) / ((rd.rd)^2) = ((r.r - (ro - c).(ro - c)).(rd.rd)) / ((rd.rd)^2)
	// (t + ((ro - c).rd)/(rd.rd))^2 = ((r.r - (ro - c).(ro - c)).(rd.rd) + ((ro - c).rd)^2) / ((rd.rd)^2)
	// t + ((ro - c).rd)/(rd.rd) = sqrt(((r.r - (ro - c).(ro - c)).(rd.rd) + ((ro - c).rd)^2) / ((rd.rd)^2))
	// t + ((ro - c).rd)/(rd.rd) = sqrt(((r.r - (ro - c).(ro - c)).(rd.rd) + ((ro - c).rd)^2)) / (rd.rd)
	// t = (-((ro - c).rd)+-sqrt(((r.r - (ro - c).(ro - c)).(rd.rd) + ((ro - c).rd)^2))) / (rd.rd)
	const Vec3* ro = &ray->origin;
	const Vec3* rd = &ray->direction;
	const Vec3* c = &sphere->centre;
	FPType r = sphere->radius;
	Vec3 ro_sub_c = vec3_sub(ro, c);
	FPType ro_sub_c_dot_rd = vec3_dot(&ro_sub_c, rd);
	FPType rd_dot_rd = vec3_dot(rd, rd);
	FPType x = -vec3_dot(&ro_sub_c, rd) / rd_dot_rd;
	if (isnan(x) || isinf(x)) {
		return (CollisionResult){
			.type = None,
			.time = 0,
			.normal = (Vec3){0,0,0},
			.colour = (Colour){0,0,0},
			.reflectiveness = 0
		};
	}
	FPType y = (r*r - vec3_dot(&ro_sub_c, &ro_sub_c)) * rd_dot_rd + ro_sub_c_dot_rd*ro_sub_c_dot_rd;
	if (y < 0) {
		return (CollisionResult){
			.type = None,
			.time = 0,
			.normal = (Vec3){0,0,0},
			.colour = (Colour){0,0,0},
			.reflectiveness = 0
		};
	}
	y = sqrt(y) / rd_dot_rd;
	FPType t1 = x - y;
	FPType t2 = x + y;
	if (t1 > 0) {
		Vec3 n = ray_point(ray, t1);
		n = vec3_sub(&n, &sphere->centre);
		n = vec3_normalize(&n);
		return (CollisionResult){
			.type = Enter,
			.time = t1,
			.normal = n,
			.colour = (Colour){1,1,1},
			.reflectiveness = 0
		};
	} if (t2 > 0) {
		Vec3 n = ray_point(ray, t2);
		n = vec3_sub(&n, &sphere->centre);
		n = vec3_normalize(&n);
		return (CollisionResult){
			.type = Exit,
			.time = t2,
			.normal = n,
			.colour = (Colour){1,1,1},
			.reflectiveness = 0
		};
	} else {
		return (CollisionResult){
			.type = None,
			.time = 0,
			.normal = (Vec3){0,0,0},
			.colour = (Colour){0,0,0},
			.reflectiveness = 0
		};
	}
}

static __attribute__((unused)) Text* collision_ray_sphere_func_glsl_code() {
	return text(
		"void collision_ray_sphere(in vec3 ro, in vec3 rd, in vec3 c, in float r, out int type, out float time, out vec3 normal, out vec3 colour, out float reflectiveness) {\n"
		"	float epsilon = 0.001;\n"
		"	vec3 ro_sub_c = ro - c;\n"
		"	float ro_sub_c_dot_rd = dot(ro_sub_c,rd);\n"
		"	float rd_dot_rd = dot(rd,rd);\n"
		"	if (rd_dot_rd < epsilon) {\n"
		"		type = 0;\n"
		"		normal = vec3(0.0, 0.0, 0.0);\n"
		"		colour = vec3(0.0, 0.0, 0.0);\n"
		"		reflectiveness = 0.0;\n"
		"	} else {\n"
		"		float x = -dot(ro_sub_c, rd) / rd_dot_rd;\n"
		"		float y = (r*r - dot(ro_sub_c, ro_sub_c)) * rd_dot_rd + ro_sub_c_dot_rd*ro_sub_c_dot_rd;\n"
		"		if (y < 0.0) {\n"
		"			type = 0;\n"
		"			normal = vec3(0.0, 0.0, 0.0);\n"
		"			colour = vec3(0.0, 0.0, 0.0);\n"
		"			reflectiveness = 0.0;\n"
		"		} else {\n"
		"			y = sqrt(y) / rd_dot_rd;\n"
		"			float t1 = x - y;\n"
		"			float t2 = x + y;\n"
		"			if (t1 > 0.0) {\n"
		"				type = 1;\n"
		"				time = t1;\n"
		"				normal = normalize(ro + rd*t1 - c);\n"
		"				colour = vec3(1.0, 1.0, 1.0);\n"
		"				reflectiveness = 0.0;\n"
		"			} else if (t2 > 0.0) {\n"
		"				type = 2;\n"
		"				time = t2;\n"
		"				normal = normalize(ro + rd*t2 - c);\n"
		"				colour = vec3(1.0, 1.0, 1.0);\n"
		"				reflectiveness = 0.0;\n"
		"			} else {\n"
		"				type = 0;\n"
		"				normal = vec3(0.0, 0.0, 0.0);\n"
		"				colour = vec3(0.0, 0.0, 0.0);\n"
		"				reflectiveness = 0.0;\n"
		"			}\n"
		"		}\n"
		"	}\n"
		"}\n"
	);
}

#endif /* COLLISION_H_ */
