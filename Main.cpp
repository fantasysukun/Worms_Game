/*********************************************************************
Worms Game
Version - May 11, 2016
Project member: Kevin Lai, Kun Su, Marvin Lai, Zhou Jing
*/

#define SDL_MAIN_HANDLED
#include<SDL.h>
#include<GL/glew.h>
#include<stdio.h>
#include<string.h>
#include<assert.h>
#include <cmath>
#include <vector>
#include<string>
#include<iostream>
#include <stdio.h> 
#include"DrawUtils.h"
#include"BackgroundDef.h"
#include"StaticBackground.h"
using namespace std;

/* Set this to true to force the game to exit */
char shouldExit = 0;

/* The previous frame's keyboard state */
unsigned char kbPrevState[SDL_NUM_SCANCODES] = { 0 };

/* The current frame's keyboard state */
const unsigned char* kbState = NULL;


float projectilePos[2] = { 526, 451 };
/* Texture for the sprite */

//Static background initialize (Layer 1)
GLuint Static_BackGround[1600];
char Static_Background_Name[100];
int Static_BackGround_Size[2];

//Destroyable  background initialize (Layer 2)
GLuint Destroyable_BackGround[1600];
char Destroyable_Background_Name[100];
int Destroyable_BackGround_Size[2];

//player initialize (Layer 3)
GLuint player_Walking_Left[2];
GLuint player_Walking_Right[2];

//Enemy initialize (Layer 3)
GLuint Enemy_Right;
GLuint Enemy_Left;

//Effect initialize (Layer 3)
GLuint projectile_image;

//Numbers initialize
GLuint Numbers_Image[9];

//Other initialize 
GLuint NewSprint;
GLuint spriteTex_Current;
GLuint BackGround_bottom;
GLuint BackGround[20];


/* size of the sprite */
int spriteSize[2];
int BackgroundSize[2];
int NewSprintSize[2];
int projectileSize[2];


float curTime = 0;
int curFrame = 0;
float timeSinceStart = 0;
float frameTimeSecs = 0.0;
float secsUntilNextFrame = 500.0;
float CheckResetFramTime = 0.0;
float ResetFramTime = 500.0;

float offset = 1.0f;
bool sprite1_Alive = true;
bool sprite2_Alive = false;
int CurrentMovementNumber = 0;
float gravity = 0.5f;


//Sprite update and display function. This function need to be change to two spile function. 
//one for update with delta time, one for drawing.
void animDraw(GLuint textures[], float x, float y, float w, float h, float deltaTime)
{
	/*
	timeSinceStart += 1000 / 60;
	//curTime = timeSinceStart / 100;
	//curFrame = curTime % 2;
	frameTimeSecs = timeSinceStart / 100;
	secsUntilNextFrame -= frameTimeSecs;
	*/

	//printf("deltaTime: %f\n", deltaTime);
	secsUntilNextFrame -= deltaTime;
	//printf("secsUntilNextFrame: %f\n", secsUntilNextFrame);
	if (secsUntilNextFrame <= 0.0f) {
		//printf("secsUntilNextFrame <= CheckResetFramTime: True\n" );
		//printf("secsUntilNextFrame <= CheckResetFramTime: \n" + (secsUntilNextFrame <= CheckResetFramTime));
		secsUntilNextFrame += ResetFramTime;
		if (curFrame == 0){
			curFrame = 1;
		}
		else {
			curFrame = 0;
		}

	}

	glDrawSprite(textures[curFrame], x, y, w, h);
}

typedef struct Camera
{
	float positionX = 216.0f;
	float positionY = 216.0f;
}Camera;
Camera camera;

typedef struct Character
{
	int health;
	char name[16];
	float posX;
	float posY;
	bool* CollisionDetectionArray; //set the getbit for this array

	GLuint Character_Image;

	//HP setup
	int HP = 100;
	GLuint HP_Image[3];
	
}Character;

//modified player to have an array of 4 characters - Marvin
typedef struct Player
{
	float positionX = 526.0f;
	float positionY = 451.0f;
	Character characters[4];
	 

}Player;
Player player;
Player playerOne, playerTwo;


//Enemy Position setup
typedef struct Enemy
{
	float positionX;
	float positionY;
	int WalkingPosition;

}Enemy;
Enemy EnemyOne, EnemyTwo;

/*
void updatePlayerPos(float x, float y) {
player.positionX = x;
}
*/
typedef struct Projectile
{
	float posX;
	float posY;
	float speed = 2.0f;
	float velocityX = 0.0f;
	float velocityY = 0.0f;
	float SpawnTime;

}Projectile;

std::vector<Projectile> projectilesVector;
std::vector<Projectile> DrawProjectiles;

void updateProjectile(Projectile* p, float dt)
{
	p->posX += p->velocityX * dt;
	p->posY += p->velocityY * dt;
}

//Need to double check
bool AABB(float x, float y, float w, float h, float x2, float y2, float w2, float h2)
{
	if (x > x2 + w2) {
		return false;
	}
	if (x + w < x2) {
		return false;
	}
	if (y > y2 + h2) {
		return false;
	}
	if (y + h < y2) {
		return false;
	}
	return true;
}

/*
Obtains the alpha array of the image file.
- Kevin Lai
*/
bool* getBytes(const char* filename){

	const int BPP = 4;

	/* open the file */
	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		fprintf(stderr, "File: %s -- Could not open for reading.\n", filename);
		return 0;
	}

	/* skip first two bytes of data we don't need */
	fseek(file, 2, SEEK_CUR);

	/* read in the image type.  For our purposes the image type should
	* be either a 2 or a 3. */
	unsigned char imageTypeCode;
	fread(&imageTypeCode, 1, 1, file);
	if (imageTypeCode != 2 && imageTypeCode != 3) {
		fclose(file);
		fprintf(stderr, "File: %s -- Unsupported TGA type: %d\n", filename, imageTypeCode);
		return 0;
	}

	/* skip 9 bytes of data we don't need */
	fseek(file, 9, SEEK_CUR);

	/* read image dimensions */
	int imageWidth = 0;
	int imageHeight = 0;
	int bitCount = 0;
	fread(&imageWidth, sizeof(short), 1, file);
	fread(&imageHeight, sizeof(short), 1, file);
	fread(&bitCount, sizeof(unsigned char), 1, file);
	fseek(file, 1, SEEK_CUR);

	/* allocate memory for image data and read it in */
	unsigned char* bytes = (unsigned char*)calloc(imageWidth * imageHeight * BPP, 1);

	// Added new boolean array called collision - Kevin Lai
	bool* collision = new bool[imageWidth * imageHeight];

	/* read in data */
	if (bitCount == 32) {
		int it;
		for (it = 0; it != imageWidth * imageHeight; ++it) {
			bytes[it * BPP + 0] = fgetc(file);
			bytes[it * BPP + 1] = fgetc(file);
			bytes[it * BPP + 2] = fgetc(file);
			bytes[it * BPP + 3] = fgetc(file);
		}

		// Added this as well. Not sure if it is needed for else statement also. - Kevin Lai
		// Also record the alpha being zero or non-zero
		bool isNonZero = (bytes[it * BPP + 3] != 0);
		collision[it] = isNonZero;

	}
	else {
		int it;
		for (it = 0; it != imageWidth * imageHeight; ++it) {
			bytes[it * BPP + 0] = fgetc(file);
			bytes[it * BPP + 1] = fgetc(file);
			bytes[it * BPP + 2] = fgetc(file);
			bytes[it * BPP + 3] = 255;
		}
	}

	fclose(file);

	return collision;
}

/*
Added pixel perfect detection
- Kevin Lai
*/
bool pixelPerfect(const char* filename1, const char* filename2, float x, float y, float w, float h, float x2, float y2, float w2, float h2){

	/*
	Intersection: X, Y, Width, Height
	Left, Top, Right, Bottom
	*/
	float intersectionX, intersectionY, intersectionW, intersectionH;

	if (x > x2){
		intersectionX = x;
	}
	else{
		intersectionX = x2;
	}
	if ((x + w) > (x2 + w2)){
		/*
		Intersection end point = (x2 + w2), intersection start point = intersectionX
		Simplified version of (intersection end point - intersection start point) = (x2 + w2) - intersectionX
		*/
		intersectionW = (x2 + w2) - intersectionX;
	}
	else{
		/*
		Intersection end point = (x + w), intersection start point = intersectionX
		Simplified version of (intersection end point - intersection start point) = (x + w) - intersectionX
		*/
		intersectionW = (x + w) - intersectionX;
	}

	if (y < y2){
		intersectionY = y2;
	}
	else{
		intersectionY = y;
	}
	if ((y + h) < (y2 + h2)){
		/*
		Intersection end point = (y + h), intersection start point = intersectionY
		Simplified version of (intersection end point - intersection start point) = (y + h) - intersectionY
		*/
		intersectionH = (y + h) - intersectionY;
	}
	else{
		/*
		Intersection end point = (y2 + h2), intersection start point = intersectionY
		Simplified version of (intersection end point - intersection start point) = (y2 + h2) - intersectionY
		*/
		intersectionH = (y2 + h2) - intersectionY;
	}

	// The alpha array of each object involved in the collision
	bool* object1 = getBytes(filename1);
	bool* object2 = getBytes(filename2);

	float offsetX = intersectionX - x, offsetX2 = intersectionX - x2, offsetY = intersectionY - y, offsetY2 = intersectionY - y2;

	for (int i = 0; i < intersectionH; ++i){

		float tempY = i + offsetY;
		float tempY2 = i + offsetY2;

		for (int j = 0; j < intersectionW; ++j){

			float tempX = j + offsetX;
			float tempX2 = j + offsetX2;

			//if ((object1[i*j] << ((int)(w - intersectionW)*32) ) & object2[i*j]){
			//	return true;
			//}

			/*
			Alpha array of each object is used in pixel perfect collision detection.
			If the alpha's of both objects are 1, then there is a collision
			*/
			if (object1[(int)(w * tempY + tempX)] & object2[(int)(w2 * tempY2 + tempX2)]){
				return true;
			}
		}
	}

	return false;
}



void updatePlayerPos(float PlayerPosX, float PlayerPosY) {
	player.positionX = PlayerPosX;
	player.positionY = PlayerPosY;
}

//Need to double check
void CollisionResolution(float ObjectAPosX, float ObjectAPosY, float ObjectASizeX, float ObjectASizeY,
	float ObjectBPosX, float ObjectBPosY, float ObjectBSizeX, float ObjectBSizeY) {
	if (AABB(ObjectAPosX, ObjectAPosY, ObjectASizeX, ObjectASizeY, ObjectBPosX, ObjectBPosY, ObjectBSizeX, ObjectBSizeY))
	{
		//printf("ObjectAPosX: %d\n, ObjectAPosY: %d\n", ObjectAPosX, ObjectAPosY);
		if (kbState[SDL_SCANCODE_LEFT]) {
			if (ObjectAPosX < ObjectBPosX + ObjectBSizeX && ObjectAPosX + ObjectASizeX > ObjectBPosX + ObjectBSizeX) {
				//ObjectAPosX < ObjectBPosX && ObjectAPosX + ObjectASizeX > ObjectBPosX && ObjectAPosX + ObjectASizeX < ObjectBPosX + ObjectASizeX
				ObjectAPosX += ObjectBPosX + ObjectBSizeX - ObjectAPosX;
				camera.positionX += offset;
			}
		}
		if (kbState[SDL_SCANCODE_RIGHT]) {
			if (ObjectAPosX + ObjectASizeX > ObjectBPosX && ObjectAPosX + ObjectASizeX < ObjectBPosX + ObjectASizeX) {
				ObjectAPosX -= ObjectAPosX + ObjectASizeX - ObjectBPosX;
				camera.positionX -= offset;
			}
		}
		if (kbState[SDL_SCANCODE_UP]) {
			if (ObjectAPosY < ObjectBPosY + ObjectBSizeY && ObjectAPosY + ObjectASizeY > ObjectBPosY + ObjectBSizeY) {
				//(ObjectAPosY < ObjectBPosY && ObjectAPosY + ObjectASizeY > ObjectBPosY && ObjectAPosY + ObjectASizeY < ObjectBPosY + ObjectASizeY
				ObjectAPosY += ObjectBPosY + ObjectBSizeY - ObjectAPosY;
				camera.positionY += offset;
			}
		}
		if (kbState[SDL_SCANCODE_DOWN]) {
			if (ObjectAPosY + ObjectASizeY > ObjectBPosY && ObjectAPosY + ObjectASizeY < ObjectBPosY + ObjectASizeY) {
				ObjectAPosY -= ObjectAPosY + ObjectASizeY - ObjectBPosY;
				camera.positionY -= offset;
			}
		}
	}
	updatePlayerPos(ObjectAPosX, ObjectAPosY);
}

//Set the camera to middle
void SetTheCameraToMiddle(char angle[]) {

	if (angle == "LEFT") {
		if (camera.positionX > 0) {
			if (player.positionX < 1440 - 320 - spriteSize[0] / 2) { camera.positionX -= offset; }
		}
	}
	if (angle == "RIGHT") {
		if (camera.positionX < 1440 - 640) {
			if (player.positionX > 320 + spriteSize[0] / 2) { camera.positionX += offset; }
		}
	}
	if (angle == "UP") {
		if (camera.positionY > 0) {
			if (player.positionY < 1440 - 240 - spriteSize[1] / 2) { camera.positionY -= offset; }
		}
	}
	if (angle == "DOWN") {
		if (camera.positionY < 1440 - 480) {
			if (player.positionY > 240 + spriteSize[1] / 2) { camera.positionY += offset; }
		}
	}
}

typedef struct WalkPathPosition
{
	float positionX;
	float positionY;

}WalkPathPosition;

WalkPathPosition WalkPathPositionOne;
WalkPathPosition WalkPathPositionTwo;
WalkPathPosition WalkPathPositionThree;
WalkPathPosition WalkPathPositionFour;

WalkPathPosition WalkPathPositionArray[];

//Load the whole static background
void LoadStatic_Background() {
	int count = 0;
	for (int y = 0; y < 40; y++)
	{
		for (int x = 0; x < 40; x++)
		{
			sprintf_s(Static_Background_Name, sizeof Static_Background_Name, "ArtResource/Static_background/Static_Background%d.tga", count + 1);
			try {
				Static_BackGround[count] = glTexImageTGAFile(Static_Background_Name, &Static_BackGround_Size[0], &Static_BackGround_Size[1]);
				count++;
			}
			catch (exception e) {

			}
		}
	}
}

//Load the whole Destroyable background
void LoadDestroyable_Background() {
	int count = 0;
	
	for (int y = 0; y < 40; y++)
	{
		for (int x = 0; x < 40; x++)
		{
			sprintf_s(Destroyable_Background_Name, sizeof Destroyable_Background_Name, "ArtResource/Destroyable_background/Destroyable_Background%d.tga", count + 1);
			try {
				Destroyable_BackGround[count] = glTexImageTGAFile(Destroyable_Background_Name, &Destroyable_BackGround_Size[0], &Destroyable_BackGround_Size[1]);
				count++;
			}
			catch (exception e) {

			}
		}
	}
}

void player_characters_HP_Image_initializationAndupdate(Character characters)
{
	try
	{
		characters.HP_Image[0] = Numbers_Image[characters.HP / 100];
		characters.HP_Image[1] = Numbers_Image[(characters.HP / 10) % 10];
		characters.HP_Image[2] = Numbers_Image[characters.HP % 10];
	}
	catch (exception e) {}
}
//***************** Main function *****************
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

	kbState = SDL_GetKeyboardState(NULL);



	/* Load the texture */
	Enemy_Right = glTexImageTGAFile("ArtResource/Enemy_Right.tga", &spriteSize[0], &spriteSize[1]);
	Enemy_Left = glTexImageTGAFile("ArtResource/Enemy_Left.tga", &spriteSize[0], &spriteSize[1]);

	
	spriteTex_Current = Enemy_Right;
	
	bool Testing_C = true;
	int Testing_Character_size[2];
	try {
		playerOne.characters[0].Character_Image = glTexImageTGAFile("ArtResource/Character/Character_Left1.tga", &Testing_Character_size[0], &Testing_Character_size[1]);
		playerOne.characters[0].Character_Image = glTexImageTGAFile("ArtResource/Character/Character_Left2.tga", NULL, NULL);
		playerOne.characters[0].Character_Image = glTexImageTGAFile("ArtResource/Character/Character_Right1.tga", NULL, NULL);
		playerOne.characters[0].Character_Image = glTexImageTGAFile("ArtResource/Character/Character_Right2.tga", NULL, NULL);
	}
	catch (exception e) {
		Testing_C = false;
	}

	

	try {
		playerOne.characters[0].Character_Image = glTexImageTGAFile("ArtResource/Character1.tga", &spriteSize[0], &spriteSize[1]);
		playerOne.characters[0].Character_Image = glTexImageTGAFile("ArtResource/Character2.tga", &spriteSize[0], &spriteSize[1]);
		playerOne.characters[0].Character_Image = glTexImageTGAFile("ArtResource/Character3.tga", &spriteSize[0], &spriteSize[1]);
		playerOne.characters[0].Character_Image = glTexImageTGAFile("ArtResource/Character4.tga", &spriteSize[0], &spriteSize[1]);
	}
	catch (exception e) {

	}


	



	//BackGround_bottom = glTexImageTGAFile("Background_bottom.tga", NULL, NULL);
	BackGround[0] = glTexImageTGAFile("ArtResource/13.tga", NULL, NULL);
	BackGround[1] = glTexImageTGAFile("ArtResource/14.tga", NULL, NULL);
	BackGround[2] = glTexImageTGAFile("ArtResource/4.tga", NULL, NULL);
	BackGround[3] = glTexImageTGAFile("ArtResource/Static_Background1.tga", NULL, NULL);

	player_Walking_Left[0] = glTexImageTGAFile("ArtResource/Left.tga", NULL, NULL);
	player_Walking_Left[1] = glTexImageTGAFile("ArtResource/Left_2.tga", NULL, NULL);
	player_Walking_Right[0] = glTexImageTGAFile("ArtResource/Right.tga", NULL, NULL);
	player_Walking_Right[1] = glTexImageTGAFile("ArtResource/Right_2.tga", NULL, NULL);

	player_Walking_Left[0] = glTexImageTGAFile("Backup_Art/Worms_Armageddon_Sprites_1.tga", &Testing_Character_size[0], &Testing_Character_size[1]);
	player_Walking_Left[1] = glTexImageTGAFile("Backup_Art/WormsFort3D_sprite1.tga", NULL, NULL);

	bool Testing_SB = true;
	GLuint Testing_Static_Background;
	int Testing_Static_Background_size[2];
	try {
		Testing_Static_Background = glTexImageTGAFile("ArtResource/Static_background/Static_Background.tga", &Testing_Static_Background_size[0], &Testing_Static_Background_size[1]);
	}
	catch (exception e) {
		Testing_SB = false;
	}
	
	bool Testing_DB = true;
	GLuint Testing_Destroyable_Background;
	int Testing_Destroyable_Background_size[2];
	try {
		Testing_Destroyable_Background = glTexImageTGAFile("ArtResource/Destroyable_background/Destroyable_Background.tga", &Testing_Destroyable_Background_size[0], &Testing_Destroyable_Background_size[1]);
	}
	catch (exception e) {
		Testing_DB = false;
	}

	//Loading numberes
	char Numbers_Name[50]; 
	int Numbers_Size[2];
	for (int i = 0; i < 9; i++)
	{
		sprintf_s(Numbers_Name, sizeof Numbers_Name, "ArtResource/Numbers/%d.tga", i);
		try {
			Numbers_Image[i] = glTexImageTGAFile(Numbers_Name, &Numbers_Size[0], &Numbers_Size[1]);
		}
		catch (exception e) {

		}
	}
	
	//aracters HP image initialization;
	
	try
	{
		player_characters_HP_Image_initializationAndupdate(playerOne.characters[0]);
		player_characters_HP_Image_initializationAndupdate(playerOne.characters[1]);
		player_characters_HP_Image_initializationAndupdate(playerOne.characters[2]);
		player_characters_HP_Image_initializationAndupdate(playerOne.characters[3]);
		player_characters_HP_Image_initializationAndupdate(playerTwo.characters[0]);
		player_characters_HP_Image_initializationAndupdate(playerTwo.characters[1]);
		player_characters_HP_Image_initializationAndupdate(playerTwo.characters[2]);
		player_characters_HP_Image_initializationAndupdate(playerTwo.characters[3]);
	}
	catch (exception e) {

	}
	

	//Loading for static background
	LoadStatic_Background();

	//Loading for destroyable background
	LoadDestroyable_Background();

	//Static_BackGround = glTexImageTGAFile("ArtResource/Static_Background.tga", &Static_BackGroundSize[0], &Static_BackGroundSize[1]);

	//NewSprint = glTexImageTGAFile("1.tga", &NewSprintSize[0], &NewSprintSize[1]);
	projectile_image = glTexImageTGAFile("ArtResource/projectile.tga", &projectileSize[0], &projectileSize[1]);
	/* Time setting */
	float lastFrameNS = 0.0f;
	float curFrameNS = (float)SDL_GetTicks();
	//printf("curFrameNS: %f\n", curFrameNS);
	//float curFrameMs = curFrameNS / 100.0f;

	// Physics time setting
	float physicsDeltaMs = 100.0f;
	float lastPhysicsFrameMs = 0.0f;

	// Mouse setting
	int mouseX, mouseY, mouseDeltaX, mouseDeltaY;
	Uint32 mouseButtons = 0;
	Uint32 prevMouseButtons = 0;

	prevMouseButtons = mouseButtons;

	Projectile p1;
	Projectile p2;
	Projectile p3;
	Projectile p4;
	Projectile p5;
	Projectile p6;
	Projectile p7;
	Projectile p8;
	Projectile p9;
	Projectile p10;
	Projectile p11;
	Projectile p12;
	Projectile p13;
	Projectile p14;
	Projectile p15;
	Projectile p16;
	Projectile p17;
	Projectile p18;
	Projectile p19;
	Projectile p20;
	Projectile p21;
	Projectile p22;
	Projectile p23;
	Projectile p24;
	Projectile p25;
	Projectile p26;
	Projectile p27;
	Projectile p28;
	Projectile p29;
	Projectile p30;
	projectilesVector.push_back(p1);
	projectilesVector.push_back(p2);
	projectilesVector.push_back(p3);
	projectilesVector.push_back(p4);
	projectilesVector.push_back(p5);
	projectilesVector.push_back(p6);
	projectilesVector.push_back(p7);
	projectilesVector.push_back(p8);
	projectilesVector.push_back(p9);
	projectilesVector.push_back(p10);
	projectilesVector.push_back(p11);
	projectilesVector.push_back(p12);
	projectilesVector.push_back(p13);
	projectilesVector.push_back(p14);
	projectilesVector.push_back(p15);
	projectilesVector.push_back(p16);
	projectilesVector.push_back(p17);
	projectilesVector.push_back(p18);
	projectilesVector.push_back(p19);
	projectilesVector.push_back(p20);
	projectilesVector.push_back(p21);
	projectilesVector.push_back(p22);
	projectilesVector.push_back(p23);
	projectilesVector.push_back(p24);
	projectilesVector.push_back(p25);
	projectilesVector.push_back(p26);
	projectilesVector.push_back(p27);
	projectilesVector.push_back(p28);
	projectilesVector.push_back(p29);
	projectilesVector.push_back(p30);

	//WalkingPosition setup
	EnemyOne.WalkingPosition = 0;
	EnemyTwo.WalkingPosition = 2;

	//Enemt WalkPathPosition setup
	WalkPathPositionOne.positionX = 200.0f;
	WalkPathPositionOne.positionY = 200.0f;
	WalkPathPositionTwo.positionX = 200.0f;
	WalkPathPositionTwo.positionY = 800.0f;
	WalkPathPositionThree.positionX = 800.0f;
	WalkPathPositionThree.positionY = 800.0f;
	WalkPathPositionFour.positionX = 800.0f;
	WalkPathPositionFour.positionY = 200.0f;
	WalkPathPosition WalkPathPositionArray[] = { WalkPathPositionOne, WalkPathPositionTwo, WalkPathPositionThree, WalkPathPositionFour };

	//Enemy Position setup
	EnemyOne.positionX = 688.0f;
	EnemyOne.positionY = 474.0f;
	EnemyTwo.positionX = 426.0f;
	EnemyTwo.positionY = 351.0f;

	/* The game loop */
	while (!shouldExit) {
		assert(glGetError() == GL_NO_ERROR);
		memcpy(kbPrevState, kbState, sizeof(kbPrevState));
		lastFrameNS = curFrameNS;


		curFrameNS = (float)SDL_GetTicks();
		//curFrameMs = curFrameNS / 1000000.0f;
		float deltaTime = (curFrameNS - lastFrameNS);
		//printf("\ndeltaTime: %f", deltaTime);
		prevMouseButtons = mouseButtons;

		/* Handle OS message pump */
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				shouldExit = 1;
			}
		}

		mouseButtons = SDL_GetMouseState(&mouseX, &mouseY); //get Mouse X, Y
		SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);

		if (mouseButtons&SDL_BUTTON_LMASK == 1 && !prevMouseButtons&SDL_BUTTON_LMASK == 1)
		{
			mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
			float mousePosX = (float)mouseX + camera.positionX;
			float mousePosY = (float)mouseY + camera.positionY;
			//printf("\nmousePosX: %f", mousePosX);
			//printf("\nmousePosY: %f", mousePosY);
			if (projectilesVector.size() > 0)
			{

				//printf("Running~~~~~~~~~~~~~~~~~~~~~~~~~~~");
				Projectile projectile = projectilesVector.back();
				projectile.posX = player.positionX;
				projectile.posY = player.positionY;

				projectile.SpawnTime = curFrameNS;
				projectilesVector.pop_back();



				float slope = std::abs(std::atan((mousePosY - projectile.posY) / (mousePosX - projectile.posX)));
				//printf("\nslope: %f", slope);
				projectile.velocityX = projectile.speed * std::cos(slope);
				projectile.velocityY = projectile.speed * std::sin(slope);
				//printf("\nmousePosX - projectile.posX: %f", mousePosX - projectile.posX);
				if (mousePosX - projectile.posX < 0)
				{
					projectile.velocityX *= -1;
				}
				//printf("\nmousePosY - projectile.posY: %f", mousePosY - projectile.posY);
				if (mousePosY - projectile.posY < 0)
				{
					projectile.velocityY *= -1;
				}

				DrawProjectiles.push_back(projectile);


			}
			//printf("\n\n RUNnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn: %d", SDL_GetTicks());
		}

		//Enemy Update
		if (sprite1_Alive) {

			float RandomMovingSpeed = rand() / (float)RAND_MAX / 10;
			if (EnemyOne.positionX < WalkPathPositionArray[EnemyOne.WalkingPosition].positionX) {
				EnemyOne.positionX += RandomMovingSpeed;
				CurrentMovementNumber++;
			}
			else if (EnemyOne.positionX > WalkPathPositionArray[EnemyOne.WalkingPosition].positionX) {
				EnemyOne.positionX -= RandomMovingSpeed;
				CurrentMovementNumber++;
			}
			if (EnemyOne.positionY < WalkPathPositionArray[EnemyOne.WalkingPosition].positionY) {
				EnemyOne.positionY += RandomMovingSpeed;
				CurrentMovementNumber++;
			}
			else if (EnemyOne.positionY > WalkPathPositionArray[EnemyOne.WalkingPosition].positionY) {
				EnemyOne.positionY -= RandomMovingSpeed;
				CurrentMovementNumber++;
			}

			if (CurrentMovementNumber == 13000) {
				EnemyOne.WalkingPosition += 1;
				CurrentMovementNumber = 0;
				if (EnemyOne.WalkingPosition == 4) { EnemyOne.WalkingPosition = 0; }
			}

			float RandomShootingSpeed = rand() / (float)RAND_MAX;
			//printf("\nRandomShootingSpeed : %f", RandomShootingSpeed);
			if (RandomShootingSpeed < 0.005f) {
				if (projectilesVector.size() > 0)
				{

					Projectile projectile = projectilesVector.back();
					projectile.posX = EnemyOne.positionX;
					projectile.posY = EnemyOne.positionY;

					projectile.SpawnTime = curFrameNS;
					projectilesVector.pop_back();
					float slopeFromEnemyOneToPlayer = std::abs(std::atan((player.positionY - projectile.posY) / (player.positionX - projectile.posX)));
					projectile.velocityX = projectile.speed * std::cos(slopeFromEnemyOneToPlayer);
					projectile.velocityY = projectile.speed * std::sin(slopeFromEnemyOneToPlayer);
					if (player.positionX - projectile.posX < 0)
					{
						projectile.velocityX *= -1;
					}

					if (player.positionY - projectile.posY < 0)
					{
						projectile.velocityY *= -1;
					}
					projectile.posX = EnemyOne.positionX + projectile.velocityX * (spriteSize[0] / 2 + projectileSize[0] / 2);
					projectile.posY = EnemyOne.positionY + projectile.velocityY * (spriteSize[1] / 2 + projectileSize[1] / 2);

					DrawProjectiles.push_back(projectile);

				}
			}
		}

		// Physics update
		do {
			// 1. Physics movement
			//printf("\n\n !!!curFrameMs: %d", curFrameNS);
			//player.positionY += gravity;
			//printf("\n\n player.positionY: %d", player.positionY);
			// Update Projectiles
			for (int i = 0; i < DrawProjectiles.size(); i++)
			{
				updateProjectile(&DrawProjectiles[i], deltaTime);
			}

			// 2. Physics collision detection
			for (int i = 0; i < DrawProjectiles.size(); i++)
			{
				if (!(camera.positionY <= DrawProjectiles[i].posY && DrawProjectiles[i].posY <= camera.positionY + 480		//display image on-screen only
					&& camera.positionX <= DrawProjectiles[i].posX && DrawProjectiles[i].posX <= camera.positionX + 640))
				{
					if (DrawProjectiles[i].posX < 1440)
					{
						projectilesVector.push_back(DrawProjectiles[i]);
					}
					DrawProjectiles.erase(DrawProjectiles.begin() + i);
				}
			}

			for (int i = 0; i < DrawProjectiles.size(); i++)
			{
				if (sprite1_Alive) {
					if (AABB(DrawProjectiles[i].posX, DrawProjectiles[i].posY, projectileSize[0], projectileSize[1], EnemyOne.positionX, EnemyOne.positionY, spriteSize[0], spriteSize[1]))
					{

						projectilesVector.push_back(DrawProjectiles[i]);
						sprite1_Alive = false;
						DrawProjectiles.erase(DrawProjectiles.begin() + i);
						sprite2_Alive = true;
					}
				}
			}

			for (int i = 0; i < DrawProjectiles.size(); i++)
			{
				if (sprite2_Alive) {
					if (AABB(DrawProjectiles[i].posX, DrawProjectiles[i].posY, projectileSize[0], projectileSize[1], 426, 351, spriteSize[0], spriteSize[1]))
					{
						projectilesVector.push_back(DrawProjectiles[i]);
						sprite2_Alive = false;
						DrawProjectiles.erase(DrawProjectiles.begin() + i);
						sprite1_Alive = true;
					}
				}
			}

			for (int i = 0; i < DrawProjectiles.size(); i++)
			{
				if (AABB(DrawProjectiles[i].posX, DrawProjectiles[i].posY, projectileSize[0], projectileSize[1], 0, 1296, 1440, 144))
				{
					projectilesVector.push_back(DrawProjectiles[i]);
					DrawProjectiles.erase(DrawProjectiles.begin() + i);
				}
			}

			for (int i = 0; i < DrawProjectiles.size(); i++)
			{
				if (AABB(DrawProjectiles[i].posX, DrawProjectiles[i].posY, projectileSize[0], projectileSize[1], 936, 936, 108, 108))
				{
					projectilesVector.push_back(DrawProjectiles[i]);
					DrawProjectiles.erase(DrawProjectiles.begin() + i);
				}
			}

			// 3. Physics collision resolution
			if (sprite1_Alive) {

				//CollisionResolution(player.positionX, player.positionY, spriteSize[0], spriteSize[1], EnemyOne.positionX, EnemyOne.positionY, spriteSize[0], spriteSize[1]);

				if (AABB(player.positionX, player.positionY, spriteSize[0], spriteSize[1], EnemyOne.positionX, EnemyOne.positionY, spriteSize[0], spriteSize[1]))
				{
					if (kbState[SDL_SCANCODE_LEFT]) {
						if (player.positionX > EnemyOne.positionX && player.positionX + spriteSize[0] > EnemyOne.positionX + spriteSize[0]) {
							player.positionX += EnemyOne.positionX + spriteSize[0] - player.positionX;
							//camera.positionX += offset;
							SetTheCameraToMiddle("RIGHT");
						}
					}
					if (kbState[SDL_SCANCODE_RIGHT]) {
						if (player.positionX < EnemyOne.positionX && player.positionX + spriteSize[0] > EnemyOne.positionX && player.positionX + spriteSize[0] < EnemyOne.positionX + spriteSize[0]) {
							player.positionX -= player.positionX + spriteSize[0] - EnemyOne.positionX;
							printf("\nplayer.positionX: %f", player.positionX);
							//camera.positionX -= offset;
							SetTheCameraToMiddle("LEFT");
						}
					}
					if (kbState[SDL_SCANCODE_UP]) {
						if (player.positionY > EnemyOne.positionY && player.positionY + spriteSize[1] > EnemyOne.positionY + spriteSize[1]) {
							player.positionY += EnemyOne.positionY + spriteSize[1] - player.positionY;
							//camera.positionY += offset;
							SetTheCameraToMiddle("DOWN");
						}
					}
					if (kbState[SDL_SCANCODE_DOWN]) {
						if (player.positionY < EnemyOne.positionY && player.positionY + spriteSize[1] > EnemyOne.positionY && player.positionY + spriteSize[1] < EnemyOne.positionY + spriteSize[1]) {
							player.positionY -= player.positionY + spriteSize[1] - EnemyOne.positionY;
							//camera.positionY -= offset;
							SetTheCameraToMiddle("UP");
						}
					}
				}

			}

			if (sprite2_Alive) {
				if (AABB(player.positionX, player.positionY, spriteSize[0], spriteSize[1], 426, 351, spriteSize[0], spriteSize[1]))
				{
					if (kbState[SDL_SCANCODE_LEFT]) {
						if (player.positionX > 426 && player.positionX + spriteSize[0] > 426 + spriteSize[0]) {
							player.positionX += 426 + spriteSize[0] - player.positionX;
							//camera.positionX += offset;
							SetTheCameraToMiddle("RIGHT");
						}
					}
					if (kbState[SDL_SCANCODE_RIGHT]) {
						if (player.positionX < 426 && player.positionX + spriteSize[0] > 426 && player.positionX + spriteSize[0] < 426 + spriteSize[0]) {
							player.positionX -= player.positionX + spriteSize[0] - 426;
							printf("\nplayer.positionX: %f", player.positionX);
							//camera.positionX -= offset;
							SetTheCameraToMiddle("LEFT");
						}
					}
					if (kbState[SDL_SCANCODE_UP]) {
						if (player.positionY > 351 && player.positionY + spriteSize[1] > 351 + spriteSize[1]) {
							player.positionY += 351 + spriteSize[1] - player.positionY;
							//camera.positionY += offset;
							SetTheCameraToMiddle("DOWN");
						}
					}
					if (kbState[SDL_SCANCODE_DOWN]) {
						if (player.positionY < 351 && player.positionY + spriteSize[1] > 351 && player.positionY + spriteSize[1] < 351 + spriteSize[1]) {
							player.positionY -= player.positionY + spriteSize[1] - 351;
							//camera.positionY -= offset;
							SetTheCameraToMiddle("UP");
						}
					}
				}
			}

			//sprite 

			if (player.positionY + spriteSize[1] > 1296) {
				player.positionY -= player.positionY + spriteSize[1] - 1296;
			}
			//

			//CollisionResolution(player.positionX, player.positionY, spriteSize[0], spriteSize[1], 936, 936, 108, 108);

			lastPhysicsFrameMs += physicsDeltaMs;
		} while (lastPhysicsFrameMs + physicsDeltaMs < curFrameNS);




		//printf("lastFrameNS: %d\n", lastFrameNS);
		curFrameNS = SDL_GetTicks();
		//printf("currentFrameNS: %d\n", currentFrameNS);

		/* Game logic */
		if (kbState[SDL_SCANCODE_ESCAPE]) { //SDL_SCANCODE_* get the keyboard state
			shouldExit = 1;
		}

		if (kbState[SDL_SCANCODE_LEFT]) {
			if (player.positionX > 0 + 1) {
				player.positionX -= offset;
				SetTheCameraToMiddle("LEFT");
				spriteTex_Current = Enemy_Left;
			}
		}
		if (kbState[SDL_SCANCODE_RIGHT]) {
			if (player.positionX < 1440 - spriteSize[0]) {
				player.positionX += offset;
				SetTheCameraToMiddle("RIGHT");
				spriteTex_Current = Enemy_Right;
			}
		}
		if (kbState[SDL_SCANCODE_UP]) {
			if (player.positionY > 0 + 1) {
				player.positionY -= offset;
				SetTheCameraToMiddle("UP");
			}
		}
		if (kbState[SDL_SCANCODE_DOWN]) {
			if (player.positionY < 1440 - spriteSize[1]) {
				player.positionY += offset;
				SetTheCameraToMiddle("DOWN");
			}
		}

		//****** Camera setting ******//
		if (kbState[SDL_SCANCODE_A]) {
			if (camera.positionX > 0) {
				camera.positionX -= offset;
			}
		}
		if (kbState[SDL_SCANCODE_D]) {
			if (camera.positionX < 1440 - 640) {
				camera.positionX += offset;
			}
		}
		if (kbState[SDL_SCANCODE_W]) {
			if (camera.positionY > 0) {
				camera.positionY -= offset;
			}
		}
		if (kbState[SDL_SCANCODE_S]) {
			if (camera.positionY < 1440 - 480) {
				camera.positionY += offset;
			}
		}

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);


		//****** Draw BackGronud ******//
		//old background, this background will be replaced once the new background is loaded

		//Static background drawing (Layer 1)
		for (int y = 0; y < 40; y++)
		{
			for (int x = 0; x < 40; x++)
			{
				int tileNum = getTile(x, y);
				if (camera.positionY / 36 <= y + 11 && y <= camera.positionY / 36 + 15		//display image on-screen only
					&& camera.positionX / 36 <= x + 1 && x <= camera.positionX / 36 + 19) {// 
					if (Static_BackGround[getStaticBackground(x, y)] != NULL) {
						try {
							glDrawSprite(Static_BackGround[getStaticBackground(x, y)], 36 * x - camera.positionX, 36 * y - camera.positionY, 36, 36);
						}
						catch (exception e) {

						}
					}
				}
			}
		}

		//Destroyable background drawing (Layer 2)
		for (int y = 0; y < 40; y++)
		{
			for (int x = 0; x < 40; x++)
			{
				int tileNum = getTile(x, y);
				if (camera.positionY / 36 <= y + 11 && y <= camera.positionY / 36 + 15		//display image on-screen only
					&& camera.positionX / 36 <= x + 1 && x <= camera.positionX / 36 + 19) {// 
					if (Destroyable_BackGround[getDestroyableBackground(x, y)] != NULL) {
						try {
							glDrawSprite(Destroyable_BackGround[getDestroyableBackground(x, y)], 36 * x - camera.positionX, 36 * y - camera.positionY, 36, 36);
						}
						catch (exception e) {

						}
					}
					else {
						glDrawSprite(BackGround[tileNum], 36 * x - camera.positionX, 36 * y - camera.positionY, 36, 36);
					}
				}
			}
		}

		//Testing background is for artist to do art testing
		if (Testing_SB) {
			if (camera.positionY <= player.positionY && player.positionY <= camera.positionY + 480		//display image on-screen only
				&& camera.positionX <= player.positionX && player.positionX <= camera.positionX + 640) {
				glDrawSprite(Testing_Static_Background, 0 - camera.positionX, 0 - camera.positionY, Testing_Static_Background_size[0], Testing_Static_Background_size[1]);
			}
		}
		
		if (Testing_DB) {
			if (camera.positionY <= player.positionY && player.positionY <= camera.positionY + 480		//display image on-screen only
				&& camera.positionX <= player.positionX && player.positionX <= camera.positionX + 640) {
				glDrawSprite(Testing_Destroyable_Background, 0 - camera.positionX, 0 - camera.positionY, Testing_Destroyable_Background_size[0], Testing_Destroyable_Background_size[1]);
			}
		}
		
		//Sprite drawing (Layer 3)
		if (spriteTex_Current == Enemy_Left)
		{
			if (camera.positionY <= player.positionY && player.positionY <= camera.positionY + 480		//display image on-screen only
				&& camera.positionX <= player.positionX && player.positionX <= camera.positionX + 640) {
				animDraw(player_Walking_Left, player.positionX - camera.positionX, player.positionY - camera.positionY, spriteSize[0], spriteSize[1], deltaTime);
			}
		}
		if (spriteTex_Current == Enemy_Right)
		{
			if (camera.positionY <= player.positionY && player.positionY <= camera.positionY + 480		//display image on-screen only
				&& camera.positionX <= player.positionX && player.positionX <= camera.positionX + 640) {
				animDraw(player_Walking_Right, player.positionX - camera.positionX, player.positionY - camera.positionY, spriteSize[0], spriteSize[1], deltaTime);
			}
		}

		if (Testing_C) {

		}
		//Camera drawing (Layer 3)

		if (camera.positionY <= player.positionY && player.positionY <= camera.positionY + 480		//display image on-screen only
			&& camera.positionX <= player.positionX && player.positionX <= camera.positionX + 640) {
			//printf("\n !!!ProjectileArrayIndex: %d", ProjectileArrayIndex);
			if (sprite1_Alive) {
				glDrawSprite(Enemy_Left, EnemyOne.positionX - camera.positionX, EnemyOne.positionY - camera.positionY, spriteSize[0], spriteSize[1]);
				//printf("\nCurrent player.positionX: %f", player.positionX);
			}
			if (sprite2_Alive) {
				glDrawSprite(Enemy_Right, EnemyTwo.positionX - camera.positionX, EnemyTwo.positionY - camera.positionY, spriteSize[0], spriteSize[1]);
			}
		}
		//glDrawSprite(projectile_image, player.positionX + 5 - camera.positionX, player.positionY + 15 - camera.positionY, projectileSize[0], projectileSize[1]);

		//Weapon drawing (Layer 3)

		//Effect drawing (Layer 3)
		for (int i = 0; i < DrawProjectiles.size(); i++) {
			if (camera.positionY <= DrawProjectiles[i].posY && DrawProjectiles[i].posY <= camera.positionY + 480		//display image on-screen only
				&& camera.positionX <= DrawProjectiles[i].posX && DrawProjectiles[i].posX <= camera.positionX + 640) {
				//printf("\n !!!ProjectileArrayIndex: %d", ProjectileArrayIndex);
				glDrawSprite(projectile_image, DrawProjectiles[i].posX + 5 - camera.positionX, DrawProjectiles[i].posY + 15 - camera.positionY, projectileSize[0], projectileSize[1]);
			}
		}

		
		//Water drawing (Layer 4)

		//UI drawing (Layer 5)

		SDL_GL_SwapWindow(window);
	}

	SDL_Quit();

	return 0;
}
