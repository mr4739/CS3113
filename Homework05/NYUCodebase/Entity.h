#pragma once
#ifndef Entity_h
#define Entity_h
#include "ShaderProgram.h"
#include "Matrix.h"
#include "Vector3.h"
#include <vector>

class Matrix;
class ShaderProgram;
class Vector3;

enum EntityType { PLAYER, BLOCK };

class Entity {
public:
	Entity();
	Entity(float x, float y, float width, float height, float rotation, EntityType type);
	void Update(float elapsed);
	void Draw(ShaderProgram& program);
	//float lerp(float v0, float v1, float t);
	bool collidesWith(Entity& otherEnt);

	float x, y, width, height;
	float velX = 0.0f, velY = 0.0f;
	float rotation;
	EntityType type;
	float size = 1.0f;

	Matrix modelMatrix;

	Vector3 position;
	Vector3 scale;
	Vector3 rgba;

	std::vector<float> vertices;
	std::vector<Vector3> points;

	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;
};

#endif