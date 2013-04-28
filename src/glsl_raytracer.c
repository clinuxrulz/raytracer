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

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;
static const int SCREEN_BPP = 24;

static SDL_Surface* screen = 0;
static int done = 0;
static GLuint programId,vertexShaderId,fragmentShaderId;

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

static void create_shader_program() {
	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShaderId, 1, &sample_vertex_shader, NULL);
	glShaderSource(fragmentShaderId, 1, &sample_fragment_shader, NULL);

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

	return 1;
}

static void render() {
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
		}
	}
}

static void run() {
	while (!done) {
		render();
		process_events();
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

#if RAYTRACE_MODE == RAYTRACE_MODE_GLSL
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
