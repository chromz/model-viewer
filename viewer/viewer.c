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
#include <string.h>

#include "viewer.h"

#define MODEL "models/rex.obj"
#define WIDHT 800
#define HEIGHT 600
#define ANGLE_DELTA 0.05F

static const struct aiScene *scene = NULL;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static bool running = true;
static GLuint program;
static mat4 model, view, projection, view_proj;
static GLuint **textures = NULL;
static GLuint *textures_widths = NULL;
static GLuint *textures_heights = NULL;
static bool *has_blend = NULL;
static GLfloat delta_light = 400.F;
static vec3 light = {-26500.F, 44801.F, -11599.F};
static vec3 zoom = {0.F, 0.F, 200.F};
static GLfloat max_zoom = 150.F;
static bool disable_light = 0;



static bool load_model(void)
{
	scene = aiImportFile(MODEL, aiProcess_Triangulate |
			     aiProcess_SortByPType);
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
			case SDLK_q: {
				running = false;
				break;
			}
			case SDLK_r: {
				disable_light = !disable_light;
				break;
			}
			case SDLK_LEFT: {
				mat4 local_rotate;
				glm_mat4_identity(local_rotate);
				vec3 vrotate = {0.F, 1.F, 0.F};
				glm_rotate(local_rotate, -ANGLE_DELTA, vrotate);
				glm_mat4_mul(model, local_rotate, model);
				break;
			}
			case SDLK_RIGHT:{
				mat4 local_rotate;
				glm_mat4_identity(local_rotate);
				vec3 vrotate = {0.F, 1.F, 0.F};
				glm_rotate(local_rotate, ANGLE_DELTA, vrotate);
				glm_mat4_mul(model, local_rotate, model);
				break;
			}

			case SDLK_UP:{
				mat4 local_rotate;
				glm_mat4_identity(local_rotate);
				vec3 vrotate = {1.F, 0.F, 1.F};
				glm_rotate(local_rotate, ANGLE_DELTA, vrotate);
				glm_mat4_mul(model, local_rotate, model);
				break;
			}

			case SDLK_DOWN:{
				mat4 local_rotate;
				glm_mat4_identity(local_rotate);
				vec3 vrotate = {1.F, 0.F, 0.F};
				glm_rotate(local_rotate, -ANGLE_DELTA, vrotate);
				glm_mat4_mul(model, local_rotate, model);
				break;
			}

			case SDLK_j: {
				mat4 local_rotate;
				glm_mat4_identity(local_rotate);
				vec3 vrotate = {0.F, 0.F, 1.F};
				glm_rotate(local_rotate, -ANGLE_DELTA, vrotate);
				glm_mat4_mul(model, local_rotate, model);
				break;
			}

			case SDLK_k: {
				mat4 local_rotate;
				glm_mat4_identity(local_rotate);
				vec3 vrotate = {0.F, 0.F, 1.F};
				glm_rotate(local_rotate, ANGLE_DELTA, vrotate);
				glm_mat4_mul(model, local_rotate, model);
				break;
			}

			case SDLK_w: {
				if (zoom[2] - 5.F > max_zoom) {
					zoom[2] -= 5.F;
				}
				break;
			}

			case SDLK_s: {
				zoom[2] += 5.F;
				break;
			}

			case SDLK_t: {
				light[0] += delta_light;
				break;
			}

			case SDLK_y: {
				light[0] -= delta_light;
				break;
			}

			case SDLK_u: {
				light[1] -= delta_light;
				break;
			}

			case SDLK_i: {
				light[1] += delta_light;
				break;
			}

			case SDLK_o: {
				light[2] -= delta_light;
				break;
			}

			case SDLK_p: {
				light[2] += delta_light;
				break;
			}

			}
		}
	}
}

static char  *concat(const char *s1, const char *s2)
{
	char *result = malloc(strlen(s1) + strlen(s2) + 1);
	if (result == NULL) {
		printf("Couldn't allocate string");
		return NULL;
	}
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

static void render_node(const struct aiNode *node)
{
	for (int i = 0; i < node->mNumMeshes; i++) {
		const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		const struct aiMaterial *mtl =
			scene->mMaterials[mesh->mMaterialIndex];


		// Doubt
		int flat_len = mesh->mNumVertices * 9;
		int flat_size = flat_len * sizeof(GLfloat);
		GLfloat *vertex_data = calloc(flat_len, sizeof(*vertex_data));

		int counter = 0;
		// Flat normals and vertices
		for (int j = 0; j < mesh->mNumVertices; j++) {
			vertex_data[counter++] = mesh->mVertices[j].x;
			vertex_data[counter++] = mesh->mVertices[j].y;
			vertex_data[counter++] = mesh->mVertices[j].z;
			vertex_data[counter++] = mesh->mNormals[j].x;
			vertex_data[counter++] = mesh->mNormals[j].y;
			vertex_data[counter++] = mesh->mNormals[j].z;
			vertex_data[counter++] = mesh->mTextureCoords[0][j].x;
			vertex_data[counter++] = mesh->mTextureCoords[0][j].y;
			vertex_data[counter++] = mesh->mTextureCoords[0][j].z;
		}

		int tex_width = textures_widths[mesh->mMaterialIndex];
		int tex_height = textures_heights[mesh->mMaterialIndex];

		GLint color_depth;
		if (has_blend[mesh->mMaterialIndex]) {
			color_depth = GL_RGBA;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			color_depth = GL_RGB;
			glDisable(GL_BLEND);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, color_depth,
			     tex_width, tex_height, 0,
			     color_depth, GL_UNSIGNED_BYTE,
			     textures[mesh->mMaterialIndex]);

		glGenerateMipmap(GL_TEXTURE_2D);

		// Change
		int face_len =  (mesh->mNumFaces) * mesh->mFaces[0].mNumIndices;
		int face_flat_size = sizeof(GLuint) * face_len;
		GLuint *faces_data = malloc(face_flat_size);

		counter = 0;
		for (int j = 0; j < mesh->mNumFaces; j++) {
			struct aiFace face = mesh->mFaces[j];
			for (int k = 0; k < face.mNumIndices; k++) {
				faces_data[counter++] = face.mIndices[k];
			}
		}

		glBufferData(GL_ARRAY_BUFFER, flat_size,
			     vertex_data, GL_STATIC_DRAW);


		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
				      sizeof(GLfloat) * 9, NULL);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
				      sizeof(GLfloat) * 9,
				      (void *) (sizeof(GLfloat) * 3));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
				      sizeof(GLfloat) * 9,
				      (void *) (sizeof(GLfloat) * 6));
		glEnableVertexAttribArray(2);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_flat_size,
			     faces_data, GL_STATIC_DRAW);

		// Can try to transpose
		glUniformMatrix4fv(
			glGetUniformLocation(program, "model"),
			1,
			GL_FALSE,
			model[0]);

		glUniformMatrix4fv(
			glGetUniformLocation(program, "view"),
			1,
			GL_FALSE,
			view[0]);

		glUniformMatrix4fv(
			glGetUniformLocation(program, "projection"),
			1,
			GL_FALSE,
			projection[0]);


		struct aiColor4D diffuse;
		if (aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE,
				       &diffuse) != AI_SUCCESS) {
			printf("Unable to get diffuse color\n");
			break;
		}

		glUniform4f(
			glGetUniformLocation(program, "color"),
			diffuse.r,
			diffuse.g,
			diffuse.b,
			1);

		glUniform4f(
			glGetUniformLocation(program, "light"),
			light[0],
			light[1],
			light[2],
			1);

		glUniform1i(
			glGetUniformLocation(program, "disable_light"),
			disable_light);

		glDrawElements(GL_TRIANGLES, face_len, GL_UNSIGNED_INT, NULL);

		/* printf("File %s\n", path.data); */
		free(vertex_data);
		free(faces_data);
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
	glm_lookat(zoom,
		   (vec3){0.F, 0.F, 0.F},
		   (vec3) {0.F, 1.F, 0.F}, view);
	render_node(scene->mRootNode);
}

static SDL_Surface* flip_vertical(SDL_Surface* sfc)
{

	SDL_Surface* result = SDL_CreateRGBSurface(sfc->flags, sfc->w, sfc->h,
						   sfc->format->BytesPerPixel *
						   8,
						   sfc->format->Rmask,
						   sfc->format->Gmask,
						   sfc->format->Bmask,
						   sfc->format->Amask);
	int pitch = sfc->pitch;
	int pxlength = pitch*sfc->h;
	GLubyte *pixels = (GLubyte *) (sfc->pixels) + pxlength;
	GLubyte *rpixels = (GLubyte *)(result->pixels) ;
	for(int line = 0; line < sfc->h; ++line) {
		memcpy(rpixels,pixels,pitch);
		pixels -= pitch;
		rpixels += pitch;
	}
	return result;
}

static void load_textures(void)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	textures_widths = malloc(scene->mNumMaterials *
				 sizeof(*textures_widths));
	textures_heights = malloc(scene->mNumMaterials *
				 sizeof(*textures_heights));
	textures = malloc(scene->mNumMaterials * sizeof(*textures));
	has_blend = malloc(scene->mNumMaterials * sizeof(*has_blend));
	for (int i = 0; i < scene->mNumMaterials; i++) {
		const struct aiMaterial *mtl = scene->mMaterials[i];
		int count =
			aiGetMaterialTextureCount(mtl, aiTextureType_DIFFUSE);
		if (count > 0) {
			struct aiString path;
			int success =
				aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE,
						     0, &path, NULL, NULL,
						     NULL, NULL, NULL, NULL);
			if (success != AI_SUCCESS) {
				printf("Error getting texture %s\n",
				       aiGetErrorString());
				break;
			}
			char *image_path = concat("models/", path.data);
			SDL_Surface *img = IMG_Load(image_path);
			if (img == NULL) {
				printf("Could not load image: %s\n",
				       image_path);
				continue;
			}
			free(image_path);
			SDL_Surface *flipped = flip_vertical(img);
			SDL_FreeSurface(img);
			if (flipped->format->BytesPerPixel == 4) {
				has_blend[i] = true;
			} else {
				has_blend[i] = false;
			}

			int tex_width = flipped->w;
			int tex_height = flipped->h;
			int tex_size = tex_width * tex_height;
			textures_widths[i] = tex_width;
			textures_heights[i] = tex_height;
			textures[i] = malloc(tex_size * sizeof(GLuint));
			for (int j = 0; j < tex_size; j++) {
				textures[i][j] = ((GLuint *)
						  flipped->pixels)[j];
			}

			/* textures[i] = img->pixels; */
			/* memcpy(textures[i], img->pixels, tex_size); */
			SDL_FreeSurface(flipped);
			printf("Loaded texture: %s id %d\n", path.data, i);
		}
	}
}

static void init(void)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			    SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
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
	read_shader("viewer/shader.vert", &vertex_shader_src, &vertex_len);
	read_shader("viewer/shader.frag", &fragment_shader_src, &fragment_len);
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
		free(log);
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
		free(log);
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
	free(vertex_shader_src);
	free(fragment_shader_src);

	glm_mat4_identity(model);
	glm_mat4_identity(view);

	vec3 vtranslate = {0.F, 0.F, 0.F};
	vec3 vscale = {.09F, .09F, .09F};
	glm_scale(model, vscale);

	glm_translate(model, vtranslate);


	glm_perspective(glm_rad(45.F), 800.F/600.F, 0.5F, 1000.0, projection);

	glViewport(0, 0, WIDHT, HEIGHT);

	if (!load_model()) {
		return;
	}

	load_textures();
	GLuint vert_buffer;
	glGenVertexArrays(1, &vert_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vert_buffer);

	GLuint element_buffer;
	glGenBuffers(1, &element_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

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
	/* int count = 0; */
	while (running) {
		event_loop(&e);
		display();
		SDL_RenderPresent(renderer);
		SDL_Delay(15);
		/* if (count++ == 5) { */
		/* 	break; */
		/* } */
	}
	free(textures);
	free(textures_heights);
	free(textures_widths);
	aiReleaseImport(scene);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

