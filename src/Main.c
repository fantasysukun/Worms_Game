/*********************************************************************
How to compile this on different platforms:

gcc Main.c DrawUtils.c `pkg-config --cflags --libs sdl2 gl glew` -o CFramework
*/

#define SDL_MAIN_HANDLED
#include<SDL.h>
#include<GL/glew.h>
#include<stdio.h>
#include<string.h>
#include<assert.h>
#include"DrawUtils.h"

/* Set this to true to force the game to exit */
char shouldExit = 0;

/* The previous frame's keyboard state */
unsigned char kbPrevState[SDL_NUM_SCANCODES] = { 0 };

/* The current frame's keyboard state */
const unsigned char* kbState = NULL;

/* position of the sprite */
int spritePos[2] = { 310, 235 };
int BackgroundPos[2] = { 0, 0 };

/* Texture for the sprite */
GLuint spriteTex_Right;
GLuint spriteTex_Left;
GLuint spriteTex_Current;

GLuint BackGround;

/* size of the sprite */
int spriteSize[2];
int BackgroundSize[2];

struct AnimFrameDef {
	// combined with the AnimDef's name to make
	// the actual texture name
	int frameNum;
	float frameTime;
};

struct AnimDef
{
	const char* name;
	AnimFrameDef frames[20];
	int numFrames;
};

// Runtime state for an animation
struct AnimData {
	AnimDef* def;
	int curFrame;
	float timeToNextFrame;
	bool isPlaying;
};







int main(void)
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Could not initialize SDL. ErrorCode=%s\n", SDL_GetError());
		return 1;
	}

	/* Create the window, OpenGL context */
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_Window* window = SDL_CreateWindow(
		"TestSDL",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		SDL_WINDOW_OPENGL);
	if (!window) {
		fprintf(stderr, "Could not create window. ErrorCode=%s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	SDL_GL_CreateContext(window);

	/* Make sure we have a recent version of OpenGL */
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		fprintf(stderr, "Could not initialize glew. ErrorCode=%s\n", glewGetErrorString(glewError));
		SDL_Quit();
		return 1;
	}
	if (GLEW_VERSION_3_0) {
		fprintf(stderr, "OpenGL 3.0 or greater supported: Version=%s\n",
			glGetString(GL_VERSION));
	}
	else {
		fprintf(stderr, "OpenGL max supported version is too low.\n");
		SDL_Quit();
		return 1;
	}

	/* Setup OpenGL state */
	glViewport(0, 0, 640, 480);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, 640, 480, 0, 0, 100);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Load the texture */
	spriteTex_Right = glTexImageTGAFile("Mega-Man-transparent.tga", &spriteSize[0], &spriteSize[1]);
	spriteTex_Left = glTexImageTGAFile("Mega-Man-transparent_Left.tga", &spriteSize[0], &spriteSize[1]);
	spriteTex_Current = spriteTex_Right;

	BackGround = glTexImageTGAFile("BackGround3.tga", &BackgroundSize[0], &BackgroundSize[1]);
	printf("spriteSize: ", spriteSize[0] + " spriteSize: " + spriteSize[1]);
	printf("BackgroundSize: ", BackgroundSize[0] + " BackgroundSize: " + BackgroundSize[1]);

	kbState = SDL_GetKeyboardState(NULL);

	/* The game loop */
	while (!shouldExit) {
		assert(glGetError() == GL_NO_ERROR);
		memcpy(kbPrevState, kbState, sizeof(kbPrevState));

		/* Handle OS message pump */
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				shouldExit = 1;
			}
		}

		/* Game logic */
		if (kbState[SDL_SCANCODE_ESCAPE]) { //SDL_SCANCODE_* get the keyboard state
			shouldExit = 1;
		}

		if (kbState[SDL_SCANCODE_LEFT]) {
			if (spritePos[0] > 0) {
				spritePos[0] -= 1;
				spriteTex_Current = spriteTex_Left;
			}
		}
		if (kbState[SDL_SCANCODE_RIGHT]) {
			if (spritePos[0] < 640 - spriteSize[0]) {
				spritePos[0] += 1;
				spriteTex_Current = spriteTex_Right;
			}
		}
		if (kbState[SDL_SCANCODE_UP]) {
			if (spritePos[1] > 0) {
				spritePos[1] -= 1;
			}
		}
		if (kbState[SDL_SCANCODE_DOWN]) {
			if (spritePos[1] < 480 - spriteSize[1]) {
				spritePos[1] += 1;
			}
		}

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawSprite(BackGround, BackgroundPos[0], BackgroundPos[1], 640, 480);
		glDrawSprite(spriteTex_Current, spritePos[0], spritePos[1], spriteSize[0], spriteSize[1]);


		/* Present to the player */
		SDL_GL_SwapWindow(window);
	}

	SDL_Quit();

	return 0;
}
