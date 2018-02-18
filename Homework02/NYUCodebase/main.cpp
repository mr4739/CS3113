// Homework 02 - Pong
// CONTROLS:
//    SPACE - reset
//    W/S - up/down for left paddle
//    UP/DOWN - up/down for right paddle


#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <iostream>
#include "stb_image.h"
#include "Matrix.h"
#include "ShaderProgram.h"
using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filepath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return retTexture;
}

bool detectCollision(float ballX, float ballY, float paddleX, float paddleY) {
	// ball left larger than paddle right
	if (ballX > paddleX + 1.0f) {
		return false;
	}
	// ball right smaller than paddle left
	else if (ballX < paddleX - 1.0f) {
		return false;
	}
	// ball bottom greater than paddle top
	else if (ballY - 0.5f > paddleY * 3.75 + 1.3333f) {
		return false;
	}
	// ball top less than paddle bottom
	else if (ballY + 0.5f < paddleY * 3.75 - 1.3333f) {
		return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	float lastFrameTicks = 0.0f;
	float angle = 0.0f;
	float playOneY = 2.16f, playTwoY = 0.0f;
	float ballX = 0.0f, ballY = 0.0f;
	float velX = 3.0f, velY = 3.0f;

	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		angle += elapsed;
		ballX += velX * elapsed * 4.0f;
		ballY += velY * elapsed * 2.0f;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					playOneY = 0.0f;
					playTwoY = 0.0f;
					ballX = 0.0f;
					ballY = 0.0f;
					velX = 2.0f;
					velY = 2.0f;
					glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				}
			}
		}

		// moving
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		// player one paddle (left side)
		if (keys[SDL_SCANCODE_W]) {
			playOneY += elapsed * 3.0f;
			if (playOneY > 2.0f + 0.1675f) {
				playOneY = 2.0f + 0.1675f;
			}
		}
		else if (keys[SDL_SCANCODE_S]) {
			playOneY -= elapsed * 3.0f;
			if (playOneY < -2.0f - 0.1675f) {
				playOneY = -2.0f - 0.1675f;
			}
		}
		// player two paddle (right side)
		if (keys[SDL_SCANCODE_UP]) {
			playTwoY += elapsed * 3.0f;
			if (playTwoY > 2.0f + 0.1675f) {
				playTwoY = 2.0f + 0.1675f;
			}
		}
		else if (keys[SDL_SCANCODE_DOWN]) {
			playTwoY -= elapsed * 3.0f;
			if (playTwoY < -2.0f - 0.1675f) {
				playTwoY = -2.0f - 0.1675f;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);

		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		// drawing player one
		program.SetColor(1.0f, 0.8f, 0.0f, 1.0f);
		modelMatrix.Identity();
		modelMatrix.Scale(0.2f, 0.75f, 1.0f);
		modelMatrix.Translate(-15.0f, playOneY, 0.0f);
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// drawing playerTwo
		modelMatrix.Identity();
		modelMatrix.Scale(0.2f, 0.75f, 1.0f);
		modelMatrix.Translate(15.0f, playTwoY, 0.0f);
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// drawing ball
		modelMatrix.Identity();
		modelMatrix.Scale(0.2f, 0.2f, 1.0f);

		// wall collisions - ball animation
		if (ballY >= 9.5f) {
			ballY = 9.5f;
			velY *= -1;
		}
		else if (ballY <= -9.5) {
			ballY = -9.5f;
			velY *= -1;
		}

		// win detecting
		if (ballX >= 17.5f) {
			ballX = 17.5f;
			velY = 0;
			velX = 0;
			glClearColor(0.164f, 0.717f, 0.462f, 1.0f);
		}
		else if (ballX <= -17.5f) {
			ballX = -17.5f;
			velY = 0;
			velX = 0;
			glClearColor(0.164f, 0.717f, 0.462f, 1.0f);
		}

		// paddle collisions
		if (detectCollision(ballX, ballY, -15.0f, playOneY) || 
			detectCollision(ballX, ballY, 15.0f, playTwoY)) {
			velX *= -1;
		}

		//modelMatrix.Translate(15.0f, ballY, 1.0f);
		modelMatrix.Translate(ballX, ballY, 1.0f);
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;
}