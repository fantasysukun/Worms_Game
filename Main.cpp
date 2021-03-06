﻿/*********************************************************************
Worms Game
Version - May 18, 2016
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
float Destroyable_Background_PositionX[1600];
float Destroyable_Background_PositionY[1600];

//player initialize (Layer 3)
GLuint player_Walking_Left[3];
GLuint player_Walking_Right[3];
GLuint player_Jumping[3];	// Added 1 more sprite here to make it work for the jumping animation - Kevin Lai

//Enemy initialize (Layer 3)
GLuint Enemy_Right;
GLuint Enemy_Left;

//Effect initialize (Layer 3)
GLuint projectile_image;

//Numbers initialize
GLuint Numbers_Image[10];

//Alphabet initialize
GLuint Alphabet_Image[52];
int Alphabet_Size[2];

//Other initialize 
GLuint NewSprint;
GLuint spriteTex_Current;
GLuint BackGround_bottom;
GLuint BackGround[20];

GLuint Character_Current_Image;
/* size of the sprite */
int spriteSize[2];
int BackgroundSize[2];
int NewSprintSize[2];
int projectileSize[2];

// Destroyable Background Byte Array - Kevin Lai, 5/11/2016
bool* destroyBackground = new bool[1600];

// Sprites Byte Arrays - Change the size later when there are more sprites - Kevin Lai, 5/11/2016
bool* characterBytes = new bool[4];

// Explosion Byte Arrays - Kevin Lai
bool* explosion;

// Turn time - Kevin Lai, 5/11/2016
float setTime = 10000; //changed it to 10 seconds just for faster testing
float defaultTime = setTime;
float displayTimer = 2000;
float delayTimer = displayTimer;

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
char hasJumped = 0;
int jumpTime = 0;
int setJumpTime = 250;

//Global Turn Timer
GLuint TurnTimer[2];

//PlayersTurn & CharactersTurn
int Players_Turn = 0;
int Characters_Turn = 0;

unsigned char** localbytes = new unsigned char*[1600];

int TitleNumberOnScreen[1600];
int TitleNumberOnScreen_Count = 0;

int Character_Image_Size[2];

int Current_Character_number;

int animeDirOne = 0;
int animeDirTwo = 0;

typedef struct Camera
{
	float positionX = 216.0f;
	float positionY = 416.0f;
}Camera;
Camera camera;

typedef struct Character
{
	char name[16];
	float posX;
	float posY;
	bool* CollisionDetectionArray; //set the getbit for this array

	GLuint Character_Image_Left;
	GLuint Character_Image_Right;

	//HP setup
	int HP = 100;
	GLuint HP_Image[3];

	int const static Character_Frame_Size = 3;
	GLuint Character_Left[Character_Frame_Size];
	GLuint Character_Right[Character_Frame_Size];
	GLuint Character_Current[Character_Frame_Size];

	bool isDead;
	bool hasJumped;

}Character;
Character character;
Character Current_2;
Character Current_Character;

//modified player to have an array of 4 characters - Marvin
typedef struct Player
{
	float positionX = 526.0f;
	float positionY = 451.0f;

	int Character_Number[4];
	Character characters[4];
	bool allCharDead = false;
	GLuint Character_Current_Image;

}Player;
Player player;
Player playerOne, playerTwo;
Player default1, default2;
Player Current_Player;



//Enemy Position setup
typedef struct Enemy
{
	float positionX;
	float positionY;
	int WalkingPosition;

}Enemy;
Enemy EnemyOne, EnemyTwo;




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

typedef struct Explosion
{
	GLuint Explosion_image;
	float x;
	float y;
	float h;
	float w;

}Explosion;

Explosion missile;



//Sprite update and display function. This function need to be change to two spile function. 
//one for update with delta time, one for drawing.
void animDraw(GLuint textures[], float x, float y, float w, float h, float deltaTime, int Frame_Size)
{

	secsUntilNextFrame -= deltaTime;
	//printf("secsUntilNextFrame: %f\n", secsUntilNextFrame);
	if (secsUntilNextFrame <= 0.0f) {

		secsUntilNextFrame += ResetFramTime;
		if (curFrame < Frame_Size - 1){
			curFrame++;
		}
		else {
			curFrame = 0;
		}
	}
	glDrawSprite(textures[curFrame], x, y, w, h);
}

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
bool pixelPerfect(bool* sprite1, bool* sprite2, float x, float y, float w, float h, float x2, float y2, float w2, float h2){

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
	bool* object1 = sprite1;
	bool* object2 = sprite2;

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

unsigned char* Getbytes(const char* filename, int* outWidth, int* outHeight)
{
	const int BPP = 4;
	/* open the file */
	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
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

	/* read in data */
	if (bitCount == 32) {
		int it;
		for (it = 0; it != imageWidth * imageHeight; ++it) {
			bytes[it * BPP + 0] = fgetc(file);
			bytes[it * BPP + 1] = fgetc(file);
			bytes[it * BPP + 2] = fgetc(file);
			bytes[it * BPP + 3] = fgetc(file);
		}
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
	return bytes;
}

// Sets specific bits to zero to make them transparent. Then, saves the result as a new GLuint.
GLuint setTransparent(unsigned char* bytes, int imageWidth, int imageHeight)
{
	const int BPP = 4;

	/* load into OpenGL */
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
		GL_BGRA, GL_UNSIGNED_BYTE, bytes);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	free(bytes);

	return tex;
}


bool pixelPerfectDetection(bool a, bool b){
	if (a && b){
		return true;
	}
	return false;
}

/*
Added pixel perfect detection for DB only
- Kevin Lai
*/
void pixelPerfectDB(bool* sprite1, bool* sprite2, float x, float y, float w, float h, float x2, float y2, float w2, float h2){

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
	bool* object1 = sprite1;
	bool* object2 = sprite2;

	bool hit = false;

	float offsetX = intersectionX - x, offsetX2 = intersectionX - x2, offsetY = intersectionY - y, offsetY2 = intersectionY - y2;

	for (int i = 0; i < intersectionH; ++i){

		float tempY = i + offsetY;
		float tempY2 = i + offsetY2;

		for (int j = 0; j < intersectionW; ++j){

			float tempX = j + offsetX;
			float tempX2 = j + offsetX2;

			/*
			Alpha array of each object is used in pixel perfect collision detection.
			If the alpha's of both objects are 1, then there is a collision
			*/
			bool a = object1[(int)(w * tempY + tempX)];
			bool b = object2[(int)(w2 * tempY2 + tempX2)];

			/*
			if (pixelPerfectDetection(a, b)){
			localbytes[(int)((w*i + j) * 4 + 3)] = 0;
			Destroyable_BackGround[i*j] = setTransparent(localbytes[(int)((w*i + j )* 4 + 3)], Destroyable_BackGround_Size[0], Destroyable_BackGround_Size[1]);
			}
			*/
		}
	}

}

/*
void updatePlayerPos(float PlayerPosX, float PlayerPosY) {
player.positionX = PlayerPosX;
player.positionY = PlayerPosY;
}
*/
//This was from the top, but I moved it to the bottom. To fix the reference problem I had to switch Player players to the int 
void updatePlayerPos(int players, int Character_Number, float x, float y)
{
	if (players == 0) {
		playerOne.characters[Character_Number].posX = x;
		playerOne.characters[Character_Number].posY = y;
	}
	else {
		playerTwo.characters[Character_Number].posX = x;
		playerTwo.characters[Character_Number].posY = y;
	}

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
	//updatePlayerPos(ObjectAPosX, ObjectAPosY);
	updatePlayerPos(Players_Turn, Current_Character_number, ObjectAPosX, ObjectAPosY);
}

//Set the camera to middle
void SetTheCameraToMiddle(char angle[]) {

	if (angle == "LEFT") {
		if (camera.positionX + Character_Image_Size[0] > 0) {
			if (Current_Character.posX < 1440 - 320 - Character_Image_Size[0] / 2) { camera.positionX -= offset; }
		}
	}
	if (angle == "RIGHT") {
		if (camera.positionX < 1440 - 640 + Character_Image_Size[0]) {
			if (Current_Character.posX > 320 + Character_Image_Size[0] / 2) { camera.positionX += offset; }
		}
	}
	if (angle == "UP") {
		if (camera.positionY + Character_Image_Size[1]> 0) {
			if (Current_Character.posY < 1440 - 240 - Character_Image_Size[1] / 2) { camera.positionY -= offset; }
		}
	}
	if (angle == "DOWN") {
		//if (camera.positionY < 1440 - 480 + Character_Image_Size[1]) {
		if (camera.positionY < 1440 - (1140 - 910) - 480 + Character_Image_Size[1]) {
			if (Current_Character.posY > 240 + Character_Image_Size[1] / 2) { camera.positionY += offset; }
		}
	}
}

void resetCamera(float newPosx, float newPosy)
{

	float LastcamerapositionX = camera.positionX;
	float LastcamerapositionY = camera.positionY;
	float CurrentcamerapositionX = camera.positionX;
	float CurrentcamerapositionY = camera.positionY;
	float TargetPosX = newPosx - 640 / 2 + Character_Image_Size[0] / 2;
	float TargetPosY = newPosy - 480 / 2 + Character_Image_Size[1] / 2;

	while (LastcamerapositionX != TargetPosX)
	{
		LastcamerapositionX = camera.positionX;
		LastcamerapositionY = camera.positionY;
		if (TargetPosX - LastcamerapositionX < 0) {
			SetTheCameraToMiddle("LEFT");
			CurrentcamerapositionX = camera.positionX;
			CurrentcamerapositionY = camera.positionY;
		}
		if (TargetPosX - LastcamerapositionX > 0) {
			SetTheCameraToMiddle("RIGHT");
			CurrentcamerapositionX = camera.positionX;
			CurrentcamerapositionY = camera.positionY;
		}
		if (TargetPosY - LastcamerapositionY < 0) {
			SetTheCameraToMiddle("UP");
			CurrentcamerapositionX = camera.positionX;
			CurrentcamerapositionY = camera.positionY;
		}
		if (TargetPosY - LastcamerapositionY > 0) {
			SetTheCameraToMiddle("DOWN");
			CurrentcamerapositionX = camera.positionX;
			CurrentcamerapositionY = camera.positionY;
		}


		if (LastcamerapositionX == CurrentcamerapositionX && LastcamerapositionY == CurrentcamerapositionY)
		{
			break;
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
			sprintf_s(Static_Background_Name, sizeof Static_Background_Name, "ArtResource/Static_background/StaticBackground (%d).tga", count + 1);
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
			sprintf_s(Destroyable_Background_Name, sizeof Destroyable_Background_Name, "ArtResource/Destroyable_background/Destroyable_Background_%d.tga", count);
			try {
				Destroyable_BackGround[count] = glTexImageTGAFile(Destroyable_Background_Name, &Destroyable_BackGround_Size[0], &Destroyable_BackGround_Size[1]);
				localbytes[count] = Getbytes(Destroyable_Background_Name, &Destroyable_BackGround_Size[0], &Destroyable_BackGround_Size[1]);
				count++;
			}
			catch (exception e) {

			}
		}
	}
}

//Load the character alpha byte arrays - Kevin Lai, 5/11/2016
void LoadCharacter_Bytes() {
	try {
		characterBytes[0] = getBytes("ArtResource/Character/Character_Left1.tga");
		characterBytes[1] = getBytes("ArtResource/Character/Character_Left2.tga");
		characterBytes[2] = getBytes("ArtResource/Character/Character_Right1.tga");
		characterBytes[3] = getBytes("ArtResource/Character/Character_Right2.tga");
	}
	catch (exception e) {

	}
}

// Load explosion alpha byte array - Kevin Lai
void LoadExplosion_Bytes(){
	try{
		explosion = getBytes("ArtResource/Character/Character_Left1.tga");
	}
	catch (exception e) {

	}
}

//Load the whole Destroyable background alpha byte array - Kevin Lai, 5/11/2016
void LoadDestroyable_Background_Bytes() {
	int count = 0;

	for (int y = 0; y < 40; y++)
	{
		for (int x = 0; x < 40; x++)
		{
			sprintf_s(Destroyable_Background_Name, sizeof Destroyable_Background_Name, "ArtResource/Destroyable_background/Destroyable_Background_%d.tga", count);
			try {
				destroyBackground[count] = getBytes(Destroyable_Background_Name);
				count++;
			}
			catch (exception e) {

			}
		}
	}
}

//Loading numberes
void Load_Numbers()
{
	char Numbers_Name[50];
	int Numbers_Size[2];
	for (int i = 0; i < 10; i++)
	{
		sprintf_s(Numbers_Name, sizeof Numbers_Name, "ArtResource/Numbers/%d.tga", i);
		try {
			Numbers_Image[i] = glTexImageTGAFile(Numbers_Name, &Numbers_Size[0], &Numbers_Size[1]);
		}
		catch (exception e) {

		}
	}
}


//Loading Alphabet
void Load_Alphabet()
{

	char Alphabet_Name[50];
	for (int i = 0; i < 26; i++)
	{
		sprintf_s(Alphabet_Name, sizeof Alphabet_Name, "ArtResource/Alphabet/%c.tga", 'a' + i);
		try {
			Alphabet_Image[i] = glTexImageTGAFile(Alphabet_Name, &Alphabet_Size[0], &Alphabet_Size[1]);
		}
		catch (exception e) {

		}
	}
	for (int i = 0; i < 26; i++)
	{
		sprintf_s(Alphabet_Name, sizeof Alphabet_Name, "ArtResource/Alphabet/%c.tga", 'A' + i);
		try {
			Alphabet_Image[i + 26] = glTexImageTGAFile(Alphabet_Name, &Alphabet_Size[0], &Alphabet_Size[1]);
		}
		catch (exception e) {

		}
	}
}

//Load & update characters' HP image - Had to add both player one and player two in order to show health
void player_characters_HP_Image_initializationAndupdate(Player players, int Character_Number)
{
	try
	{
		playerOne.characters[Character_Number].HP_Image[0] = Numbers_Image[playerOne.characters[Character_Number].HP / 100];
		playerOne.characters[Character_Number].HP_Image[1] = Numbers_Image[(playerOne.characters[Character_Number].HP / 10) % 10];
		playerOne.characters[Character_Number].HP_Image[2] = Numbers_Image[playerOne.characters[Character_Number].HP % 10];

		playerTwo.characters[Character_Number].HP_Image[0] = Numbers_Image[playerTwo.characters[Character_Number].HP / 100];
		playerTwo.characters[Character_Number].HP_Image[1] = Numbers_Image[(playerTwo.characters[Character_Number].HP / 10) % 10];
		playerTwo.characters[Character_Number].HP_Image[2] = Numbers_Image[playerTwo.characters[Character_Number].HP % 10];

		//printf("\ncharacters.HP: %d%d%d", playerOne.characters[Character_Number].HP / 100, (playerOne.characters[Character_Number].HP / 10) % 10, playerOne.characters[Character_Number].HP % 10);
	}
	catch (exception e) {}
}

void Load_UI()
{
	GLuint Turn_Timer;
	int Turn_Timer_Size[2];
	GLuint Final_Timer;
	int Final_Timer_Size[2];
	GLuint Team_HP_Background;
	int Team_HP_Background_Size[2];
	GLuint Team_HP;
	int Team_HP_Size[2];

	GLuint PlayerOne_Vitory_image;
	int PlayerOne_Vitory_image_Size[2];
	GLuint PlayerTwo_Vitory_image;
	int PlayerTwo_Vitory_image_Size[2];

	try {
		Turn_Timer = glTexImageTGAFile("ArtResource/UI/Turn_Timer.tga", &Turn_Timer_Size[0], &Turn_Timer_Size[1]);
		Final_Timer = glTexImageTGAFile("ArtResource/UI/Final_Timer.tga", &Final_Timer_Size[0], &Final_Timer_Size[1]);
		Team_HP_Background = glTexImageTGAFile("ArtResource/UI/Team_HP_Background.tga", &Team_HP_Background_Size[0], &Team_HP_Background_Size[1]);
		Team_HP = glTexImageTGAFile("ArtResource/UI/Team_HP.tga", &Team_HP_Size[0], &Team_HP_Size[1]);
		PlayerOne_Vitory_image = glTexImageTGAFile("ArtResource/UI/PlayerOne_Vitory_image.tga", &PlayerOne_Vitory_image_Size[0], &PlayerOne_Vitory_image_Size[1]);
		PlayerTwo_Vitory_image = glTexImageTGAFile("ArtResource/UI/PlayerTwo_Vitory_image.tga", &PlayerTwo_Vitory_image_Size[0], &PlayerTwo_Vitory_image_Size[1]);
	}
	catch (exception e) {

	}

}

void Player_Turn_Timer_UI(int timeLeft) {
	int temp = timeLeft;
	TurnTimer[0] = Numbers_Image[timeLeft / 10000 % 10];
	TurnTimer[1] = Numbers_Image[timeLeft / 1000 % 10];

}

void GetTitleNumberOnScreen()
{
	for (int y = 0; y < 40; y++)
	{
		for (int x = 0; x < 40; x++)
		{
			int tileNum = getTile(x, y);
			if (camera.positionY / 36 <= y + 11 && y <= camera.positionY / 36 + 15		//display image on-screen only
				&& camera.positionX / 36 <= x + 1 && x <= camera.positionX / 36 + 19) {// 
				if (Destroyable_BackGround[getDestroyableBackground(x, y)] != NULL) {
					try {
						TitleNumberOnScreen[TitleNumberOnScreen_Count] = getDestroyableBackground(x, y);
						TitleNumberOnScreen_Count++;
					}
					catch (exception e) {
					}
				}
			}
		}
	}

}

//Returns Current Player
Player getCurrentPlayer(){
	if (Players_Turn == 0) {
		return playerOne;

	}
	else {
		return playerTwo;
	}

}
//Returns Current Character
Character getCurrentCharacter(int Players_Turn){
	if (Players_Turn == 0) {
		return playerOne.characters[Current_Character_number];
	}
	else {
		return playerTwo.characters[Current_Character_number];
	}

}

//This physically changes playerOne and PlayerTwo's Health - this fixes the reference problem
void setCurrentPlayerHP(int currentPlayer, int currentChar, int damage) {
	if (currentPlayer == 0) {
		playerOne.characters[currentChar].HP = damage;
	}
	if (currentPlayer == 1) {
		playerTwo.characters[currentChar].HP = damage;
	}

}


//Keeps track of whose turn it is based on player and character 
void Player_Character_Turn(int currentPlayer, int currentCharacter) {
	if (currentPlayer == 0) {
		Players_Turn = 1;
	}
	else {
		Players_Turn = 0;
		//Characters_Turn++;
		if (currentCharacter == 3) {
			Characters_Turn = 0;
		}
	}

}

/*
* Turn Timer Counter for each player's turn
* Call this in the update for each frame - Kevin Lai, 5/11/2016
*/
void turnTimer(float deltaTimeCount){
	defaultTime -= deltaTimeCount;
	delayTimer -= deltaTimeCount;
	if (defaultTime <= 0){
		defaultTime += setTime;
		delayTimer = displayTimer;
		Player_Character_Turn(Players_Turn, Characters_Turn);
		Current_Player = getCurrentPlayer();
		Current_Character = getCurrentCharacter(Players_Turn);
		resetCamera(Current_Character.posX, Current_Character.posY);
	}

}

/*
* Jump Timer
* Call this in the update for each frame - Kevin Lai, 5/11/2016
*/
void jumpTimer(float deltaTimeCount){
	if (jumpTime > 0){
		jumpTime -= deltaTimeCount;
	}
	else{
		jumpTime = 0;
	}
}



//Right Now this only checked character 1, for the other characters just add && 2.isDead == true && 3.isDead == true, etc
void checkDeaths() {
	for (int i = 0; i < 4; i++) {
		if (playerOne.characters[i].HP <= 0) {
			playerOne.characters[i].isDead = true;
		}
		if (playerTwo.characters[i].HP <= 0) {
			playerTwo.characters[i].isDead = true;
		}
	}
	if (playerOne.characters[0].isDead == true) {
		playerOne.allCharDead = true;
	}
	if (playerTwo.characters[0].isDead == true) {
		playerTwo.allCharDead = true;
	}
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

	for (int i = 0; i < 4; i++)
	{
		playerOne.Character_Number[i] = i;
		playerTwo.Character_Number[i] = i;
		playerOne.characters[i].posX = 426.0f;
		playerOne.characters[i].posY = 651.0f;
		playerTwo.characters[i].posX = 726.0f;
		playerTwo.characters[i].posY = 651.0f;
	}

	/* Load the texture */
	//Enemy_Right = glTexImageTGAFile("ArtResource/Enemy_Right.tga", &spriteSize[0], &spriteSize[1]);
	//Enemy_Left = glTexImageTGAFile("ArtResource/Enemy_Left.tga", &spriteSize[0], &spriteSize[1]);




	int Character_Image_Size[2];
	character.Character_Left[0] = glTexImageTGAFile("ArtResource/Character/Character_Left1.tga", &Character_Image_Size[0], &Character_Image_Size[1]);
	character.Character_Left[1] = glTexImageTGAFile("ArtResource/Character/Character_Left2.tga", &Character_Image_Size[0], &Character_Image_Size[1]);
	character.Character_Left[2] = glTexImageTGAFile("ArtResource/Character/Character_Left3.tga", &Character_Image_Size[0], &Character_Image_Size[1]);

	character.Character_Right[0] = glTexImageTGAFile("ArtResource/Character/Character_Right1.tga", &Character_Image_Size[0], &Character_Image_Size[1]);
	character.Character_Right[1] = glTexImageTGAFile("ArtResource/Character/Character_Right2.tga", &Character_Image_Size[0], &Character_Image_Size[1]);
	character.Character_Right[2] = glTexImageTGAFile("ArtResource/Character/Character_Right3.tga", &Character_Image_Size[0], &Character_Image_Size[1]);

	character.Character_Image_Left = glTexImageTGAFile("ArtResource/Character/Character_Left1.tga", &Character_Image_Size[0], &Character_Image_Size[1]);
	character.Character_Image_Right = glTexImageTGAFile("ArtResource/Character/Character_Right1.tga", &Character_Image_Size[0], &Character_Image_Size[1]);
	Character_Current_Image = character.Character_Image_Left;
	//playerOne.Character_Current_Image = Character_Current_Image;
	//playerTwo.Character_Current_Image = Character_Current_Image;

	/*
	//BackGround_bottom = glTexImageTGAFile("Background_bottom.tga", NULL, NULL);
	BackGround[0] = glTexImageTGAFile("ArtResource/13.tga", NULL, NULL);
	BackGround[1] = glTexImageTGAFile("ArtResource/14.tga", NULL, NULL);
	BackGround[2] = glTexImageTGAFile("ArtResource/4.tga", NULL, NULL);
	BackGround[3] = glTexImageTGAFile("ArtResource/Static_Background1.tga", NULL, NULL);
	*/
	player_Walking_Left[0] = glTexImageTGAFile("ArtResource/Left.tga", NULL, NULL);
	player_Walking_Left[1] = glTexImageTGAFile("ArtResource/Left_2.tga", NULL, NULL);
	player_Walking_Right[0] = glTexImageTGAFile("ArtResource/Right.tga", NULL, NULL);
	player_Walking_Right[1] = glTexImageTGAFile("ArtResource/Right_2.tga", NULL, NULL);

	// Added 1 more sprite to the jumping animation - Kevin Lai
	player_Jumping[0] = glTexImageTGAFile("ArtResource/Character/Character_Jump1.tga", NULL, NULL);
	player_Jumping[1] = glTexImageTGAFile("ArtResource/Character/Character_Jump2.tga", NULL, NULL);
	player_Jumping[2] = glTexImageTGAFile("ArtResource/Character/Character_Jump3.tga", NULL, NULL);

	int Character_Left_Size[2];

	//This is the real left and right animation for playerOne and playerTwo
	for (int i = 0; i < 4; i++) {
		playerOne.characters[i].Character_Left[0] = glTexImageTGAFile("ArtResource/Character/Character_Left1.tga", &Character_Left_Size[0], &Character_Left_Size[1]);
		playerTwo.characters[i].Character_Left[0] = glTexImageTGAFile("ArtResource/Character/Character_Left1.tga", &Character_Left_Size[0], &Character_Left_Size[1]);
		playerOne.characters[i].Character_Right[0] = glTexImageTGAFile("ArtResource/Character/Character_Right1.tga", &Character_Left_Size[0], &Character_Left_Size[1]);
		playerTwo.characters[i].Character_Right[0] = glTexImageTGAFile("ArtResource/Character/Character_Right1.tga", &Character_Left_Size[0], &Character_Left_Size[1]);

	}
	character.Character_Left[0] = glTexImageTGAFile("ArtResource/Character/Character_Left1.tga", &Character_Left_Size[0], &Character_Left_Size[1]);
	character.Character_Left[1] = glTexImageTGAFile("ArtResource/Character/Character_Left2.tga", &Character_Left_Size[0], &Character_Left_Size[1]);
	character.Character_Left[2] = glTexImageTGAFile("ArtResource/Character/Character_Left3.tga", NULL, NULL);

	try
	{
		int Character_Right_Size[2];
		character.Character_Right[0] = glTexImageTGAFile("ArtResource/Character/Character_Right1.tga", &Character_Right_Size[0], &Character_Right_Size[1]);
		character.Character_Right[1] = glTexImageTGAFile("ArtResource/Character/Character_Right2.tga", &Character_Right_Size[0], &Character_Right_Size[1]);
		character.Character_Right[2] = glTexImageTGAFile("ArtResource/Character/Character_Right3.tga", NULL, NULL);
	}
	catch (exception e) {}

	GLuint DeadthAnimation[2];
	int DeadthAnimation_Size[2];
	DeadthAnimation[0] = glTexImageTGAFile("ArtResource/Character/Deadv1.tga", &DeadthAnimation_Size[0], &DeadthAnimation_Size[1]);
	DeadthAnimation[1] = glTexImageTGAFile("ArtResource/Character/Deadv2.tga", &DeadthAnimation_Size[0], &DeadthAnimation_Size[1]);

	bool Testing_SB = true;
	GLuint Testing_Static_Background;
	int Testing_Static_Background_size[2];
	try {
		Testing_Static_Background = glTexImageTGAFile("ArtResource/Static_background/static_background.tga", &Testing_Static_Background_size[0], &Testing_Static_Background_size[1]);
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

	//Loading Numberes
	Load_Numbers();

	//Loading Alphabet
	Load_Alphabet();

	//aracters HP image initialization;
	try
	{
		for (int i = 0; i < 4; i++)
		{
			player_characters_HP_Image_initializationAndupdate(playerOne, playerOne.Character_Number[i]);
			player_characters_HP_Image_initializationAndupdate(playerTwo, playerTwo.Character_Number[i]);
		}
	}
	catch (exception e) {

	}

	//Load UI Layer
	Load_UI();

	//Loading for static background
	// LoadStatic_Background();

	//Loading for destroyable background
	LoadDestroyable_Background();

	// Loading the byte array for each character sprite - Kevin Lai, 5/11/2016
	LoadCharacter_Bytes();

	//Loading the byte array for each tile in destroyable background - Kevin Lai, 5/11/2016
	LoadDestroyable_Background_Bytes();

	// LoadExplosion Byte array - Kevin Lai
	LoadExplosion_Bytes();

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
	//EnemyOne.WalkingPosition = 0;
	//EnemyTwo.WalkingPosition = 2;

	//Enemt WalkPathPosition setup
	//WalkPathPositionOne.positionX = 200.0f;
	//WalkPathPositionOne.positionY = 200.0f;
	//WalkPathPositionTwo.positionX = 200.0f;
	//WalkPathPositionTwo.positionY = 800.0f;
	//WalkPathPositionThree.positionX = 800.0f;
	//WalkPathPositionThree.positionY = 800.0f;
	//WalkPathPositionFour.positionX = 800.0f;
	//WalkPathPositionFour.positionY = 200.0f;
	//WalkPathPosition WalkPathPositionArray[] = { WalkPathPositionOne, WalkPathPositionTwo, WalkPathPositionThree, WalkPathPositionFour };

	////Enemy Position setup
	//EnemyOne.positionX = 688.0f;
	//EnemyOne.positionY = 474.0f;
	//EnemyTwo.positionX = 426.0f;
	//EnemyTwo.positionY = 351.0f;

	//Game logic setup
	Current_Player = getCurrentPlayer();
	Current_Character = getCurrentCharacter(Players_Turn);

	default1 = playerOne;
	default2 = playerTwo;

	/* The game loop */
	while (!shouldExit) {
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
		if (kbState[SDL_SCANCODE_R]) {
			playerOne = default1;
			playerTwo = default2;
			defaultTime = setTime;
			delayTimer = displayTimer;
		}
		if (playerOne.allCharDead == false && playerTwo.allCharDead == false) {

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

			//Handles the Turn UI
			turnTimer(deltaTime);
			Player_Turn_Timer_UI(defaultTime);

			Current_Character_number = Characters_Turn;

			//This solves the HP not Changing everytime it gets hit
			Current_Character = getCurrentCharacter(Players_Turn);

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
					projectile.posX = Current_Character.posX;
					projectile.posY = Current_Character.posY;

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
					projectile.posX = Current_Character.posX + projectile.velocityX * (Character_Image_Size[0] / 2 + projectileSize[0] / 2);
					projectile.posY = Current_Character.posY + projectile.velocityY * (Character_Image_Size[1] / 2 + projectileSize[1] / 2);
					DrawProjectiles.push_back(projectile);


				}
				//printf("\n\n RUNnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn: %d", SDL_GetTicks());
			}

			////Enemy Update
			//if (sprite1_Alive) {

			//	float RandomMovingSpeed = rand() / (float)RAND_MAX / 10;
			//	if (EnemyOne.positionX < WalkPathPositionArray[EnemyOne.WalkingPosition].positionX) {
			//		EnemyOne.positionX += RandomMovingSpeed;
			//		CurrentMovementNumber++;
			//	}
			//	else if (EnemyOne.positionX > WalkPathPositionArray[EnemyOne.WalkingPosition].positionX) {
			//		EnemyOne.positionX -= RandomMovingSpeed;
			//		CurrentMovementNumber++;
			//	}
			//	if (EnemyOne.positionY < WalkPathPositionArray[EnemyOne.WalkingPosition].positionY) {
			//		EnemyOne.positionY += RandomMovingSpeed;
			//		CurrentMovementNumber++;
			//	}
			//	else if (EnemyOne.positionY > WalkPathPositionArray[EnemyOne.WalkingPosition].positionY) {
			//		EnemyOne.positionY -= RandomMovingSpeed;
			//		CurrentMovementNumber++;
			//	}

			//	if (CurrentMovementNumber == 13000) {
			//		EnemyOne.WalkingPosition += 1;
			//		CurrentMovementNumber = 0;
			//		if (EnemyOne.WalkingPosition == 4) { EnemyOne.WalkingPosition = 0; }
			//	}

			//	float RandomShootingSpeed = rand() / (float)RAND_MAX;
			//	//printf("\nRandomShootingSpeed : %f", RandomShootingSpeed);
			//	if (RandomShootingSpeed < 0.005f) {
			//		if (projectilesVector.size() > 0)
			//		{

			//			Projectile projectile = projectilesVector.back();
			//			projectile.posX = EnemyOne.positionX;
			//			projectile.posY = EnemyOne.positionY;

			//			projectile.SpawnTime = curFrameNS;
			//			projectilesVector.pop_back();
			//			float slopeFromEnemyOneToPlayer = std::abs(std::atan((player.positionY - projectile.posY) / (player.positionX - projectile.posX)));
			//			projectile.velocityX = projectile.speed * std::cos(slopeFromEnemyOneToPlayer);
			//			projectile.velocityY = projectile.speed * std::sin(slopeFromEnemyOneToPlayer);
			//			if (player.positionX - projectile.posX < 0)
			//			{
			//				projectile.velocityX *= -1;
			//			}

			//			if (player.positionY - projectile.posY < 0)
			//			{
			//				projectile.velocityY *= -1;
			//			}
			//			projectile.posX = EnemyOne.positionX + projectile.velocityX * (spriteSize[0] / 2 + projectileSize[0] / 2);
			//			projectile.posY = EnemyOne.positionY + projectile.velocityY * (spriteSize[1] / 2 + projectileSize[1] / 2);

			//			DrawProjectiles.push_back(projectile);

			//		}
			//	}
			//}

			// Physics update
			do {
				// 1. Physics movement
				//printf("\n\n !!!curFrameMs: %d", curFrameNS);


				//playerOne.characters[0].posY +=  gravity;
				//playerTwo.characters[0].posY +=  gravity;


				/*
					Jump time is based on deltaTime now. - Kevin Lai
				*/
				jumpTimer(deltaTime);

				/*
					Added checks for player turns. - Kevin Lai
				*/
				if (Current_Character.hasJumped == 1 && jumpTime <= 0) {
					Current_Character.hasJumped = 0;

					if (Players_Turn == 0){
						playerOne.characters[0].hasJumped = 0;
					}
					else{
						playerTwo.characters[0].hasJumped = 0;
					}

				}
				else if (Current_Character.hasJumped == 1){


					//Someone will need to change this to current_player.current_character.positionX later - Kevin Lai
					//This is basically the speed of the jump. Default at the moment is 4 * speed_of_gravity.

					if (Players_Turn == 0){
						playerOne.characters[0].posY -= 2 * gravity;
					}
					else{
						playerTwo.characters[0].posY -= 2 * gravity;
					}

				}


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

				/*
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
				*/
				/*
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
				*/

				for (int i = 0; i < DrawProjectiles.size(); i++)
				{
					if (Players_Turn == 0)
					{
						if (AABB(DrawProjectiles[i].posX, DrawProjectiles[i].posY, projectileSize[0], projectileSize[1], playerTwo.characters[0].posX, playerTwo.characters[0].posY, Character_Image_Size[0], Character_Image_Size[1])) //Character_Left_Size[0], Character_Left_Size[1] nned to be change later
						{
							projectilesVector.push_back(DrawProjectiles[i]);
							DrawProjectiles.erase(DrawProjectiles.begin() + i);

							//This decrements the HP and uses the setHP to directly change the health of either playerOne or Two's characters
							playerTwo.characters[0].HP -= 1;
							printf("Current Character's HP: %d \n", playerTwo.characters[0].HP);
							setCurrentPlayerHP(Players_Turn + 1, Current_Character_number, playerTwo.characters[0].HP);
							player_characters_HP_Image_initializationAndupdate(playerTwo, 0);
						}
					}
					if (Players_Turn == 1)
					{
						if (AABB(DrawProjectiles[i].posX, DrawProjectiles[i].posY, projectileSize[0], projectileSize[1], playerOne.characters[0].posX, playerOne.characters[0].posY, Character_Image_Size[0], Character_Image_Size[1])) //Character_Left_Size[0], Character_Left_Size[1] nned to be change later
						{
							projectilesVector.push_back(DrawProjectiles[i]);
							DrawProjectiles.erase(DrawProjectiles.begin() + i);

							//This decrements the HP and uses the setHP to directly change the health of either playerOne or Two's characters
							playerOne.characters[0].HP -= 1;
							printf("Current Character's HP: %d \n", playerOne.characters[0].HP);
							setCurrentPlayerHP(Players_Turn + 1, Current_Character_number, playerOne.characters[0].HP);
							player_characters_HP_Image_initializationAndupdate(playerOne, 0);
						}
					}
				}

				// 3. Physics collision resolution
				//if (sprite1_Alive) {

				//	//CollisionResolution(player.positionX, player.positionY, spriteSize[0], spriteSize[1], EnemyOne.positionX, EnemyOne.positionY, spriteSize[0], spriteSize[1]);

				//	if (AABB(player.positionX, player.positionY, spriteSize[0], spriteSize[1], EnemyOne.positionX, EnemyOne.positionY, spriteSize[0], spriteSize[1]))
				//	{
				//		if (kbState[SDL_SCANCODE_LEFT]) {
				//			if (player.positionX > EnemyOne.positionX && player.positionX + spriteSize[0] > EnemyOne.positionX + spriteSize[0]) {
				//				player.positionX += EnemyOne.positionX + spriteSize[0] - player.positionX;
				//				//camera.positionX += offset;
				//				SetTheCameraToMiddle("RIGHT");
				//			}
				//		}
				//		if (kbState[SDL_SCANCODE_RIGHT]) {
				//			if (player.positionX < EnemyOne.positionX && player.positionX + spriteSize[0] > EnemyOne.positionX && player.positionX + spriteSize[0] < EnemyOne.positionX + spriteSize[0]) {
				//				player.positionX -= player.positionX + spriteSize[0] - EnemyOne.positionX;
				//				printf("\nplayer.positionX: %f", player.positionX);
				//				//camera.positionX -= offset;
				//				SetTheCameraToMiddle("LEFT");
				//			}
				//		}
				//		if (kbState[SDL_SCANCODE_UP]) {
				//			if (player.positionY > EnemyOne.positionY && player.positionY + spriteSize[1] > EnemyOne.positionY + spriteSize[1]) {
				//				player.positionY += EnemyOne.positionY + spriteSize[1] - player.positionY;
				//				//camera.positionY += offset;
				//				SetTheCameraToMiddle("DOWN");
				//			}
				//		}
				//		if (kbState[SDL_SCANCODE_DOWN]) {
				//			if (player.positionY < EnemyOne.positionY && player.positionY + spriteSize[1] > EnemyOne.positionY && player.positionY + spriteSize[1] < EnemyOne.positionY + spriteSize[1]) {
				//				player.positionY -= player.positionY + spriteSize[1] - EnemyOne.positionY;
				//				//camera.positionY -= offset;
				//				SetTheCameraToMiddle("UP");
				//			}
				//		}
				//	}

				//}

				//if (sprite2_Alive) {
				//	if (AABB(player.positionX, player.positionY, spriteSize[0], spriteSize[1], 426, 351, spriteSize[0], spriteSize[1]))
				//	{
				//		if (kbState[SDL_SCANCODE_LEFT]) {
				//			if (player.positionX > 426 && player.positionX + spriteSize[0] > 426 + spriteSize[0]) {
				//				player.positionX += 426 + spriteSize[0] - player.positionX;
				//				//camera.positionX += offset;
				//				SetTheCameraToMiddle("RIGHT");
				//			}
				//		}
				//		if (kbState[SDL_SCANCODE_RIGHT]) {
				//			if (player.positionX < 426 && player.positionX + spriteSize[0] > 426 && player.positionX + spriteSize[0] < 426 + spriteSize[0]) {
				//				player.positionX -= player.positionX + spriteSize[0] - 426;
				//				printf("\nplayer.positionX: %f", player.positionX);
				//				//camera.positionX -= offset;
				//				SetTheCameraToMiddle("LEFT");
				//			}
				//		}
				//		if (kbState[SDL_SCANCODE_UP]) {
				//			if (player.positionY > 351 && player.positionY + spriteSize[1] > 351 + spriteSize[1]) {
				//				player.positionY += 351 + spriteSize[1] - player.positionY;
				//				//camera.positionY += offset;
				//				SetTheCameraToMiddle("DOWN");
				//			}
				//		}
				//		if (kbState[SDL_SCANCODE_DOWN]) {
				//			if (player.positionY < 351 && player.positionY + spriteSize[1] > 351 && player.positionY + spriteSize[1] < 351 + spriteSize[1]) {
				//				player.positionY -= player.positionY + spriteSize[1] - 351;
				//				//camera.positionY -= offset;
				//				SetTheCameraToMiddle("UP");
				//			}
				//		}
				//	}
				//}

				//sprite 

				if (playerOne.characters[0].posY + Character_Image_Size[1] > 910) {
					playerOne.characters[0].posY -= playerOne.characters[0].posY + Character_Image_Size[1] - 910;
					CollisionResolution(playerOne.characters[0].posX, playerOne.characters[0].posY, Character_Image_Size[0], Character_Image_Size[1], 0, 910, 1440, 1440 - 910);
				}
				if (playerTwo.characters[0].posY + Character_Image_Size[1] > 910) {
					playerTwo.characters[0].posY -= playerTwo.characters[0].posY + Character_Image_Size[1] - 910;
					CollisionResolution(playerTwo.characters[0].posX, playerTwo.characters[0].posY, Character_Image_Size[0], Character_Image_Size[1], 0, 910, 1440, 1440 - 910);
				}
				//

				//CollisionResolution(player.positionX, player.positionY, spriteSize[0], spriteSize[1], 936, 936, 108, 108);

				/*
				// pixelPerfectDB to set to transparent
				for (int i = 0; i < 1600; i++){
				if (AABB(Destroyable_Background_PositionX[i], Destroyable_Background_PositionY[i], Destroyable_BackGround_Size[0], Destroyable_BackGround_Size[1], missile.x, missile.y, missile.w, missile.h)){
				if (pixelPerfect(&destroyBackground[i], explosion, Destroyable_Background_PositionX[i], Destroyable_Background_PositionY[i], Destroyable_BackGround_Size[0], Destroyable_BackGround_Size[1], missile.x, missile.y, missile.w, missile.h)){
				pixelPerfectDB(&destroyBackground[i], explosion, Destroyable_Background_PositionX[i], Destroyable_Background_PositionY[i], Destroyable_BackGround_Size[0], Destroyable_BackGround_Size[1], missile.x, missile.y, missile.w, missile.h);
				}
				}
				}
				*/

				lastPhysicsFrameMs += physicsDeltaMs;
			} while (lastPhysicsFrameMs + physicsDeltaMs < curFrameNS);




			//printf("lastFrameNS: %d\n", lastFrameNS);
			curFrameNS = SDL_GetTicks();
			//printf("currentFrameNS: %d\n", currentFrameNS);

			/* Game logic */
			if (kbState[SDL_SCANCODE_ESCAPE]) { //SDL_SCANCODE_* get the keyboard state
				shouldExit = 1;
			}
			//Uncomment for movement location
			//printf("\nCurrent_Character.posX: %f urrent_Character.posY: %f", Current_Character.posX, Current_Character.posY);

			if (kbState[SDL_SCANCODE_LEFT]) {
				if (Current_Character.posX > 0 + 1) {
					Current_Character.posX -= offset;
					updatePlayerPos(Players_Turn, Current_Character_number, Current_Character.posX, Current_Character.posY);
					SetTheCameraToMiddle("LEFT");
					if (Players_Turn == 0)
					{
						animeDirOne = 0;
						//playerOne.Character_Current_Image = character.Character_Image_Left;
						//animDraw(character.Character_Left, playerOne.positionX, playerOne.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 3);
					}
					if (Players_Turn == 1)
					{
						animeDirTwo = 0;
						//playerTwo.Character_Current_Image = character.Character_Image_Left;
						animDraw(character.Character_Left, playerTwo.positionX, playerTwo.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 3);
					}
				}
			}
			if (kbState[SDL_SCANCODE_RIGHT]) {
				if (Current_Character.posX < 1440 - Character_Image_Size[0]) {
					Current_Character.posX += offset;
					updatePlayerPos(Players_Turn, Current_Character_number, Current_Character.posX, Current_Character.posY);
					SetTheCameraToMiddle("RIGHT");
					if (Players_Turn == 0)
					{
						animeDirOne = 1;
						//playerOne.Character_Current_Image = character.Character_Image_Right;
					}
					if (Players_Turn == 1)
					{
						animeDirTwo = 1;
						//playerTwo.Character_Current_Image = character.Character_Image_Right;
					}
				}
			}
			if (kbState[SDL_SCANCODE_UP]) {
				if (Current_Character.posY > 0 + 1) {
					Current_Character.posY -= offset;
					updatePlayerPos(Players_Turn, Current_Character_number, Current_Character.posX, Current_Character.posY);
					SetTheCameraToMiddle("UP");

				}
			}
			if (kbState[SDL_SCANCODE_DOWN]) {
				if (Current_Character.posY < 1440 - Character_Image_Size[1]) {
					Current_Character.posY += offset;
					updatePlayerPos(Players_Turn, Current_Character_number, Current_Character.posX, Current_Character.posY);
					SetTheCameraToMiddle("DOWN");
				}
			}
			if (kbState[SDL_SCANCODE_SPACE]) {
				if (Current_Character.posY > 0 + 1) {
					Current_Character.posY -= offset;
					updatePlayerPos(Players_Turn, Current_Character_number, Current_Character.posX, Current_Character.posY);
					SetTheCameraToMiddle("UP");

					/*
						Adjusted the character jump variables to tell which character's turn it is. - Kevin Lai
						And which character should be the one that is jumping.
					*/
					if (Current_Character.hasJumped == 0) {

						Current_Character.hasJumped = 1;

						if (Players_Turn == 0){
							playerOne.characters[0].hasJumped = 1;
						}
						else{
							playerTwo.characters[0].hasJumped = 1;
						}

						/*
						This will determine how high the character will jump up. - Kevin Lai
						Equation is: Distance = jumpTime * jump_speed

						Example: jumpTime = 27, jump_speed = 6 * gravity = 6 * (0.5) = 3
						As a result, Distance = 27 * 3 = 81.
						*/
						jumpTime = setJumpTime;

					}

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

			/*
			Updating the character position after each button press. - Kevin Lai
			Someone will need to modify this based on whose turn it is. So, example: player 2 character 3's position will be updated.
			*/
			//updatePlayerPos(playerOne, playerOne.Character_Number[0], player.positionX, player.positionY);

			// Example: final version should look something like this - Kevin Lai
			// updatePlayerPos(Players_Turn, current_character, current_character_x, current_character_y);

			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);


			//****** Draw BackGronud ******//
			//old background, this background will be replaced once the new background is loaded
			/*
			Current_Player = playerOne;
			Current_Character_number = 0;
			Current_Character = Current_Player.characters[Current_Character_number];
			*/

			//Static background drawing (Layer 1)
			if (camera.positionY <= Current_Character.posY && Current_Character.posY <= camera.positionY + 480		//display image on-screen only
				&& camera.positionX <= Current_Character.posX && Current_Character.posX <= camera.positionX + 640) {
				glDrawSprite(Testing_Static_Background, 0 - camera.positionX, 0 - camera.positionY, Testing_Static_Background_size[0], Testing_Static_Background_size[1]);
			}


			//Destroyable background drawing (Layer 2)

			for (int y = 0; y < 40; y++)
			{
				for (int x = 0; x < 40; x++)
				{
					int tileNum = getTile(x, y);
					if (camera.positionY / 36 <= y + 11 && y <= camera.positionY / 36 + 15		//display image on-screen only
						&& camera.positionX / 36 <= x + 1 && x <= camera.positionX / 36 + 19) {// 

						//bool testing = (Destroyable_BackGround[getDestroyableBackground(x, y)] != NULL);
						//fputs(testing ? "true" : "false", stdout);
						try {
							if (Destroyable_BackGround[getDestroyableBackground(x, y)] != NULL) {

								glDrawSprite(Destroyable_BackGround[getDestroyableBackground(x, y)], 36 * x - camera.positionX, 36 * y - camera.positionY, 36, 36);
								Destroyable_Background_PositionX[x*y] = x;
								Destroyable_Background_PositionY[x*y] = y;
							}
						}
						catch (exception e) {

						}

					}
				}
			}


			/*
			Current_Character.posX = player.positionX;
			Current_Character.posY = player.positionY;
			*/
			//Testing background is for artist to do art testing
			if (true) {
				glDrawSprite(Testing_Static_Background, 0 - camera.positionX, 0 - camera.positionY, Testing_Static_Background_size[0], Testing_Static_Background_size[1]);
				glDrawSprite(Testing_Destroyable_Background, 0 - camera.positionX, 0 - camera.positionY, Testing_Destroyable_Background_size[0], Testing_Destroyable_Background_size[1]);

				/*
				if (camera.positionY <= Current_Character.posY && Current_Character.posY <= camera.positionY + 480		//display image on-screen only
				&& camera.positionX <= Current_Character.posX && Current_Character.posX <= camera.positionX + 640) {
				glDrawSprite(Testing_Static_Background, 0 - camera.positionX, 0 - camera.positionY, Testing_Static_Background_size[0], Testing_Static_Background_size[1]);
				}

				if (camera.positionY <= Current_Character.posY && Current_Character.posY <= camera.positionY + 480		//display image on-screen only
				&& camera.positionX <= Current_Character.posX && Current_Character.posX <= camera.positionX + 640) {
				glDrawSprite(Testing_Destroyable_Background, 0 - camera.positionX, 0 - camera.positionY, Testing_Destroyable_Background_size[0], Testing_Destroyable_Background_size[1]);
				}
				*/
			}

			//Sprite drawing (Layer 3)
			/*
			if (camera.positionY <= Current_Character.posY && Current_Character.posY <= camera.positionY + 480		//display image on-screen only
			&& camera.positionX <= Current_Character.posX && Current_Character.posX <= camera.positionX + 640) {
			*/

			// drew only the first character of Player One and Two, Dont jump because it will revert to the previous code havent made it for jump yet
			if (!playerOne.characters[0].isDead)
			{
				//Jumping animation is mostly fixed. - Kevin Lai
				if (Current_Character.hasJumped == 1 && playerOne.characters[0].hasJumped == 1 && Players_Turn == 0) {
					animDraw(player_Jumping, playerOne.characters[0].posX - camera.positionX, playerOne.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 2);
				}
				else{
					if (animeDirOne == 0) {
						animDraw(character.Character_Left, playerOne.characters[0].posX - camera.positionX, playerOne.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 3);
					}
					if (animeDirOne == 1) {
						animDraw(character.Character_Right, playerOne.characters[0].posX - camera.positionX, playerOne.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 3);
					}
					//glDrawSprite(playerOne.Character_Current_Image, playerOne.characters[0].posX - camera.positionX, playerOne.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1]);
				}
				glDrawSprite(Alphabet_Image[15], playerOne.characters[0].posX - camera.positionX - Alphabet_Size[0] / 2, playerOne.characters[0].posY - camera.positionY - Alphabet_Size[1], Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);
				glDrawSprite(Numbers_Image[1], playerOne.characters[0].posX - camera.positionX, playerOne.characters[0].posY - camera.positionY - Alphabet_Size[1], Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);
				glDrawSprite(playerOne.characters[0].HP_Image[0], playerOne.characters[0].posX - camera.positionX - Alphabet_Size[0] / 2, playerOne.characters[0].posY - camera.positionY - Alphabet_Size[1] / 2, Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);
				glDrawSprite(playerOne.characters[0].HP_Image[1], playerOne.characters[0].posX - camera.positionX, playerOne.characters[0].posY - camera.positionY - Alphabet_Size[1] / 2, Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);
				glDrawSprite(playerOne.characters[0].HP_Image[2], playerOne.characters[0].posX - camera.positionX + Alphabet_Size[0] / 2, playerOne.characters[0].posY - camera.positionY - Alphabet_Size[1] / 2, Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);

			}
			else
			{
				//Art is not working
				//animDraw(DeadthAnimation, playerOne.characters[0].posX - camera.positionX, playerOne.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 2);
			}

			if (!playerTwo.characters[0].isDead)
			{
				//Jumping animation is mostly fixed. - Kevin Lai
				if (Current_Character.hasJumped == 1 && playerTwo.characters[0].hasJumped == 1 && Players_Turn == 1) {
					animDraw(player_Jumping, playerTwo.characters[0].posX - camera.positionX, playerTwo.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 2);
				}
				else{
					if (animeDirTwo == 0) {
						animDraw(character.Character_Left, playerTwo.characters[0].posX - camera.positionX, playerTwo.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 3);
					}
					if (animeDirTwo == 1) {
						animDraw(character.Character_Right, playerTwo.characters[0].posX - camera.positionX, playerTwo.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 3);
					}
				}
				//glDrawSprite(playerTwo.Character_Current_Image, playerTwo.characters[0].posX - camera.positionX, playerTwo.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1]);
				glDrawSprite(Alphabet_Image[15], playerTwo.characters[0].posX - camera.positionX - Alphabet_Size[0] / 2, playerTwo.characters[0].posY - camera.positionY - Alphabet_Size[1], Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);
				glDrawSprite(Numbers_Image[2], playerTwo.characters[0].posX - camera.positionX, playerTwo.characters[0].posY - camera.positionY - Alphabet_Size[1], Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);
				glDrawSprite(playerTwo.characters[0].HP_Image[0], playerTwo.characters[0].posX - camera.positionX - Alphabet_Size[0] / 2, playerTwo.characters[0].posY - camera.positionY - Alphabet_Size[1] / 2, Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);
				glDrawSprite(playerTwo.characters[0].HP_Image[1], playerTwo.characters[0].posX - camera.positionX, playerTwo.characters[0].posY - camera.positionY - Alphabet_Size[1] / 2, Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);
				glDrawSprite(playerTwo.characters[0].HP_Image[2], playerTwo.characters[0].posX - camera.positionX + Alphabet_Size[0] / 2, playerTwo.characters[0].posY - camera.positionY - Alphabet_Size[1] / 2, Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);

			}
			else
			{
				//Art is not working
				//animDraw(DeadthAnimation, playerOne.characters[0].posX - camera.positionX, playerOne.characters[0].posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1], deltaTime, 2);
			}
			//glDrawSprite(Character_Current_Image, Current_Character.posX - camera.positionX, Current_Character.posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1]);



			//draws Health
			//glDrawSprite(Current_Character.HP_Image[2], Current_Character.posX - camera.positionX + Alphabet_Size[0] / 2, Current_Character.posY - camera.positionY - Alphabet_Size[1] / 2, Alphabet_Size[0] / 2, Alphabet_Size[1] / 2);

			//}

			/*
			if (spriteTex_Current == Enemy_Right)
			{
			if (camera.positionY <= Current_Character.posY && Current_Character.posY <= camera.positionY + 480		//display image on-screen only
			&& camera.positionX <= Current_Character.posX && Current_Character.posX <= camera.positionX + 640) {
			glDrawSprite(Character_Current_Image, Current_Character.posX - camera.positionX, Current_Character.posY - camera.positionY, Character_Image_Size[0], Character_Image_Size[1]);

			}
			}
			*/

			//Camera drawing (Layer 3)

			//if (camera.positionY <= Current_Character.posY && Current_Character.posY <= camera.positionY + 480		//display image on-screen only
			//	&& camera.positionX <= Current_Character.posX && Current_Character.posX <= camera.positionX + 640) {
			//	//printf("\n !!!ProjectileArrayIndex: %d", ProjectileArrayIndex);
			//	if (sprite1_Alive) {
			//		glDrawSprite(Enemy_Left, EnemyOne.positionX - camera.positionX, EnemyOne.positionY - camera.positionY, spriteSize[0], spriteSize[1]);
			//		//printf("\nCurrent player.positionX: %f", player.positionX);
			//	}
			//	if (sprite2_Alive) {
			//		glDrawSprite(Enemy_Right, EnemyTwo.positionX - camera.positionX, EnemyTwo.positionY - camera.positionY, spriteSize[0], spriteSize[1]);
			//	}
			//}
			//glDrawSprite(projectile_image, player.positionX + 5 - camera.positionX, player.positionY + 15 - camera.positionY, projectileSize[0], projectileSize[1]);

			//Weapon drawing (Layer 3)

			//Effect drawing (Layer 3)
			if (!playerOne.characters[0].isDead && !playerTwo.characters[0].isDead)
			{
				for (int i = 0; i < DrawProjectiles.size(); i++) {
					if (camera.positionY <= DrawProjectiles[i].posY && DrawProjectiles[i].posY <= camera.positionY + 480		//display image on-screen only
						&& camera.positionX <= DrawProjectiles[i].posX && DrawProjectiles[i].posX <= camera.positionX + 640) {
						//printf("\n !!!ProjectileArrayIndex: %d", ProjectileArrayIndex);
						glDrawSprite(projectile_image, DrawProjectiles[i].posX + 5 - camera.positionX, DrawProjectiles[i].posY + 15 - camera.positionY, projectileSize[0], projectileSize[1]);
					}
				}

			}

			//Draw the timer on the bottom left corner
			if (playerOne.allCharDead == false && playerTwo.allCharDead == false) {
				glDrawSprite(TurnTimer[0], 0, 480 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(TurnTimer[1], Alphabet_Size[0], 480 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
			}

			//Display whose Turn
			if (delayTimer >= 0 && playerOne.allCharDead == false && playerTwo.allCharDead == false) {
				//Player 1's Characters 1-4
				if (Players_Turn == 0) {
					glDrawSprite(Alphabet_Image[15], 320 - 3 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[11], 320 - 2 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[0], 320 - 1 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[24], 320, 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[4], 320 + (1 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[17], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Numbers_Image[1], 320 + (3 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);

					/*if (Current_Character_number == 0) {
					glDrawSprite(Numbers_Image[1], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					}
					if (Current_Character_number == 1) {
					glDrawSprite(Numbers_Image[2], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					}
					if (Current_Character_number == 2) {
					glDrawSprite(Numbers_Image[3], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					}
					if (Current_Character_number == 3) {
					glDrawSprite(Numbers_Image[4], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					}*/

				}
				//Player 2's Characters 1-4
				if (Players_Turn == 1) {
					glDrawSprite(Alphabet_Image[15], 320 - 3 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[11], 320 - 2 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[0], 320 - 1 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[24], 320, 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[4], 320 + (1 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Alphabet_Image[17], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					glDrawSprite(Numbers_Image[2], 320 + (3 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);

					/*if (Current_Character_number == 0) {
					glDrawSprite(Numbers_Image[1], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					}
					if (Current_Character_number == 1) {
					glDrawSprite(Numbers_Image[2], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					}
					if (Current_Character_number == 2) {
					glDrawSprite(Numbers_Image[3], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					}
					if (Current_Character_number == 3) {
					glDrawSprite(Numbers_Image[4], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
					}*/

				}
			}

			//Calls the to see if all the characters on either side to see if they are dead for the Winner
			checkDeaths();

			//UI for Winner 
			//Player 1 Win
			if (playerTwo.allCharDead == true) {
				glDrawSprite(Alphabet_Image[15], 320 - 2 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(Numbers_Image[1], 320 - 1 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(Alphabet_Image[22], 320, 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(Alphabet_Image[8], 320 + (1 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(Alphabet_Image[13], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
			}
			//Player 2 Win
			if (playerOne.allCharDead == true) {
				glDrawSprite(Alphabet_Image[15], 320 - 2 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(Numbers_Image[2], 320 - 1 * Alphabet_Size[0], 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(Alphabet_Image[22], 320, 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(Alphabet_Image[8], 320 + (1 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
				glDrawSprite(Alphabet_Image[13], 320 + (2 * Alphabet_Size[0]), 120 - Alphabet_Size[1], Alphabet_Size[0], Alphabet_Size[1]);
			}

			//Water drawing (Layer 4)

			//UI drawing (Layer 5)

			SDL_GL_SwapWindow(window);
		}
	}

	SDL_Quit();

	return 0;
}
