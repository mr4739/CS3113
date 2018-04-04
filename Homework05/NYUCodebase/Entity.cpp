#include "Entity.h"
#include "ShaderProgram.h"
#include "Matrix.h"
#include "SatCollision.h"

Entity::Entity() {}
Entity::Entity(float x, float y, float width, float height, float rotation, EntityType type)
	: x(x), y(y), position(x, y, 0.0f), scale(1.0f, 1.0f, 1.0f), width(width), rotation(rotation), height(height), type(type) {
	points.emplace_back(-0.5f * width, 0.5f * height, 0.0f);
	points.emplace_back(-0.5f * width, -0.5f * height, 0.0f);
	points.emplace_back(0.5f * width, -0.5f * height, 0.0f);
	points.emplace_back(0.5f * width, 0.5f * height, 0.0f);
	vertices.insert(vertices.end(), {
		-0.5f * width, -0.5f * height,
		0.5f * width, -0.5f * height,
		0.5f * width, 0.5f * height,
		-0.5f * width, -0.5f * height,
		0.5f * width, 0.5f * height,
		-0.5f * width, 0.5f * height });
}

void Entity::Update(float elapsed) {
	if (position.x < -3.55f + width/2) {
		position.x = -3.55f + width/2;
		velX *= -1;
	}
	if (position.x > 3.55f - width/2) {
		position.x = 3.55f - width/2;
		velX *= -1;
	}
	if (position.y >  2.0f - height/2) {
		position.y = 2.0f - height / 2;
		velY *= -1;
	}
	if (position.y < -2.0f + height / 2) {
		position.y = -2.0f + height / 2;
		velY *= -1;
	}
	position.x += velX * elapsed;
	position.y += velY * elapsed;
}

void Entity::Draw(ShaderProgram& program) {
	modelMatrix.Identity();
	modelMatrix.Translate(position.x, position.y, position.z);
	modelMatrix.Rotate(rotation);
	modelMatrix.Scale(scale.x, scale.y, scale.z);
	program.SetModelMatrix(modelMatrix);
	/*float vertices[] = { 
		-0.5 * width, -0.5 * height, 
		0.5 * width, -0.5 * height, 
		0.5 * width, 0.5 * height, 
		-0.5 * width, -0.5 * height, 
		0.5 * width, 0.5 * height, 
		-0.5 * width, 0.5 * height };*/
	program.SetColor(rgba.x, rgba.y, rgba.z, 1.0f);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

bool Entity::collidesWith(Entity& otherEnt) {
	std::pair<float, float> penetration;
	std::vector<std::pair<float, float>> e1Points;
	std::vector<std::pair<float, float>> e2points;
	for (size_t i = 0; i < points.size(); i++) {
		e1Points.push_back(modelMatrix * points[i]);
	}
	for (size_t i = 0; i < otherEnt.points.size(); i++) {
		e2points.push_back(otherEnt.modelMatrix * otherEnt.points[i]);
	}
	bool collided = CheckSATCollision(e1Points, e2points, penetration);

	if (collided) {
		position.x += penetration.first * 0.5f;
		position.y += penetration.second * 0.5f;
		otherEnt.position.x -= penetration.first * 0.5f;
		otherEnt.position.y -= penetration.second * 0.5f;
		if (fabs(penetration.first) > fabs(penetration.second)) {
			velX *= -1;
			otherEnt.velX *= -1;
		}
		else {
			velY *= -1;
			otherEnt.velY *= -1;
		}
	}

	return collided;
	/*return !(y - height / 2 > otherEnt.y + otherEnt.height / 2 ||
		y + height / 2 < otherEnt.y - otherEnt.height / 2 ||
		x - width / 2 > otherEnt.x + otherEnt.width / 2 ||
		x + width / 2 < otherEnt.x - otherEnt.width / 2);*/
}