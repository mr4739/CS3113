// Homework 04 - Platformer Demo
// INSTRUCTIONS:
//   - LEFT, RIGHT to walk
//   - UP to jump
//   - R to restart
//   - Player is chased by a ghost enemy and dies if caught or if player falls in lava
//   - (This game is unwinnable)


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
#include "FlareMap.h"
#include "Entity.h"
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>

#define LEVEL_HEIGHT 20
#define LEVEL_WIDTH 48
#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
using namespace std;

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
FlareMap map;
float TILE_SIZE = 0.5f;
float FRICTION = 2.0f;
float GRAVITY = 3.0f;
Matrix projectionMatrix, modelMatrix, viewMatrix;
float lastFrameTicks = 0.0f;
float elapsed = 0.0f;
float accumulator = 0.0f;
int TRX, TRY, TLX, TLY, BLX, BLY, BRX, BRY;
int CTX, CTY, CRX, CRY, CBX, CBY, CLX, CLY;
int centX, centY;
int nonsolids[] = { 0, 16, 73 };

Entity player;
SheetSprite playerSprite;
Entity enemy;
SheetSprite enemySprite;
GLuint tileset;

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return retTexture;
}
void worldToTileCoords(float worldX, float worldY, int* gridX, int* gridY) {
	*gridX = (int)(worldX / TILE_SIZE * 0.5);
	*gridY = (int)(-worldY / TILE_SIZE * 0.5);
}

bool collides(int textureID);
void renderLevel(ShaderProgram* program);
void replay();

void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Homework04 - Platformer Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 800, 400);
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	tileset = LoadTexture(RESOURCE_FOLDER"spritesheet_rgba.png");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.455f, 0.0f, 0.416f, 1.0f);

	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);
	
	map.Load("hw04map.txt");
	playerSprite = SheetSprite(tileset, 79, TILE_SIZE);
	player = Entity(13.0f, -14.0f, 0.5f, 0.5f, 0.0f, -4.0f, playerSprite, 1, PLAYER, false);
	enemySprite = SheetSprite(tileset, 445, TILE_SIZE);
	enemy = Entity(28.0f, -8.0f, 0.5f, 0.5f, 0.0f, 0.0f, enemySprite, 10, ENEMY, false);
}

void ProcessEvents() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_R) {
				replay();
			}
		}
	}
}
void Update(float elapsed) {
	if (player.health) {
		if (keys[SDL_SCANCODE_LEFT] && !player.collidedLeft) {
			player.velX = -3.0f;
		}
		else if (keys[SDL_SCANCODE_RIGHT] && !player.collidedRight) {
			player.velX = 3.0f;
		}

		player.collidedBottom = false;
		player.collidedTop = false;
		player.collidedLeft = false;
		player.collidedRight = false;

		// corners of player sprite
		worldToTileCoords(player.x + 0.5, player.y - 0.5, &centX, &centY);
		worldToTileCoords(player.x, player.y, &TLX, &TLY);
		worldToTileCoords(player.x + 1, player.y, &TRX, &TRY);
		worldToTileCoords(player.x, player.y - 1, &BLX, &BLY);
		worldToTileCoords(player.x + 1, player.y - 1, &BRX, &BRY);

		// centers of player sprite sides
		worldToTileCoords(player.x + 0.5, player.y, &CTX, &CTY);
		worldToTileCoords(player.x + 0.5, player.y - 1, &CBX, &CBY);
		worldToTileCoords(player.x, player.y - 0.5, &CLX, &CLY);
		worldToTileCoords(player.x + 1, player.y - 0.5, &CRX, &CRY);

		//-------- Top Center, Top Left and Top Right corner check
		if (collides(map.mapData[CTY][CTX]) && (collides(map.mapData[TLY][TLX]) || collides(map.mapData[TRY][TRX]))) {
			player.collidedTop = true;
		}
		//-------- Bottom Center, Bottom Left and Bottom Right corner check
		else if (collides(map.mapData[CBY][CBX]) && (collides(map.mapData[BLY][BLX]) || collides(map.mapData[BRY][BRX]))) {
			player.collidedBottom = true;
			player.isJumping = false;
		}
		//-------- Left Center, Top Left and Bottom Left corner check
		if (collides(map.mapData[CLY][CLX]) && (collides(map.mapData[BLY][BLX]) || collides(map.mapData[TLY][TLX]))) {
			player.collidedLeft = true;
		}
		//-------- Right Center, Top Right and Bottom Right corner check
		else if (collides(map.mapData[CRY][CRX]) && (collides(map.mapData[TRY][TRX]) || collides(map.mapData[BRY][BRX]))) {
			player.collidedRight = true;
		}
		if (keys[SDL_SCANCODE_UP] && !player.collidedTop && player.collidedBottom) {
			player.velY = 2.0f;
			player.jump();
		}
		
		if (player.y <= -16.0f) {
			player.health = 0;
		}

		player.Update(elapsed, FRICTION, GRAVITY);
		enemy.y += (player.y - enemy.y) / abs(player.y - enemy.y) * elapsed;
		enemy.x += (player.x - enemy.x) / abs(player.x - enemy.x) * elapsed;

		if (player.collidesWith(enemy)) {
			player.health = 0;
		}

	}
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	modelMatrix.Identity();
	program.SetModelMatrix(modelMatrix);
	program.SetViewMatrix(viewMatrix);

	renderLevel(&program);
	viewMatrix.Identity();
	player.Draw(program);
	viewMatrix.Identity();
	enemy.Draw(program);
	viewMatrix.Translate(-player.x * 0.5f, -player.y * 0.5f, 0.0f);
	program.SetViewMatrix(viewMatrix);

	SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[]) {
	Setup();
	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		ProcessEvents();
		Update(elapsed);
		Render();
	}
	SDL_Quit();
	getchar();
	return 0;
}

void replay() {
	player.x = 13.0f;
	player.y = -14.0f;
	enemy.x = 28.0f;
	enemy.y = -8.0f;
	player.health = 1;
}

void renderLevel(ShaderProgram* program) {
	modelMatrix.Identity();
	program->SetModelMatrix(modelMatrix);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetViewMatrix(viewMatrix);

	glBindTexture(GL_TEXTURE_2D, tileset);
	std::vector<float> vertexData;
	std::vector<float> texCoorData;
	for (int y = 0; y < map.mapHeight; y++) {
		for (int x = 0; x < map.mapWidth; x++) {
			if (map.mapData[y][x] != 0) {
				float u = (float)((((int)map.mapData[y][x])) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)map.mapData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
				vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
					});
				texCoorData.insert(texCoorData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
					});
			}
		}
	}
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoorData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, tileset);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

bool collides(int textureID) {
	for (int i = 0; i < 3; i++) {
		if (nonsolids[i] == textureID) {
			return false;
		}
	}
	return true;
}