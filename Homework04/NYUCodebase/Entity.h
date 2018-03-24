#pragma once
#ifndef Entity_h
#define Entity_h
#include "SheetSprite.h"

enum EntityType { PLAYER, ENEMY };

class Entity {
public:
	Entity();
	Entity(float x, float y, float width, float height, float accelX, float accelY, SheetSprite sprite, int health, EntityType type, bool isStatic);
	void Update(float elapsed, float friction, float grav);
	void Draw(ShaderProgram& program);
	float lerp(float v0, float v1, float t);
	bool collidesWith(const Entity& otherEnt) const;
	void jump();

	float x, y, width, height;
	float velX = 0.0f, velY = 0.0f;
	float accelX, accelY;
	SheetSprite sprite;
	EntityType type;
	int health;

	float maxJumpHeight = 0.0f;
	bool isStatic;
	bool isJumping;
	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;
};

#endif