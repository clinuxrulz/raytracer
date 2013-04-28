/*
 * camera.h
 *
 *  Created on: 20/04/2013
 *      Author: clinton
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "util.h"
#include "axes.h"
#include "vec3.h"
#include "ray.h"
#include "text.h"

typedef struct {
	Axes axes;
	FPType screen_depth;
}Camera;

static inline FPType camera_screen_depth(FPType screen_height, FPType fov) {
	return (FPType)0.5 * screen_height / tan_deg(fov);
}

static inline Camera camera_init() {
	return (Camera) { .axes = axes_identity(), .screen_depth = 400 };
}

static inline Camera camera_set_fov(const Camera* camera, double screen_height, double fov) {
	Camera result = *camera;
	result.screen_depth = camera_screen_depth(screen_height, fov);
	return result;
}

static inline Camera camera_set_axes(const Camera* camera, const Axes* axes) {
	Camera result = *camera;
	result.axes = *axes;
	return result;
}

static inline Ray camera_screen_coord_to_ray(const Camera* camera, int coordX, int coordY, int screenWidth, int screenHeight) {
	Vec3 depth_vector = vec3_scale(&camera->axes.w, -camera->screen_depth);
	Vec3 x_vector = vec3_scale(&camera->axes.u, coordX - screenWidth/2);
	Vec3 y_vector = vec3_scale(&camera->axes.v, screenHeight/2 - coordY);
	Vec3 rd = vec3_add(&depth_vector, &x_vector);
	rd = vec3_add(&rd, &y_vector);
	rd = vec3_scale(&rd, (FPType)1.0 / vec3_length(&rd));
	return ray_init(
		&camera->axes.o,
		&rd
	);
}

static inline Text* camera_screen_coord_to_ray_glsl_code() {
	return text(
		"void screen_coord_to_ray(in vec2 coord, out vec3 ro, out vec3 rd) {\n"
		"	vec3 depth_vector = camera_w * -screen_depth;\n"
		"	vec3 x_vector = camera_u * (coord.x - screen_width/2.0);\n"
		"	vec3 y_vector = camera_v * (screen_height/2.0 - coord.y);\n"
		"	ro = camera_o;\n"
		"	rd = normalize(depth_vector + x_vector + y_vector);\n"
		"}\n"
	);
}

static inline Camera camera_move_forward(const Camera* camera, FPType dist) {
	Vec3 m = vec3_scale(&camera->axes.w, -dist);
	Axes axes = axes_translate(&camera->axes, &m);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_move_back(const Camera* camera, FPType dist) {
	Vec3 m = vec3_scale(&camera->axes.w, dist);
	Axes axes = axes_translate(&camera->axes, &m);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_move_left(const Camera* camera, FPType dist) {
	Vec3 m = vec3_scale(&camera->axes.u, -dist);
	Axes axes = axes_translate(&camera->axes, &m);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_move_right(const Camera* camera, FPType dist) {
	Vec3 m = vec3_scale(&camera->axes.u, dist);
	Axes axes = axes_translate(&camera->axes, &m);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_move_up(const Camera* camera, FPType dist) {
	Vec3 m = vec3_scale(&camera->axes.v, dist);
	Axes axes = axes_translate(&camera->axes, &m);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_move_down(const Camera* camera, FPType dist) {
	Vec3 m = vec3_scale(&camera->axes.v, -dist);
	Axes axes = axes_translate(&camera->axes, &m);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_turn_left(const Camera* camera, double a) {
	Axes axes = axes_rotate_v(&camera->axes, -a);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_turn_right(const Camera* camera, double a) {
	Axes axes = axes_rotate_v(&camera->axes, a);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_turn_up(const Camera* camera, double a) {
	Axes axes = axes_rotate_u(&camera->axes, a);
	return camera_set_axes(
		camera,
		&axes
	);
}

static inline Camera camera_turn_down(const Camera* camera, double a) {
	Axes axes = axes_rotate_u(&camera->axes, -a);
	return camera_set_axes(
		camera,
		&axes
	);
}

#endif /* CAMERA_H_ */
