/*
 * scene.c
 *
 *  Created on: 21/04/2013
 *      Author: clinton
 */

#include <assert.h>
#include <memory.h>
#include <malloc.h>
#include "scene.h"

typedef enum {
	SceneType_Empty,
	SceneType_Sphere,
	SceneType_Plane,
	SceneType_HalfSpace,
	SceneType_Box,
	SceneType_FromSpace,
	SceneType_Invert,
	SceneType_Union,
	SceneType_Checker,
	SceneType_Reflective
}SceneType;

typedef CollisionResult (*RayCollisionFn)(const Ray* ray, const Scene* scene);
typedef void (*DataDestructorFn)(void* data);
typedef int (*IsPointInSolidFn)(const Scene* scene, const Vec3* point);
typedef Text* (*RayCollisionFnGLSLCode)(const Scene* scene);
typedef Text* (*IsPointInSolidFnGLSLCode)(const Scene* scene);

struct _Scene {
	SceneType type;
	void* data;
	DataDestructorFn data_destructor_fn;
	RayCollisionFn ray_collision_fn;
	RayCollisionFnGLSLCode ray_collision_fn_glsl_code;
	IsPointInSolidFn is_point_in_solid_fn;
	IsPointInSolidFnGLSLCode is_point_in_solid_fn_glsl_code;
	int ref_count;
};

typedef struct {
	const Scene* scene1;
	const Scene* scene2;
}ScenePair;

static void free_data_destructor(void* data) {
	free(data);
}

static void scene_data_destructor(void* data) {
	scene_unref((Scene*)data);
}

static void scene_pair_destructor(void* data) {
	scene_unref((Scene*)((ScenePair*)data)->scene1);
	scene_unref((Scene*)((ScenePair*)data)->scene2);
	free(data);
}

CollisionResult collision_ray_scene_empty(const Ray* ray, const Scene* scene) {
	return (CollisionResult){
		None,
		(FPType)0,
		(Vec3){0,0,0},
		(Colour){0,0,0},
		(FPType)0
	};
}

Text* collision_ray_scene_empty_glsl_code(const Scene* scene) {
	return text(
		"type = 0;\n"
		"time = 0.0\n"
		"normal = vec3(0.0,0.0,0.0);\n"
		"reflectiveness = 0.0;\n"
	);
}

int scene_empty_is_point_in_solid(const Scene* scene, const Vec3* point) {
	return 0;
}

Text* scene_empty_is_point_in_solid_glsl_code(const Scene* scene) {
	return text("inside = 0;");
}

static Scene* scene_new() {
	Scene* scene = malloc(sizeof(Scene));
	scene->type = SceneType_Empty;
	scene->data = 0;
	scene->data_destructor_fn = free_data_destructor;
	scene->ray_collision_fn = collision_ray_scene_empty;
	scene->ray_collision_fn_glsl_code = collision_ray_scene_empty_glsl_code;
	scene->is_point_in_solid_fn = scene_empty_is_point_in_solid;
	scene->is_point_in_solid_fn_glsl_code = scene_empty_is_point_in_solid_glsl_code;
	scene->ref_count = 1;
	return scene;
}

static void scene_free(Scene* scene) {
	if (scene->data != 0) {
		scene->data_destructor_fn(scene->data);
	}
	free(scene);
}

Scene* scene_empty() {
	Scene* scene = scene_new();
	scene->type = SceneType_Empty;
	scene->data = 0;
	scene->ray_collision_fn = collision_ray_scene_empty;
	scene->ray_collision_fn_glsl_code = collision_ray_scene_empty_glsl_code;
	scene->is_point_in_solid_fn = scene_empty_is_point_in_solid;
	scene->is_point_in_solid_fn_glsl_code = scene_empty_is_point_in_solid_glsl_code;
	return scene;
}

CollisionResult collision_ray_scene_sphere(const Ray* ray, const Scene* scene) {
	return collision_ray_sphere(ray, (const Sphere*)scene->data);
}

Text* collision_ray_scene_sphere_glsl_code(const Scene* scene) {
	const Sphere* sphere = (const Sphere*)scene->data;
	const Text* r[] = {
		text(
			"{\n"
			"    vec3 c = vec3("
		),
		text_from_float(sphere->centre.x), text(", "), text_from_float(sphere->centre.y), text(", "), text_from_float(sphere->centre.z),
		text(
			");\n"
			"    float r = "
		),
		text_from_float(sphere->radius),
		text(
			";\n"
			"    collision_ray_sphere(ro, rd, c, r, type, time, normal, colour, reflectiveness);\n"
			"}"
		)
	};
	return text_append_many(r, sizeof(r) / sizeof(r[0]));
}

int scene_sphere_is_point_in_solid(const Scene* scene, const Vec3* point) {
	const Sphere* sphere = (const Sphere*)scene->data;
	Vec3 tmp = vec3_sub(point, &sphere->centre);
	return vec3_length_squared(&tmp) <= sphere->radius * sphere->radius;
}

Text* scene_sphere_is_point_in_solid_glsl_code(const Scene* scene) {
	const Sphere* sphere = (const Sphere*)scene->data;
	const Text* r[] = {
		text(
			"{\n"
			"	vec3 c = vec3("
		),
		text_from_float(sphere->centre.x), text(", "), text_from_float(sphere->centre.y), text(", "), text_from_float(sphere->centre.z),
		text(
			");\n"
			"	float r = "
		),
		text_from_float(sphere->radius),
		text(
			";\n"
			"	point_in_solid = (pt - c) <= r*r;\n"
			"}"
		)
	};
	return text_append_many(r, sizeof(r) / sizeof(r[0]));
}

Scene* scene_sphere(const Sphere* sphere) {
	Scene* scene = scene_new();
	scene->type = SceneType_Sphere;
	scene->data = malloc(sizeof(Sphere));
	memcpy(scene->data, sphere, sizeof(Sphere));
	scene->data_destructor_fn = free_data_destructor;
	scene->ray_collision_fn = collision_ray_scene_sphere;
	scene->ray_collision_fn_glsl_code = collision_ray_scene_sphere_glsl_code;
	scene->is_point_in_solid_fn = scene_sphere_is_point_in_solid;
	scene->is_point_in_solid_fn_glsl_code = scene_sphere_is_point_in_solid_glsl_code;
	return scene;
}

CollisionResult collision_ray_scene_plane(const Ray* ray, const Scene* scene) {
	return collision_ray_plane(ray, (const Plane*)scene->data);
}

Text* collision_ray_scene_plane_glsl_code(const Scene* scene) {
	const Plane* plane = (const Plane*)scene->data;
	const Text* r[] = {
		text(
			"{\n"
			"    vec3 n = vec3("
		),
		text_from_float(plane->n.x), text(", "), text_from_float(plane->n.y), text(", "), text_from_float(plane->n.z),
		text(
			");\n"
			"    float r = "
		),
		text_from_float(plane->d),
		text(
			";\n"
			"    collision_ray_plane(ro, rd, n, d, type, time, normal, colour, reflectiveness);\n"
			"}"
		)
	};
	return text_append_many(r, sizeof(r) / sizeof(r[0]));
}

Scene* scene_plane(const Plane* plane) {
	Scene* scene = scene_new();
	scene->type = SceneType_Plane;
	scene->data = malloc(sizeof(Plane));
	memcpy(scene->data, plane, sizeof(Plane));
	scene->data_destructor_fn = free_data_destructor;
	scene->ray_collision_fn = collision_ray_scene_plane;
	scene->ray_collision_fn_glsl_code = collision_ray_scene_plane_glsl_code;
	return scene;
}

int scene_half_space_is_point_inside_solid(const Scene* scene, const Vec3* point) {
	const Plane* plane = (const Plane*)scene->data;
	return vec3_dot(point, &plane->n) + plane->d <= 0;
}

Scene* scene_half_space(const Plane* plane) {
	Scene* scene = scene_new();
	scene->type = SceneType_HalfSpace;
	scene->data = malloc(sizeof(Plane));
	memcpy(scene->data, plane, sizeof(Plane));
	scene->data_destructor_fn = free_data_destructor;
	scene->ray_collision_fn = collision_ray_scene_plane;
	scene->is_point_in_solid_fn = scene_half_space_is_point_inside_solid;
	return scene;
}

Scene* scene_box(const Box* box) {
	Plane rightPlane = (Plane){(Vec3){1,0,0},(FPType)-0.5 * box->lenX};
	Plane leftPlane = (Plane){(Vec3){-1,0,0},(FPType)-0.5 * box->lenX};
	Plane backPlane = (Plane){(Vec3){0,1,0},(FPType)-0.5 * box->lenY};
	Plane frontPlane = (Plane){(Vec3){0,-1,0},(FPType)-0.5 * box->lenY};
	Plane topPlane = (Plane){(Vec3){0,0,1},(FPType)-0.5 * box->lenZ};
	Plane bottomPlane = (Plane){(Vec3){0,0,-1},(FPType)-0.5 * box->lenZ};
	return scene_from_space(
		scene_intersect(
			scene_intersect(
				scene_half_space(&rightPlane),
				scene_half_space(&leftPlane)
			),
			scene_intersect(
				scene_intersect(
					scene_half_space(&backPlane),
					scene_half_space(&frontPlane)
				),
				scene_intersect(
					scene_half_space(&topPlane),
					scene_half_space(&bottomPlane)
				)
			)
		),
		&box->axes
	);
}

typedef struct {
	const Scene* scene;
	Axes space;
}FromSpaceData;

void from_space_data_destructor(void* data) {
	scene_unref((Scene*)((FromSpaceData*)data)->scene);
	free(data);
}

CollisionResult collision_ray_scene_from_space(const Ray* ray, const Scene* scene) {
	const Scene* base_scene = ((FromSpaceData*)scene->data)->scene;
	const Axes* space = &((FromSpaceData*)scene->data)->space;
	Ray ray2 = ray_to_space(ray, space);
	CollisionResult cr = collision_ray_scene(&ray2, base_scene);
	cr.normal = vector_from_space(&cr.normal, space);
	return cr;
}

int scene_from_space_is_point_in_solid(const Scene* scene, const Vec3* point) {
	const Scene* base_scene = ((FromSpaceData*)scene->data)->scene;
	const Axes* space = &((FromSpaceData*)scene->data)->space;
	Vec3 point2 = point_to_space(point, space);
	return scene_is_point_in_solid(base_scene, &point2);
}

Scene* scene_from_space(const Scene* scene, const Axes* space) {
	Scene* r = scene_new();
	r->type = SceneType_FromSpace;
	r->data = malloc(sizeof(FromSpaceData));
	*((FromSpaceData*)r->data) = (FromSpaceData){scene, *space};
	r->data_destructor_fn = from_space_data_destructor;
	r->ray_collision_fn = collision_ray_scene_from_space;
	r->is_point_in_solid_fn = scene_from_space_is_point_in_solid;
	return r;
}

CollisionResult collision_ray_scene_invert(const Ray* ray, const Scene* scene) {
	CollisionResult r = collision_ray_scene(ray, (const Scene*)scene->data);
	if (r.type == None) {
		return r;
	} else if (r.type == Enter) {
		r.type = Exit;
		r.normal = vec3_scale(&r.normal, (FPType)-1);
		return r;
	} else if (r.type == Exit) {
		r.type = Enter;
		r.normal = vec3_scale(&r.normal, (FPType)-1);
		return r;
	} else {
		assert(!"unexpected line reached");
		return r;
	}
}

Text* collision_ray_scene_invert_glsl_code(const Scene* scene) {
	const Scene* base_scene = (const Scene*)scene->data;
	return text_append(
		base_scene->ray_collision_fn_glsl_code(base_scene),
		text(
			"if (type == 1) {\n"
			"	type = 2;\n"
			"	normal *= -1;\n"
			"} else if (type == 2) {\n"
			"	type = 1;\n"
			"	normal *= -1;\n"
			"}\n"
		)
	);
}

int scene_invert_is_point_inside_solid(const Scene* scene, const Vec3* point) {
	return !scene_is_point_in_solid((const Scene*)scene->data, point);
}

Text* scene_invert_is_point_in_solid_fn_glsl_code(const Scene* scene) {
	const Scene* base_scene = (const Scene*)scene->data;
	const Text* r[] = {
		text_indent_lines(base_scene->is_point_in_solid_fn_glsl_code(base_scene), text("    ")),
		text("point_in_solid = !point_in_solid;\n")
	};
	return text_append_many(r, sizeof(r) / sizeof(r[0]));
}

Scene* scene_invert(const Scene* scene) {
	Scene* r = scene_new();
	r->type = SceneType_Invert;
	r->data = (void*)scene;
	r->data_destructor_fn = scene_data_destructor;
	r->ray_collision_fn = collision_ray_scene_invert;
	r->ray_collision_fn_glsl_code = collision_ray_scene_invert_glsl_code;
	r->is_point_in_solid_fn = scene_invert_is_point_inside_solid;
	r->is_point_in_solid_fn_glsl_code = scene_invert_is_point_in_solid_fn_glsl_code;
	return r;
}

CollisionResult collision_ray_scene_union(const Ray* ray, const Scene* scene) {
	const Scene* scene1 = ((ScenePair*)scene->data)->scene1;
	const Scene* scene2 = ((ScenePair*)scene->data)->scene2;

	const int max_iterations = 10;

	CollisionResult r1;
	{
		Ray ray2 = *ray;
		Vec3 p;
		for (int i = 0; i < max_iterations; ++i) {
			r1 = collision_ray_scene(&ray2, scene1);
			if (r1.type == None) { break; }
			p = ray_point(&ray2, r1.time);
			if (!scene_is_point_in_solid(scene2, &p)) { break; }
			Vec3 ro = ray_point(&ray2, r1.time+0.1);
			ray2 = ray_init(&ro, &ray2.direction);
		}
		if (r1.type != None) {
			Vec3 v = vec3_sub(&p, &ray->origin);
			r1.time = vec3_dot(&ray->direction, &v);
		}
	}

	CollisionResult r2;
	{
		Ray ray2 = *ray;
		Vec3 p;
		for (int i = 0; i < max_iterations; ++i) {
			r2 = collision_ray_scene(&ray2, scene2);
			if (r2.type == None) { break; }
			p = ray_point(&ray2, r2.time);
			if (!scene_is_point_in_solid(scene1, &p)) { break; }
			Vec3 ro = ray_point(&ray2, r2.time+0.1);
			ray2 = ray_init(&ro, &ray2.direction);
		}
		if (r2.type != None) {
			Vec3 v = vec3_sub(&p, &ray->origin);
			r2.time = vec3_dot(&ray->direction, &v);
		}
	}

	if (r1.type == None) {
		if (r2.type == None) {
			return (CollisionResult){.type=None};
		} else {
			return r2;
		}
	} else {
		if (r2.type == None) {
			return r1;
		} else {
			if (r1.time < r2.time) {
				return r1;
			} else {
				return r2;
			}
		}
	}
}

int scene_union_is_point_in_solid(const Scene* scene, const Vec3* point) {
	const Scene* scene1 = ((ScenePair*)scene->data)->scene1;
	const Scene* scene2 = ((ScenePair*)scene->data)->scene2;
	return scene_is_point_in_solid(scene1, point) || scene_is_point_in_solid(scene2, point);
}

Text* scene_union_is_point_in_solid_glsl_code(const Scene* scene) {
	const ScenePair* pair = (const ScenePair*)scene->data;
	const Text* r[] = {
		text("{\n"),
		pair->scene1->is_point_in_solid_fn_glsl_code(pair->scene1),
		text("bool tmp = point_in_solid;\n"),
		pair->scene2->is_point_in_solid_fn_glsl_code(pair->scene2),
		text("point_in_solid = point_in_soild && tmp;\n")
	};
	return text_append_many(r, sizeof(r) / sizeof(r[0]));
}

Scene* scene_union(const Scene* scene1, const Scene* scene2) {
	Scene* r = scene_new();
	r->type = SceneType_Union;
	r->data = malloc(sizeof(ScenePair));
	*((ScenePair*)r->data) = (ScenePair){scene1, scene2};
	r->data_destructor_fn = scene_pair_destructor;
	r->ray_collision_fn = collision_ray_scene_union;
	r->is_point_in_solid_fn = scene_union_is_point_in_solid;
	r->is_point_in_solid_fn_glsl_code = scene_union_is_point_in_solid_glsl_code;
	return r;
}

Scene* scene_intersect(const Scene* scene1, const Scene* scene2) {
	return scene_invert(scene_union(scene_invert(scene1), scene_invert(scene2)));
}

Scene* scene_subtract(const Scene* scene1, const Scene* scene2) {
	return scene_invert(scene_union(scene_invert(scene1), scene2));
}

typedef struct {
	const Scene* scene;
	FPType size;
	Colour colour1;
	Colour colour2;
}CheckerData;

void checker_data_destructor(void* data) {
	scene_unref((Scene*)((CheckerData*)data)->scene);
	free(data);
}

CollisionResult collision_ray_scene_checker(const Ray* ray, const Scene* scene) {
	CheckerData* data = (CheckerData*)scene->data;
	CollisionResult cr = collision_ray_scene(ray, data->scene);
	if (cr.type != None) {
		Vec3 p = ray_point(ray, cr.time);
		int a = (fmod(p.x+11111,2*data->size) > data->size);
		int b = (fmod(p.y+11111,2*data->size) > data->size);
		int c = (fmod(p.z+11111,2*data->size) > data->size);
		if (a ^ b ^ c) {
			cr.colour = data->colour1;
		} else {
			cr.colour = data->colour2;
		}
	}
	return cr;
}

Scene* scene_checker(const Scene* scene, FPType size, const Colour* colour1, const Colour* colour2) {
	Scene* r = scene_new();
	r->type = SceneType_Checker;
	r->data = malloc(sizeof(CheckerData));
	*((CheckerData*)r->data) = (CheckerData){scene, size, *colour1, *colour2};
	r->data_destructor_fn = checker_data_destructor;
	r->ray_collision_fn = collision_ray_scene_checker;
	return r;
}

typedef struct {
	const Scene* scene;
	FPType reflectiveness;
}ReflectiveData;

void reflective_data_destructor(void* data) {
	scene_unref((Scene*)((ReflectiveData*)data)->scene);
	free(data);
}

CollisionResult collision_ray_scene_reflective(const Ray* ray, const Scene* scene) {
	CollisionResult cr = collision_ray_scene(ray, ((ReflectiveData*)scene->data)->scene);
	if (cr.type != None) {
		cr.reflectiveness = ((ReflectiveData*)scene->data)->reflectiveness;
	}
	return cr;
}

Scene* scene_reflective(const Scene* scene, FPType reflectiveness) {
	Scene* r = scene_new();
	r->type = SceneType_Reflective;
	r->data = malloc(sizeof(ReflectiveData));
	*((ReflectiveData*)r->data) = (ReflectiveData){scene, reflectiveness};
	r->data_destructor_fn = reflective_data_destructor;
	r->ray_collision_fn = collision_ray_scene_reflective;
	return r;
}

void scene_ref(Scene* scene) {
	++scene->ref_count;
}

void scene_unref(Scene* scene) {
	if (--scene->ref_count == 0) {
		scene_free(scene);
	}
}

CollisionResult collision_ray_scene(const Ray* ray, const Scene* scene) {
	return scene->ray_collision_fn(ray, scene);
}

int scene_is_point_in_solid(const Scene* scene, const Vec3* point) {
	return scene->is_point_in_solid_fn(scene, point);
}

Text* collision_ray_scene_glsl_code(const Scene* scene) {
	const Text* r[] = {
		text(
			"void collision_ray_scene(in vec3 ro, in vec3 rd, out int type, out float time, out vec3 normal, out vec3 colour, out float reflectiveness) {\n"
			"    vec3 pt;\n"
			"    bool point_in_solid;\n"
		),
		text_indent_lines(scene->ray_collision_fn_glsl_code(scene), text("    ")),
		text("\n}\n")
	};
	return text_append_many(r, sizeof(r) / sizeof(r[0]));
}
