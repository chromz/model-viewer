// Rodrigo Custodio

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cglm/cglm.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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
static GLuint program;
static mat4 model, view, projection, view_proj;

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

static void render_node(const struct aiNode *node)
{
	struct aiMatrix4x4 model = node->mTransformation;
	for (int i = 0; i < node->mNumMeshes; i++) {
		const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		const struct aiMaterial *mtl =
			scene->mMaterials[mesh->mMaterialIndex];
		struct aiString path;

		int count =
			aiGetMaterialTextureCount(mtl, aiTextureType_DIFFUSE);
		if (count < 1) {
			break;
		}
		int success = aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE,
						   0, &path, NULL, NULL, NULL,
						   NULL, NULL, NULL);
		if (success != AI_SUCCESS) {
			printf("Error getting texture %s\n",
			       aiGetErrorString());
			break;
		}
		// Remove assimp first 2 chars
		char *image_name = path.data + 2;
		SDL_Surface *img = IMG_Load(image_name);

		int tex_width = img->w;
		int tex_height = img->h;
		GLubyte *texture_data =  img->pixels;
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			     tex_width, tex_height, 0,
			     GL_RGB, GL_UNSIGNED_BYTE, texture_data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Doubt
		int flat_size = mesh->mNumVertices * 9 * sizeof(GLfloat);
		GLfloat *vertex_data = malloc(flat_size);
		int counter = 0;
		int offset = mesh->mNumVertices;
		int offset_tex = offset * 2;
		// Flat normals and vertices
		for (int j = 0; j < mesh->mNumVertices; j++) {
			vertex_data[counter] = mesh->mVertices[j].x;
			vertex_data[offset + counter] = mesh->mNormals[j].x;
			vertex_data[offset_tex + counter++] =
				mesh->mTextureCoords[0][j].x;
			vertex_data[counter] = mesh->mVertices[j].y;
			vertex_data[offset + counter] = mesh->mNormals[j].y;
			vertex_data[offset_tex + counter++] =
				mesh->mTextureCoords[0][j].y;
			vertex_data[counter] = mesh->mVertices[j].z;
			vertex_data[offset + counter++] = mesh->mNormals[j].z;
			vertex_data[offset_tex + counter++] =
				mesh->mTextureCoords[0][j].z;
		}

		GLuint vert_buffer;
		glGenVertexArrays(1, &vert_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vert_buffer);
		glBufferData(GL_ARRAY_BUFFER, flat_size,
			     vertex_data, GL_STATIC_DRAW);

		/* printf("File %s\n", path.data); */
		free(vertex_data);
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		render_node(node->mChildren[i]);
	}
}

static void display(void)
{
	/* glClearColor(.5F, .5F, .5F, 1.F); */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	render_node(scene->mRootNode);
	/* glDrawArrays(GL_TRIANGLES, 0, 3); */
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

	glClearColor(0.5F, 0.5F, 0.5F, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

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

	glUseProgram(program);

	glm_mat4_identity(model);
	glm_mat4_identity(view);

	/* mat4 translate, rotate, scale; */
	/* vec3 vtranslate = {0.F, 0.F, 0.F}; */
	/* vec3 vrotate = {0.F, 1.F, 0.F}; */
	/* vec3 vscale = {1.F, 1.F, 1.F}; */
	/* glm_mat4_identity(translate); */
	/* glm_mat4_identity(rotate); */
	/* glm_mat4_identity(scale); */
	/* /\* glm_mat4_print(temp, stdout); *\/ */
	/* glm_translate(translate, vtranslate); */
	/* glm_rotate(rotate, 0, vrotate); */
	/* glm_scale(scale, vscale); */

	/* glm_mat4_mul(rotate, scale, model); */
	/* glm_mat4_mul(translate, model, model); */

	/* glm_lookat((vec3){0.F, 0.F, 5.F}, */
	/* 	   (vec3){0.F, 0.F, 0.F}, */
	/* 	   (vec3) {0.F, 1.F, 0.F}, view); */

	glm_perspective(glm_rad(45.F), 800.F/600.F, 0.1, 1000.0, projection);

	glViewport(0, 0, WIDHT, HEIGHT);

	if (!load_model()) {
		return;
	}

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

