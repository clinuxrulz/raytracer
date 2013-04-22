/*
 * scene.h
 *
 *  Created on: 21/04/2013
 *      Author: clinton
 */

#ifndef SCENE_H_
#define SCENE_H_

#include "types.h"
#include "collision.h"
#include "sphere.h"
#include "plane.h"

typedef struct _Scene Scene;

Scene* scene_empty();
Scene* scene_sphere(const Sphere* sphere);
Scene* scene_plane(const Plane* plane);
Scene* scene_invert(const Scene* scene);
Scene* scene_union(const Scene* scene1, const Scene* scene2);
Scene* scene_intersect(const Scene* scene1, const Scene* scene2);
Scene* scene_subtract(const Scene* scene1, const Scene* scene2);
Scene* scene_checker(const Scene* scene, FPType size, const Colour* colour1, const Colour* colour2);
Scene* scene_reflective(const Scene* scene, FPType reflectiveness);
void scene_ref(Scene* scene);
void scene_unref(Scene* scene);

CollisionResult collision_ray_scene(const Ray* ray, const Scene* scene);
int scene_is_point_in_solid(const Scene* scene, const Vec3* point);

#endif /* SCENE_H_ */
