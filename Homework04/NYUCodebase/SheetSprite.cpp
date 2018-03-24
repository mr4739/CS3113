#include "SheetSprite.h"

SheetSprite::SheetSprite() {};
SheetSprite::SheetSprite(unsigned int textureID, int idNumber, float size) : textureID(textureID), idNumber(idNumber), size(size) {
	u = (float)(idNumber % 30) / (float)30;
	v = (float)(idNumber / 30) / (float)30;
	width = 1.0f / 30.0f;
	height = 1.0f / 30.0f;
}

void SheetSprite::Draw(ShaderProgram* program, float x, float y) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	float vertices[] = {
		size * x, size * y,
		size * x, (size * y) - size,
		(size * x) + size, (size * y) - size,
		size * x, size * y,
		(size * x) + size, (size * y) - size,
		(size * x) + size, size * y
	};
	GLfloat texCoords[] = {
		u, v,
		u, v + (height),
		u + width, v + (height),
		u, v,
		u + width, v + (height),
		u + width, v
	};
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}