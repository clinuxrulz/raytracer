/*
 * glsl_raytracer.c
 *
 *  Created on: 27/04/2013
 *      Author: Clinton
 */

#include <SDL.h>
#include <GL/glew.h>
//#include <gl/gl.h>
//#include <gl/glu.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "text.h"
#include "collision.h"
#include "scene.h"
#include "camera.h"

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;
static const int SCREEN_BPP = 24;

static SDL_Surface* screen = 0;
static int done = 0;
static GLuint programId,vertexShaderId,fragmentShaderId;
static Camera camera;

static int left_down = 0;
static int right_down = 0;
static int down_down = 0;
static int up_down = 0;
static int w_down = 0;
static int s_down = 0;
static int a_down = 0;
static int d_down = 0;

static const char* sample_vertex_shader =
	"void main() {\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"}\n";

static const char* sample_fragment_shader =
	"void main() {\n"
	"	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

static void print_code(const char* code) {
	int len = strlen(code);
	int line = 1;
	int show_line = 1;
	for (int i = 0; i < len; ++i) {
		if (show_line) {
			printf("%i:", line);
			show_line = 0;
		}
		printf("%c", code[i]);
		if (code[i] == '\n') {
			show_line = 1;
			++line;
		}
	}
}

static void create_shader_program() {
	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	Vec3 sphere_centre = (Vec3){0,0,-200};
	FPType sphere_radius = 50;
	Sphere sphere = sphere_init(&sphere_centre, sphere_radius);
	Scene* scene = scene_sphere(&sphere);
	Text* txt_fragment_shader;
	{
		const Text* tmp[] = {
			text(
				"uniform float screen_width;\n"
				"uniform float screen_height;\n"
				"uniform float screen_depth;\n"
				"uniform vec3 camera_u;\n"
				"uniform vec3 camera_v;\n"
				"uniform vec3 camera_w;\n"
				"uniform vec3 camera_o;\n"
				"uniform float test;\n"
				"\n"
			),
			camera_screen_coord_to_ray_glsl_code(),
			collision_ray_plane_func_glsl_code(),
			collision_ray_sphere_func_glsl_code(),
			collision_ray_scene_glsl_code(scene),
			text(
				"void main() {\n"
				"	vec3 ro;\n"
				"	vec3 rd;\n"
				"	int type;\n"
				"	float time;\n"
				"	vec3 normal;\n"
				"	vec3 colour;\n"
				"	float reflectiveness;\n"
				"	\n"
				"	screen_coord_to_ray(gl_FragCoord.xy, ro, rd);\n"
				"	collision_ray_scene(ro, rd, type, time, normal, colour, reflectiveness);\n"
				"	gl_FragColor = vec4(vec3(colour), 1.0);\n"
				"}\n"
			)
		};
		txt_fragment_shader = text_append_many(tmp, sizeof(tmp) / sizeof(Text*));
	}
	char* fragment_shader = malloc(text_length(txt_fragment_shader)+1);
	const char* fs = fragment_shader;
	text_to_string(txt_fragment_shader, fragment_shader);
	print_code(fs);

	/*
	Text* test_code = collision_ray_scene_glsl_code(scene);
	char* t = malloc(text_length(test_code)+1);
	text_to_string(test_code, t);
	print_code((const char*)t);
	free(t);
	text_unref(test_code);
	*/

	glShaderSource(vertexShaderId, 1, &sample_vertex_shader, NULL);
	glShaderSource(fragmentShaderId, 1, &fs, NULL);

	free(fragment_shader);
	text_unref(txt_fragment_shader);

	glCompileShader(vertexShaderId);
	glCompileShader(fragmentShaderId);

	printShaderInfoLog(vertexShaderId);
	printShaderInfoLog(fragmentShaderId);

	programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);

	printProgramInfoLog(programId);

	glUseProgram(programId);
}

static void free_shader_program() {
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);
	glDeleteProgram(programId);
}

static int init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return 0;
	}
	int video_flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL;
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, video_flags);
	if (screen == 0) {
		return 0;
	}

	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, -1);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	glewInit();

	create_shader_program();

	camera = camera_init();
	camera = camera_set_fov(&camera, SCREEN_HEIGHT, 45);

	return 1;
}

static void render() {
	glUniform1f(glGetUniformLocation(programId, "screen_width"), SCREEN_WIDTH);
	glUniform1f(glGetUniformLocation(programId, "screen_height"), SCREEN_HEIGHT);
	glUniform1f(glGetUniformLocation(programId, "screen_depth"), camera.screen_depth);
	glUniform3f(glGetUniformLocation(programId, "camera_u"), camera.axes.u.x, camera.axes.u.y, camera.axes.u.z);
	glUniform3f(glGetUniformLocation(programId, "camera_v"), camera.axes.v.x, camera.axes.v.y, camera.axes.v.z);
	glUniform3f(glGetUniformLocation(programId, "camera_w"), camera.axes.w.x, camera.axes.w.y, camera.axes.w.z);
	glUniform3f(glGetUniformLocation(programId, "camera_o"), camera.axes.o.x, camera.axes.o.y, camera.axes.o.z);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glBegin(GL_QUADS);
		glVertex3f(0, 0, 0);
		glVertex3f(SCREEN_WIDTH, 0, 0);
		glVertex3f(SCREEN_WIDTH, SCREEN_HEIGHT, 0);
		glVertex3f(0, SCREEN_HEIGHT, 0);
	glEnd();

	SDL_GL_SwapBuffers();
}

static void process_events() {
	SDL_Event event;
	while (SDL_PollEvent(&event) == 1) {
		switch (event.type) {
		case SDL_QUIT:
			done = 1;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_LEFT:
				left_down = 1;
				break;
			case SDLK_RIGHT:
				right_down = 1;
				break;
			case SDLK_UP:
				up_down = 1;
				break;
			case SDLK_DOWN:
				down_down = 1;
				break;
			case SDLK_w:
				w_down = 1;
				break;
			case SDLK_s:
				s_down = 1;
				break;
			case SDLK_a:
				a_down = 1;
				break;
			case SDLK_d:
				d_down = 1;
				break;
			default:
				break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_LEFT:
				left_down = 0;
				break;
			case SDLK_RIGHT:
				right_down = 0;
				break;
			case SDLK_UP:
				up_down = 0;
				break;
			case SDLK_DOWN:
				down_down = 0;
				break;
			case SDLK_w:
				w_down = 0;
				break;
			case SDLK_s:
				s_down = 0;
				break;
			case SDLK_a:
				a_down = 0;
				break;
			case SDLK_d:
				d_down = 0;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

static void move_camera() {
	if (left_down) {
		camera = camera_turn_left(&camera, 3);
	}
	if (right_down) {
		camera = camera_turn_right(&camera, 3);
	}
	if (down_down) {
		camera = camera_turn_down(&camera, 3);
	}
	if (up_down) {
		camera = camera_turn_up(&camera, 3);
	}
	if (w_down) {
		camera = camera_move_forward(&camera, 5);
	}
	if (s_down) {
		camera = camera_move_back(&camera, 5);
	}
	if (a_down) {
		camera = camera_move_left(&camera, 5);
	}
	if (d_down) {
		camera = camera_move_right(&camera, 5);
	}
}

static void run() {
	while (!done) {
		render();
		process_events();
		move_camera();
		SDL_Delay(10);
	}
}

static void final() {
	free_shader_program();
}

static void start() {
	init();
	run();
	final();
	SDL_Quit();
}

#if (RAYTRACE_MODE==RAYTRACE_MODE_GLSL)
#ifdef _WIN32
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	start();
	return 0;
}
#else
int main(int argc, char** argv) {
	start();
	return 0;
}
#endif
#endif
