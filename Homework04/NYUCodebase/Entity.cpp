#include "Entity.h"
#include "ShaderProgram.h"

Entity::Entity() {}
Entity::Entity(float x, float y, float width, float height, float accelX, float accelY, SheetSprite sprite, int health, EntityType type, bool isStatic)
	: x(x), y(y), width(width), height(height), accelX(accelX), accelY(accelY), sprite(sprite), health(health), type(type), isStatic(isStatic) {}

void Entity::Update(float elapsed, float friction, float grav) {
	if (!isStatic) {
		/*velX = lerp(velX, 0.0f, elapsed*friction);
		velY = lerp(velY, 0.0f, elapsed*grav);*/
		/*velX += accelX * elapsed;
		velY += accelY * elapsed;*/

		if (isJumping) {
			collidedBottom = false;
			velY = 2.0f;
			//accelY = -1.0f;
		}
		velX = lerp(velX, 0.0f, elapsed*friction);
		velY = lerp(velY, 0.0f, elapsed*grav);
		velX += accelX * elapsed;
		velY += accelY * elapsed;
		if (collidedTop || (isJumping && y >= maxJumpHeight)) {
			isJumping = false;
			velY *= -1;
		}
		if (collidedBottom) {
			isJumping = false;
			velY = 0.0f;
		}
		x += velX * elapsed;
		y += velY * elapsed;
	}
}

void Entity::jump() {
	if (collidedBottom) {
		isJumping = true;
		maxJumpHeight = y + velY * 1.5f;
	}
}

void Entity::Draw(ShaderProgram& program) {
	if (health > 0) {
		Matrix modelMatrix;
		Matrix viewMatrix;
		viewMatrix.Translate(-x*1.5, -y*1.5, 0.0f);
		modelMatrix.Translate(x, y, 0.0f);
		program.SetModelMatrix(modelMatrix);
		program.SetViewMatrix(viewMatrix);

		sprite.Draw(&program, x, y);
	}
}

float Entity::lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t * v1;
}

bool Entity::collidesWith(const Entity& otherEnt) const {
	/*float otherRight = otherEnt.x + otherEnt.width/2;
	float otherLeft = otherEnt.x - otherEnt.width/2;
	float otherTop = otherEnt.y + otherEnt.height/2;
	float otherBot = otherEnt.y - otherEnt.height/2;

	float right = x + width/2;
	float left = x - width/2;
	float top = y + height/2;
	float bot = y - height/2;

	bool collision = !(bot > otherTop || top < otherBot || left > otherRight || right < otherLeft);
	return collision;*/

	return !(y - height / 2 > otherEnt.y + otherEnt.height / 2 ||
		y + height / 2 < otherEnt.y - otherEnt.height / 2 ||
		x - width / 2 > otherEnt.x + otherEnt.width / 2 ||
		x + width / 2 < otherEnt.x - otherEnt.width / 2);
}