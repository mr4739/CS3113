/* Homework 03 - Space Invaders*/

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
#include <vector>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
ShaderProgram program;
SDL_Event event;
bool done = false;
float lastFrameTicks = 0.0f;
float elapsed = 0.0f;
float angle = 0.0f;
const float aspect = (float)640 / (float)360;
const float projHeight = 2.0f, projWidth = 2.0f * aspect;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

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
class SheetSprite {
public:
	SheetSprite() {};
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) :
		textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
	void Draw(ShaderProgram* program) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u,v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};
		float aspectRatio = width / height;
		float vertices[] = {
			-0.5f * size * aspectRatio, -0.5f * size,
			0.5f * size * aspectRatio, 0.5f * size,
			-0.5f * size * aspectRatio, 0.5f * size,
			0.5f * size * aspectRatio, 0.5f * size,
			-0.5f * size * aspectRatio, -0.5f * size,
			0.5f * size * aspectRatio, -0.5f * size,
		};
		// draw arrays
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}
	float u, v, width, height, size;
	unsigned int textureID;
};
class Entity {
public:
	Entity() {};
	Entity(float x, float y, float width, float height, SheetSprite sprite, int health) :
		x(x), y(y), width(width), height(height), sprite(sprite), health(health) {}
	void Draw(ShaderProgram& program) {
		if (health > 0) {
			Matrix modelMatrix;
			Matrix projectionMatrix;
			Matrix viewMatrix;

			projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
			modelMatrix.Translate(x, y, 0.0f);
			modelMatrix.Rotate(rotation);

			program.SetModelMatrix(modelMatrix);
			program.SetProjectionMatrix(projectionMatrix);
			program.SetViewMatrix(viewMatrix);

			sprite.Draw(&program);
		}
	}
	void shootBullet(int type);
	bool collidesWith(const Entity& otherEnt) const {
		return !(y - height / 2 > otherEnt.y + otherEnt.height / 2 ||
			y + height / 2 < otherEnt.y - otherEnt.height / 2 ||
			x - width / 2 > otherEnt.x + otherEnt.width / 2 ||
			x + width / 2 < otherEnt.x - otherEnt.width / 2);
	}
	float x, y, width, height;
	float rotation = 0.0f, velX = 0.0f, velY = 0.0f;
	SheetSprite sprite;
	int health;
};

enum GameMode { STATE_TITLE_SCREEN, STATE_GAME_LEVEL, WIN, LOSE };
GameMode mode = STATE_TITLE_SCREEN;
float bulletReload = 0.5f;
float enemyBulletReload = 0.0f;
int score = 0;

class GameState {
public:
	// initialize sprites and player
	void loadEntities() {
		fontTextureID = LoadTexture(RESOURCE_FOLDER"font1.png");
		sheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
		playerSprite = SheetSprite(sheetTexture, 224.0f / 1024.0f, 832.0f / 1024.0f,
			99.0f / 1024.0f, 75.0f / 1024.0f, 0.5);
		player = Entity(0.0f, projHeight*-0.85f, playerSprite.width*playerSprite.size / playerSprite.height,
			playerSprite.size, playerSprite, 3);
		bulletSprite = SheetSprite(sheetTexture, 856.0f / 1024.0f, 421.0f / 1024.0f,
			9.0f / 1024.0f, 54.0f / 1024.0f, 0.3);
		enemySprite = SheetSprite(sheetTexture, 425.0f / 1024.0f, 468.0f / 1024.0f,
			93.0f / 1024.0f, 84.0f / 1024.0f, 0.3);
	}
	// adds new enemies to enemies vector
	void spawnEnemies() {
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 4; j++) {
				Entity enemy = Entity(projWidth*-0.7f + i * enemySprite.width * 5,
					projHeight*0.4f + j * enemySprite.height * 4,
					enemySprite.width*enemySprite.size / enemySprite.height, enemySprite.size, enemySprite, 1);
				enemy.velX = 1.5f;
				enemies.push_back(enemy);
			}
		}
	}
	// reset for new game
	void reset() {
		enemies.clear();
		playerBullets.clear();
		enemyBullets.clear();
		player.health = 3;
		score = 0;
		spawnEnemies();
	}
	void Update() {
		// player mvmt
		if (keys[SDL_SCANCODE_LEFT]) {
			player.x -= elapsed * 3.0f;
			if (player.x <= -projWidth + player.width / 2) {
				player.x = -projWidth + player.width / 2;
			}
		}
		else if (keys[SDL_SCANCODE_RIGHT]) {
			player.x += elapsed * 3.0f;
			if (player.x >= projWidth - player.width / 2) {
				player.x = projWidth - player.width / 2;
			}
		}
		// enemy mvmt
		if (enemies[0].x - enemies[0].width / 2 < -3.55f || enemies.back().x + enemies.back().width / 2 > 3.55f) {
			for (int i = 0; i < enemies.size(); i++) {
				//enemies[i].y -= 0.2f;
				enemies[i].velX *= -1;
			}
		}
		for (int i = 0; i < enemies.size(); i++) {
			enemies[i].x += elapsed * enemies[i].velX;
		}
		if (keys[SDL_SCANCODE_UP] && bulletReload >= 0.5f && player.health > 0) {
			player.shootBullet(0);
			bulletReload = 0.0f;
		}
		int randomEnemy = rand() % enemies.size();
		if (enemyBulletReload >= 0.6f) {
			enemies[randomEnemy].shootBullet(1);
			enemyBulletReload = 0.0f;
		}
		// update bullets
		// ------- player bullets vs enemies
		for (int i = 0; i < playerBullets.size(); i++) {
			playerBullets[i].y += playerBullets[i].velY * elapsed;
			if (playerBullets[i].y >= projHeight) {
				playerBullets.erase(playerBullets.begin() + i);
			}
			else {
				for (int j = 0; j < enemies.size(); j++) {
					if (playerBullets[i].collidesWith(enemies[j])) {
						score += 100;
						playerBullets[i].health -= 1;
						enemies[j].health -= 1;
						playerBullets.erase(playerBullets.begin() + i);
						if (enemies[j].health == 0) {
							enemies.erase(enemies.begin() + j);
						}
						break;
					}
				}
			}
		}
		// ------- enemy bullets vs player
		for (int i = 0; i < enemyBullets.size(); i++) {
			enemyBullets[i].y += enemyBullets[i].velY * elapsed;
			if (enemyBullets[i].y <= -projHeight) {
				enemyBullets.erase(enemyBullets.begin() + i);
			}
			else {
				if (enemyBullets[i].collidesWith(player)) {
					enemyBullets[i].health -= 1;
					enemyBullets.erase(enemyBullets.begin() + i);
					player.health -= 1;
				}
			}
		}
		// player is dead
		if (player.health <= 0) {
			mode = LOSE;
		}
		// all enemies dead
		if (enemies.size() == 0) {
			mode = WIN;
		}
	}
	GLuint sheetTexture;
	int fontTextureID;
	SheetSprite playerSprite, enemySprite, bulletSprite;
	Entity player;
	std::vector<Entity> enemies;
	std::vector<Entity> playerBullets;
	std::vector<Entity> enemyBullets;
};

GameState state;

void Entity::shootBullet(int type) {
	Entity bullet = Entity(x, y, state.bulletSprite.width * state.bulletSprite.size,
		state.bulletSprite.height * state.bulletSprite.size, state.bulletSprite, 1.0f);
	// type = 1 = enemy bullet
	if (type) {
		bullet.velY = -2.0f;
		state.enemyBullets.push_back(bullet);
	}
	// type = 0 = player bullet
	else {
		bullet.velY = 2.0f;
		state.playerBullets.push_back(bullet);
	}
}

void DrawText(ShaderProgram* program, int fontTexture, std::string text, float x, float y, float size, float spacing) {
	Matrix modelMatrix;
	modelMatrix.Translate(x, y, 0);
	program->SetModelMatrix(modelMatrix);

	float textureSize = 1.0f / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float textureX = (float)(spriteIndex % 16) / 16.0f;
		float textureY = (float)(spriteIndex / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			});
		texCoordData.insert(texCoordData.end(), {
			textureX, textureY,
			textureX, textureY + textureSize,
			textureX + textureSize, textureY,
			textureX + textureSize, textureY + textureSize,
			textureX + textureSize, textureY,
			textureX, textureY + textureSize,
			});
	}
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, (int)text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Homework03 - Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 640, 360);
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glClearColor(0.455f, 0.0f, 0.416f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	state.loadEntities();
	state.spawnEnemies();
}

void ProcessEvents() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
}

void updateState() {
	switch (mode) {
	case STATE_TITLE_SCREEN:
		break;
	case STATE_GAME_LEVEL:
		state.Update();
		break;
	}
}

void RenderGame() {
	if (keys[SDL_SCANCODE_0]) {
		mode = STATE_TITLE_SCREEN;
	}
	glClear(GL_COLOR_BUFFER_BIT);
	// draw text
	DrawText(&program, state.fontTextureID, "Lives: " + std::to_string(state.player.health), -3.0f, -1.5f, 0.3f, -0.15f);
	DrawText(&program, state.fontTextureID, "Score: " + std::to_string(score), 1.7f, -1.5f, 0.3f, -0.15f);
	// draw entities
	state.player.Draw(program);
	for (int i = 0; i < state.enemies.size(); i++) {
		state.enemies[i].Draw(program);
	}
	bulletReload += elapsed;
	enemyBulletReload += elapsed;
	for (int i = 0; i < state.playerBullets.size(); i++) {
		state.playerBullets[i].Draw(program);
	}
	for (int i = 0; i < state.enemyBullets.size(); i++) {
		state.enemyBullets[i].Draw(program);
	}
	SDL_GL_SwapWindow(displayWindow);
}

void renderState() {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (mode) {
	case STATE_TITLE_SCREEN:
		if (keys[SDL_SCANCODE_SPACE]) {
			mode = STATE_GAME_LEVEL;
		}
		state.player.Draw(program);
		DrawText(&program, state.fontTextureID, "SPACE INVADERS", 7*(-0.5+0.12), 0.7f, 0.5f, -0.12f);
		DrawText(&program, state.fontTextureID, "SPACE to start", 7*(-0.3f+0.12), 0.2f, 0.3f, -0.12f);
		DrawText(&program, state.fontTextureID, "LEFT/RIGHT to move", 9*(-0.3f+0.12), -0.2f, 0.3f, -0.12f);
		DrawText(&program, state.fontTextureID, "UP to shoot", 11/2*(-0.3f+0.12), -0.6f, 0.3f, -0.12f);
		SDL_GL_SwapWindow(displayWindow);
		break;
	case STATE_GAME_LEVEL:
		RenderGame();
		break;
	case WIN:
		if (keys[SDL_SCANCODE_SPACE]) {
			state.reset();
			mode = STATE_TITLE_SCREEN;
		}
		state.player.Draw(program);
		DrawText(&program, state.fontTextureID, "VICTORY!", 4*(-0.5f+0.12), 0.7f, 0.5f, -0.12f);
		DrawText(&program, state.fontTextureID, "Score: " + std::to_string(score), 11/2 * (-0.3f + 0.12), 0.2f, 0.3f, -0.12f);
		DrawText(&program, state.fontTextureID, "SPACE to restart", 8*(-0.3f + 0.12), -0.2f, 0.3f, -0.12f);
		SDL_GL_SwapWindow(displayWindow);
		break;
	case LOSE:
		if (keys[SDL_SCANCODE_SPACE]) {
			state.reset();
			mode = STATE_TITLE_SCREEN;
		}
		state.player.Draw(program);
		DrawText(&program, state.fontTextureID, "YOU DIED!", 9/2 * (-0.5f + 0.12), 0.7f, 0.5f, -0.12f);
		DrawText(&program, state.fontTextureID, "Score: " + std::to_string(score), 11 / 2 * (-0.3f + 0.12), 0.2f, 0.3f, -0.12f);
		DrawText(&program, state.fontTextureID, "SPACE to restart", 8 * (-0.3f + 0.12), -0.2f, 0.3f, -0.12f);
		SDL_GL_SwapWindow(displayWindow);
		break;
	}
}

int main(int argc, char *argv[]) {
	Setup();
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		ProcessEvents();
		updateState();
		renderState();
	}
	SDL_Quit();
	return 0;
}
