// Rodrigo Custodio

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cglm/cglm.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>

#include "viewer.h"

#define MODEL "rex.obj"
#define WIDHT 800
#define HEIGHT 600

static const struct aiScene *scene = NULL;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static bool running = true;
static mat4 the_mat;
static GLuint program;

static float angle = 0.F;



static bool load_model(void)
{
	scene = aiImportFile(MODEL, aiProcessPreset_TargetRealtime_MaxQuality);
	if (!scene) {
		printf("Unable to load model\n");
		return false;
	}
	return true;
}

static bool read_shader(const char *filename, GLchar **shader, GLint *len)
{
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Unable to read shader file\n");
		return false;
	}
	fseek(file, 0, SEEK_END);
	*len = ftell(file);
	fseek(file, 0, SEEK_SET);
	*shader = malloc(*len + 1);
	if (!shader) {
		printf("Unable to malloc shader string\n");
		return false;
	}
	fread(*shader, 1, *len, file);
	(*shader)[*len] = '\0';
	fclose(file);
	return true;
}


static void event_loop(SDL_Event *e)
{
	while (SDL_PollEvent(e)) {
		if (e->type == SDL_QUIT) {
			running = false;
		}
		if (e->type == SDL_KEYDOWN) {
			switch (e->key.keysym.sym) {
			case SDLK_q:
				running = false;
				break;
			case SDLK_r:
				glClearColor(1.0, 0.0, 0.0, 1.0);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
		}
	}
}

static void display(void)
{
	/* glClearColor(.5F, .5F, .5F, 1.F); */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glUseProgram(program);
	glUniformMatrix4fv(
		glGetUniformLocation(program, "theMatrix"),
		1,
		GL_FALSE,
		the_mat[0]
		);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void init(void)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			    SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		printf("Could not load glew %s\n", glewGetErrorString(err));
		return;
	}

	glClearColor(0.5, 1.0, 0.5, 1.0);

	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar *vertex_shader_src = NULL;
	GLchar *fragment_shader_src = NULL;
	GLint vertex_len = 0;
	GLint fragment_len = 0;
	read_shader("shader.vert", &vertex_shader_src, &vertex_len);
	read_shader("shader.frag", &fragment_shader_src, &fragment_len);
	if (vertex_shader_src == NULL || fragment_shader_src == NULL) {
		printf("An error occurred while reading shaders\n");
		return;
	}

	const GLchar *vert_src = vertex_shader_src;
	const GLchar *frag_src = fragment_shader_src;
	// Susp
	glShaderSource(vert_shader, 1, &vert_src, NULL);
	glCompileShader(vert_shader);

	glShaderSource(frag_shader, 1, &frag_src, NULL);
	glCompileShader(frag_shader);

	GLint is_compiled = 0;
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &is_compiled);
	if(is_compiled == GL_FALSE)
	{
		printf("Error compiling vertex shader\n");
		GLint log_size = 0;
		glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &log_size);
		char *log = malloc(sizeof(*log) * log_size);
		glGetShaderInfoLog(vert_shader, log_size, &log_size, log);
		printf("Error: %s\n", log);
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);
		return;
	}
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &is_compiled);
	if(is_compiled == GL_FALSE)
	{
		printf("Error compiling fragment shader\n");
		GLint log_size = 0;
		glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &log_size);
		char *log = malloc(sizeof(*log) * log_size);
		glGetShaderInfoLog(frag_shader, log_size, &log_size, log);
		printf("Error: %s\n", log);
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);
		return;
	}

	program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);
	glLinkProgram(program);
	GLint is_linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
	if (is_linked == GL_FALSE) {
		printf("Could not link compiled shaders\n");
		glDeleteProgram(program);
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);
		return;
	}

	glDetachShader(program, vert_shader);
	glDetachShader(program, frag_shader);

	GLuint vertex_buffer;
	GLfloat vertex_data[18] = {
		-0.5, -0.5, 0.0, 1.0, 0.0, 0.0,
		0.5, -0.5, 0.0, 0.0, 1.0, 0.0,
		0.0, 0.5, 0.0, 0.0, 0.0, 1.0,
	};
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(GLfloat) * 18,
		     vertex_data,
		     GL_STATIC_DRAW);

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		4 * 6,
		(void *) 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		4 * 6,
		(void *) (4 * 3));
	glEnableVertexAttribArray(1);

	mat4 model, view, projection, view_proj;
	mat4 translate, rotate, scale;
	vec3 vtranslate = {0.F, 0.F, 0.F};
	vec3 vrotate = {0.F, 1.F, 0.F};
	vec3 vscale = {1.F, 1.F, 1.F};
	glm_mat4_identity(the_mat);
	glm_mat4_identity(translate);
	glm_mat4_identity(rotate);
	glm_mat4_identity(scale);
	/* glm_mat4_print(temp, stdout); */
	glm_translate(translate, vtranslate);
	glm_rotate(rotate, 0, vrotate);
	glm_scale(scale, vscale);

	glm_mat4_mul(rotate, scale, model);
	glm_mat4_mul(translate, model, model);

	glm_lookat((vec3){0.F, 0.F, 5.F},
		   (vec3){0.F, 0.F, 0.F},
		   (vec3) {0.F, 1.F, 0.F}, view);

	glm_perspective(glm_rad(45.F), 800.F/600.F, 0.1, 1000.0, projection);

	glm_mat4_mul(view, model, the_mat);
	glm_mat4_mul(projection, the_mat, the_mat);

	printf("THE MAT\n");
	glm_mat4_print(the_mat, stdout);
	glViewport(0, 0, WIDHT, HEIGHT);

}


bool viewer_start(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL Init error: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Model viewer",
				  SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  WIDHT, HEIGHT, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		printf("Unable to create window: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	renderer = SDL_CreateRenderer(window,
				      -1,
				      SDL_RENDERER_ACCELERATED |
				      SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		SDL_DestroyWindow(window);
		printf("Could not create renderer %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	init();
	SDL_Event e;
	while (running) {
		event_loop(&e);
		display();
		SDL_RenderPresent(renderer);
		SDL_Delay(15);
	}

	SDL_Quit();
}

