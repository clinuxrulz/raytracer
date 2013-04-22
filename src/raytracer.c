/*
 ============================================================================
 Name        : raytracer.c
 Author      : Clinton Selke
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */

#include "camera.h"
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;
static const int SCREEN_BPP = 24;

static SDL_Surface* screen = 0;
static int done = 0;
static Scene* scene = 0;
static Camera camera;

static void init_scene() {
	camera = camera_init();
	camera = camera_set_fov(&camera, SCREEN_HEIGHT, 75);
	//camera = camera_turn_up(&camera, 90);
	//camera = camera_move_back(&camera, 400);
	Vec3 centre1 = {-40,10,200};
	FPType radius1 = 60;
	Sphere sphere1 = sphere_init(&centre1, radius1);
	Vec3 centre2 = {40,10,160};
	FPType radius2 = 60;
	Sphere sphere2 = sphere_init(&centre2, radius2);
	Vec3 n = (Vec3){0,1,0};
	Plane plane = plane_init(&n, 50);
	Colour colour1 = (Colour){1,1,1};
	Colour colour2 = (Colour){1,0,0};
	scene = scene_union(
		scene_reflective(
			scene_subtract(
				scene_sphere(&sphere1),
				scene_sphere(&sphere2)
			),
			0.3
		),
		scene_checker(
			scene_plane(&plane),
			50,
			&colour1,
			&colour2
		)
	);
}

static void final_scene() {
	scene_unref(scene);
}

static void init_video() {
	int video_flags = SDL_DOUBLEBUF | SDL_HWACCEL | SDL_HWSURFACE;
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, video_flags);
	SDL_WM_SetCaption("Raytracer", 0);
}

static void draw() {
	unsigned char* pixels = screen->pixels;
	if (SDL_MUSTLOCK(screen)) {
		SDL_LockSurface(screen);
	}
	Vec3 light_dir = (Vec3){1,1,-1};
	light_dir = vec3_normalize(&light_dir);
	int offset1 = 0;
	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		int offset2 = 0;
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			Ray ray = camera_screen_coord_to_ray(&camera, x, y, SCREEN_WIDTH, SCREEN_HEIGHT);
			CollisionResult cr;
			while (1) {
				cr = collision_ray_scene(&ray, scene);
				if (cr.type == Exit) {
					Vec3 ro = ray_point(&ray, cr.time + (FPType)0.1);
					ray = ray_init(&ro, &ray.direction);
					continue;
				}
				break;
			}
			if (cr.type == Enter) {
				Colour clr = cr.colour;
				FPType reflectiveness = cr.reflectiveness;
				Vec3 point = ray_point(&ray, cr.time);
				FPType a = vec3_dot(&light_dir, &cr.normal);
				if (a < 0.3) { a = 0.3; }

				if (reflectiveness > (FPType)0.0) {
					Vec3 rd = vec3_reflect(&ray.direction, &cr.normal);
					Ray ray2 = ray_init(&point, &rd);
					Vec3 ro = ray_point(&ray2, 0.1);
					ray2 = ray_init(&ro, &rd);
					while (1) {
						cr = collision_ray_scene(&ray2, scene);
						if (cr.type == Exit) {
							Vec3 ro = ray_point(&ray2, cr.time + (FPType)0.1);
							ray2 = ray_init(&ro, &ray2.direction);
							continue;
						}
						break;
					}
					clr = colour_mix(&clr, &cr.colour, reflectiveness);
				}

				{
					Vec3 ro = ray_point(&ray, cr.time);
					ray = ray_init(&ro, &light_dir);
					ro = ray_point(&ray, (FPType)0.1);
					ray = ray_init(&ro, &light_dir);
				}
				while (1) {
					cr = collision_ray_scene(&ray, scene);
					if (cr.type == Exit) {
						Vec3 ro = ray_point(&ray, cr.time + (FPType)0.1);
						ray = ray_init(&ro, &ray.direction);
						continue;
					}
					break;
				}
				if (cr.type == Enter) {
					// Shadow
					a *= 0.8;
					if (a < 0.3) { a = 0.3; }
				}

				if (a < (FPType)0) { a = (FPType)0; }
				if (a > (FPType)1) { a = (FPType)1; }
				if (clr.red < (FPType)0) { clr.red = (FPType)0; }
				if (clr.green < (FPType)0) { clr.green = (FPType)0; }
				if (clr.blue < (FPType)0) { clr.blue = (FPType)0; }
				if (clr.red > (FPType)1) { clr.red = (FPType)1; }
				if (clr.green > (FPType)1) { clr.green = (FPType)1; }
				if (clr.blue > (FPType)1) { clr.blue = (FPType)1; }
				pixels[offset1+offset2+2] = (unsigned char)(255*a*clr.red);
				pixels[offset1+offset2+1] = (unsigned char)(255*a*clr.green);
				pixels[offset1+offset2+0] = (unsigned char)(255*a*clr.blue);
			} else {
				pixels[offset1+offset2+2] = 0;
				pixels[offset1+offset2+1] = 0;
				pixels[offset1+offset2+0] = 0;
			}
			offset2 += 3;
		}
		offset1 += screen->pitch;
	}
	if (SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}
	SDL_Flip(screen);
}

static void process_events() {
	SDL_Event event;
	while (SDL_PollEvent(&event) == 1) {
		switch (event.type) {
		case SDL_QUIT:
			done = 1;
			break;
		}
	}
}

static void run() {
	init_scene();
	init_video();
	while (!done) {
		draw();
		process_events();
		SDL_Delay(10);
	}
	final_scene();
}

int main(void) {
	SDL_Init(SDL_INIT_VIDEO);
	run();
	SDL_Quit();
	return EXIT_SUCCESS;
}
