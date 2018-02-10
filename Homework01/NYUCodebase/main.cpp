#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "stb_image.h"
#include "Matrix.h"
#include "ShaderProgram.h"
#include <math.h>

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
	ShaderProgram program, untexProgram;
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	untexProgram.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	
	GLuint sanicTexture = LoadTexture(RESOURCE_FOLDER"sanic.png");
	GLuint alienTexture = LoadTexture(RESOURCE_FOLDER"alienYellow.png");
	GLuint grassTexture = LoadTexture(RESOURCE_FOLDER"grass.png");
	GLuint sunTexture = LoadTexture(RESOURCE_FOLDER"alienYellow_round.png");

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	float lastFrameTicks = 0.0f;
	float angle = 0.0f;
	float positionX = 0.0f, positionY = 0.0f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.792f, 0.867f, 0.894f, 1.0f);

	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					positionX = 0.0f;
					positionY = 0.0f;
				}
			}
		}
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		angle += elapsed;

		// moving
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_LEFT]) {
			positionX -= elapsed * 3.0f;
			if (positionX < -3.55f + 0.5f) {
				positionX = -3.55f + 0.5f;
			}
		}
		else if (keys[SDL_SCANCODE_RIGHT]) {
			positionX += elapsed * 3.0f;
			if (positionX > 3.55f - 0.5f) {
				positionX = 3.55f - 0.5f;
			}
		}
		else if (keys[SDL_SCANCODE_UP]) {
			positionY += elapsed * 3.0f;
			if (positionY >  2.0f - 0.5f) {
				positionY = 2.0f - 0.5f;
			}
		}
		else if (keys[SDL_SCANCODE_DOWN]) {
			positionY -= elapsed * 3.0f;
			if (positionY < -2.0f + 0.5f) {
				positionY = -2.0f + 0.5f;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);

		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		// drawing block (textured)
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		modelMatrix.Identity();
		modelMatrix.Translate(0.0f, -1.5f, 0.0f);
		modelMatrix.Scale(8.0f, 1.0f, 1.0f);
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// drawing alien (textured)
		glBindTexture(GL_TEXTURE_2D, alienTexture);
		modelMatrix.Identity();
		modelMatrix.Translate(-2.0, -1.0f, 0.0f);
		modelMatrix.Scale(0.5f, 0.75f, 1.0f);
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// squares behind the sun (untextured)
		glUseProgram(untexProgram.programID);
		untexProgram.SetColor(1.0f, 0.8f, 0.0f, 1.0f);
		modelMatrix.Identity();
		untexProgram.SetModelMatrix(modelMatrix);
		untexProgram.SetProjectionMatrix(projectionMatrix);
		untexProgram.SetViewMatrix(viewMatrix);
		glVertexAttribPointer(untexProgram.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(untexProgram.positionAttribute);

		modelMatrix.Translate(2.55f, 1.0f, 0.0f);
		modelMatrix.Rotate(0.8f);
		untexProgram.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		modelMatrix.Identity();
		modelMatrix.Translate(2.55f, 1.0f, 0.0f);
		untexProgram.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(untexProgram.positionAttribute);
		glDisableVertexAttribArray(untexProgram.texCoordAttribute);

		// drawing sun (textured)
		glUseProgram(program.programID);
		glBindTexture(GL_TEXTURE_2D, sunTexture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		modelMatrix.Identity();
		modelMatrix.Translate(2.55f, 1.0f, 0.0f);
		modelMatrix.Scale(1.1f, 1.1f, 1.1f);
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// drawing sanic
		glBindTexture(GL_TEXTURE_2D, sanicTexture);
		modelMatrix.Identity();
		modelMatrix.Translate(positionX, positionY, 0.0f);
		modelMatrix.Rotate(angle * -20);
		program.SetModelMatrix(modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;
}
