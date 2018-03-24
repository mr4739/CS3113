#pragma once

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int textureID, int idNumber, float size);
	void Draw(ShaderProgram* program, float x, float y);

	float u, v, width, height, size;
	unsigned int textureID;
	int idNumber;
};
