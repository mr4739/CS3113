// Homework 05 - Separated Axis Collision Demo

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <iostream>
#include "Matrix.h"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Vector3.h"
#include <vector>
using namespace std;
#include "SatCollision.h"
//#define FIXED_TIMESTEP 0.0166666f
//#define MAX_TIMESTEPS 6

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
ShaderProgram program;
SDL_Event event;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
bool done = false;

Matrix projectionMatrix, modelMatrix, viewMatrix;
float lastFrameTicks = 0.0f;
float elapsed = 0.0f;
float accumulator = 0.0f;
std::vector<Entity> blocks;
Entity ent1, ent2, ent3, ent4;

void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Homework05 - Separated Axis Collision Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 640, 360);
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	//glClearColor(0.455f, 0.0f, 0.416f, 1.0f);
	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	glUseProgram(program.programID);

	for (int i = 0; i < 15; i++) {
		float randX = (rand() / (float)RAND_MAX * 3.0f * 2) - 3.0f;
		float randY = (rand() / (float)RAND_MAX * 1.5f * 2) - 1.5f;
		float randRot = rand() / (float)RAND_MAX * 2 * M_PI;
		float randWidth = rand() / (float)RAND_MAX * 0.5f + 0.2f;
		float randHeight = rand() / (float)RAND_MAX * 0.5f + 0.2f;
		float randVelX = rand() / (float)RAND_MAX * 4.0f - 1.0f;
		float randVelY = rand() / (float)RAND_MAX * 4.0f - 1.0f;
		Entity block = Entity(randX, randY, randWidth, randHeight, randRot, BLOCK);
		block.velX = randVelX;
		block.velY = randVelY;
		block.rgba.x = rand() / (float)RAND_MAX;
		block.rgba.y = rand() / (float)RAND_MAX;
		block.rgba.z = rand() / (float)RAND_MAX;
		blocks.push_back(block);
	}

}

void ProcessInput() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
}

void Update(float elapsed) {
	for (size_t i = 0; i < blocks.size(); i++) {
		blocks[i].Update(elapsed);
		for (size_t j = 0; j < blocks.size(); j++) {
			blocks[i].collidesWith(blocks[j]);
		}
	}
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	for (size_t i = 0; i < blocks.size(); i++) {
		blocks[i].Draw(program);
	}
	SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[]) {
	Setup();
	std::cout << 3 << std::endl;
	while (!done) {
		ProcessInput();

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		Update(elapsed);
		Render();
	}

	SDL_Quit();
	return 0;
}
