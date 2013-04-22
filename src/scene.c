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
	SceneType_Invert,
	SceneType_Union,
	SceneType_Intersect,
	SceneType_Subtract,
	SceneType_Checker,
	SceneType_Reflective
}SceneType;

typedef CollisionResult (*RayCollisionFn)(const Ray* ray, const Scene* scene);

typedef void (*DataDestructorFn)(void* data);

typedef int (*IsPointInSolidFn)(const Scene* scene, const Vec3* point);

struct _Scene {
	SceneType type;
	void* data;
	DataDestructorFn data_destructor_fn;
	RayCollisionFn ray_collision_fn;
	IsPointInSolidFn is_point_in_solid_fn;
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

int scene_empty_is_point_in_solid(const Scene* scene, const Vec3* point) {
	return 0;
}

static Scene* scene_new() {
	Scene* scene = malloc(sizeof(Scene));
	scene->type = SceneType_Empty;
	scene->data = 0;
	scene->data_destructor_fn = free_data_destructor;
	scene->ray_collision_fn = collision_ray_scene_empty;
	scene->is_point_in_solid_fn = scene_empty_is_point_in_solid;
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
	scene->is_point_in_solid_fn = scene_empty_is_point_in_solid;
	return scene;
}

CollisionResult collision_ray_scene_sphere(const Ray* ray, const Scene* scene) {
	return collision_ray_sphere(ray, (const Sphere*)scene->data);
}

int scene_sphere_is_point_in_solid(const Scene* scene, const Vec3* point) {
	const Sphere* sphere = (const Sphere*)scene->data;
	Vec3 tmp = vec3_sub(point, &sphere->centre);
	return vec3_length_squared(&tmp) <= sphere->radius * sphere->radius;
}

Scene* scene_sphere(const Sphere* sphere) {
	Scene* scene = scene_new();
	scene->type = SceneType_Sphere;
	scene->data = malloc(sizeof(Sphere));
	memcpy(scene->data, sphere, sizeof(Sphere));
	scene->data_destructor_fn = free_data_destructor;
	scene->ray_collision_fn = collision_ray_scene_sphere;
	scene->is_point_in_solid_fn = scene_sphere_is_point_in_solid;
	return scene;
}

CollisionResult collision_ray_scene_plane(const Ray* ray, const Scene* scene) {
	return collision_ray_plane(ray, (const Plane*)scene->data);
}

Scene* scene_plane(const Plane* plane) {
	Scene* scene = scene_new();
	scene->type = SceneType_Plane;
	scene->data = malloc(sizeof(Plane));
	memcpy(scene->data, plane, sizeof(Plane));
	scene->data_destructor_fn = free_data_destructor;
	scene->ray_collision_fn = collision_ray_scene_plane;
	return scene;
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

int scene_invert_is_point_inside_solid(const Scene* scene, const Vec3* point) {
	return !scene_is_point_in_solid((const Scene*)scene->data, point);
}

Scene* scene_invert(const Scene* scene) {
	Scene* r = scene_new();
	r->type = SceneType_Invert;
	r->data = (void*)scene;
	r->data_destructor_fn = scene_data_destructor;
	r->ray_collision_fn = collision_ray_scene_invert;
	r->is_point_in_solid_fn = scene_invert_is_point_inside_solid;
	return r;
}

CollisionResult collision_ray_scene_union(const Ray* ray, const Scene* scene) {
	const Scene* scene1 = ((ScenePair*)scene->data)->scene1;
	const Scene* scene2 = ((ScenePair*)scene->data)->scene2;
	Ray ray2 = *ray;
	FPType t_extra = 0.0;
	while (1) {
		CollisionResult r1 = collision_ray_scene(&ray2, scene1);
		CollisionResult r2 = collision_ray_scene(&ray2, scene2);
		Vec3 p1 = ray_point(&ray2, r1.time);
		Vec3 p2 = ray_point(&ray2, r2.time);
		int p1_inside = scene_is_point_in_solid(scene2, &p1);
		int p2_inside = scene_is_point_in_solid(scene1, &p2);
		FPType t1 = r1.time;
		FPType t2 = r2.time;
		r1.time += t_extra;
		r2.time += t_extra;
		if (r1.type == None) {
			if (r2.type == None) {
				return (CollisionResult){.type=None};
			} else {
				if (!p2_inside) {
					return r2;
				}
			}
		} else {
			if (r2.type == None) {
				if (!p1_inside) {
					return r1;
				}
			} else {
				if (r1.time < r2.time) {
					if (!p1_inside) {
						return r1;
					}
				} else {
					if (!p2_inside) {
						return r2;
					}
				}
			}
		}
		FPType t = (r1.type != None && t1 < t2) ? t1 : t2;
		t_extra += t+0.1;
		Vec3 ro = ray_point(&ray2, t+0.1);
		ray2 = ray_init(&ro, &ray2.direction);
	}
	assert(!"unexpected line reached");
	return (CollisionResult){.type=None};
}

Scene* scene_union(const Scene* scene1, const Scene* scene2) {
	Scene* r = scene_new();
	r->type = SceneType_Union;
	r->data = malloc(sizeof(ScenePair));
	*((ScenePair*)r->data) = (ScenePair){scene1, scene2};
	r->data_destructor_fn = scene_pair_destructor;
	r->ray_collision_fn = collision_ray_scene_union;
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
