// Rodrigo Custodio

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
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
static SDL_GLContext ctx;
static bool running = true;

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
		return false;
	}

	ctx = SDL_GL_CreateContext(window);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			    SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);



	SDL_GL_SetSwapInterval(1);
	SDL_Event e;
	while (running) {
		event_loop(&e);
		SDL_GL_SwapWindow(window);
		SDL_Delay(1);
	}


	SDL_GL_DeleteContext(ctx);
	SDL_Quit();
}

