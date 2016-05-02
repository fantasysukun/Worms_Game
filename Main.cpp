/*********************************************************************
Worms Game
version - may.1.2016
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

//Destroyable  background initialize (Layer 2)
GLuint Destroyable_BackGround[1600];

//player initialize (Layer 3)
GLuint player_Walking_Left[2];
GLuint player_Walking_Right[2];

//Enemy initialize (Layer 3)
GLuint Enemy_Right;
GLuint Enemy_Left;

//Effect initialize (Layer 3)
GLuint projectile_image;

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
int Static_BackGroundSize[2];

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

typedef struct Player
{
	float positionX = 526.0f;
	float positionY = 451.0f;

}Player;
Player player;

//Enemy Position setup
typedef struct Enemy
{
	float positionX;
	float positionY;
	int WalkingPosition;

}Enemy;
Enemy EnemyOne;
Enemy EnemyTwo;

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

/*
int Static_Background[40][40] =
{
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, },
	{ 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, },
	{ 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, },
	{ 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, },
	{ 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, },
	{ 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, },
	{ 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, },
	{ 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, },
	{ 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, },
	{ 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, },
	{ 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, },
	{ 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, },
	{ 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, },
	{ 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, },
	{ 560, 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597, 598, 599, },
	{ 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, },
	{ 640, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, },
	{ 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, },
	{ 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, },
	{ 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, },
	{ 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 838, 839, },
	{ 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879, },
	{ 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, },
	{ 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, },
	{ 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, },
	{ 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, },
	{ 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079, },
	{ 1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119, },
	{ 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1127, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1137, 1138, 1139, 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1152, 1153, 1154, 1155, 1156, 1157, 1158, 1159, },
	{ 1160, 1161, 1162, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1173, 1174, 1175, 1176, 1177, 1178, 1179, 1180, 1181, 1182, 1183, 1184, 1185, 1186, 1187, 1188, 1189, 1190, 1191, 1192, 1193, 1194, 1195, 1196, 1197, 1198, 1199, },
	{ 1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1210, 1211, 1212, 1213, 1214, 1215, 1216, 1217, 1218, 1219, 1220, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1233, 1234, 1235, 1236, 1237, 1238, 1239, },
	{ 1240, 1241, 1242, 1243, 1244, 1245, 1246, 1247, 1248, 1249, 1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258, 1259, 1260, 1261, 1262, 1263, 1264, 1265, 1266, 1267, 1268, 1269, 1270, 1271, 1272, 1273, 1274, 1275, 1276, 1277, 1278, 1279, },
	{ 1280, 1281, 1282, 1283, 1284, 1285, 1286, 1287, 1288, 1289, 1290, 1291, 1292, 1293, 1294, 1295, 1296, 1297, 1298, 1299, 1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310, 1311, 1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319, },
	{ 1320, 1321, 1322, 1323, 1324, 1325, 1326, 1327, 1328, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1343, 1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359, },
	{ 1360, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1370, 1371, 1372, 1373, 1374, 1375, 1376, 1377, 1378, 1379, 1380, 1381, 1382, 1383, 1384, 1385, 1386, 1387, 1388, 1389, 1390, 1391, 1392, 1393, 1394, 1395, 1396, 1397, 1398, 1399, },
	{ 1400, 1401, 1402, 1403, 1404, 1405, 1406, 1407, 1408, 1409, 1410, 1411, 1412, 1413, 1414, 1415, 1416, 1417, 1418, 1419, 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1428, 1429, 1430, 1431, 1432, 1433, 1434, 1435, 1436, 1437, 1438, 1439, },
	{ 1440, 1441, 1442, 1443, 1444, 1445, 1446, 1447, 1448, 1449, 1450, 1451, 1452, 1453, 1454, 1455, 1456, 1457, 1458, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467, 1468, 1469, 1470, 1471, 1472, 1473, 1474, 1475, 1476, 1477, 1478, 1479, },
	{ 1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487, 1488, 1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, 1499, 1500, 1501, 1502, 1503, 1504, 1505, 1506, 1507, 1508, 1509, 1510, 1511, 1512, 1513, 1514, 1515, 1516, 1517, 1518, 1519, },
	{ 1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, 1529, 1530, 1531, 1532, 1533, 1534, 1535, 1536, 1537, 1538, 1539, 1540, 1541, 1542, 1543, 1544, 1545, 1546, 1547, 1548, 1549, 1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1558, 1559, },
	{ 1560, 1561, 1562, 1563, 1564, 1565, 1566, 1567, 1568, 1569, 1570, 1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, 1579, 1580, 1581, 1582, 1583, 1584, 1585, 1586, 1587, 1588, 1589, 1590, 1591, 1592, 1593, 1594, 1595, 1596, 1597, 1598, 1599, },
};

int getStatic_Background(int x, int y) { return Static_Background[y][x]; }
*/

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

// Added pixel perfect detection - Kevin Lai

unsigned char* getBytes(const char* filename){

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
	bool** collision = new bool*[imageWidth];
	for (int i = 0; i < imageWidth; ++i){
		collision[i] = new bool[imageHeight];
	}

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
		collision[it % imageWidth][it / imageWidth] = isNonZero;

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

bool pixelPerfect(const char* filename1, const char* filename2, float x, float y, float w, float h, float x2, float y2, float w2, float h2){

	// Intersection: X, Y, Width, Height
	// Left, Top, Right, Bottom
	float intersectionX, intersectionY, intersectionW, intersectionH;

	if (x > x2){
		intersectionX = x;
	}
	else{
		intersectionX = x2;
	}
	if ( (x + w) > (x2 + w2) ){
		// Simplified version of (x2 + w2) - w2
		intersectionW = w2;
	}
	else{
		// Simplified version of (x + w) - w
		intersectionW = w;
	}
	if (y < y2){
		intersectionY = y2;
	}
	else{
		intersectionY = y;
	}
	if ( (y + h) < (y2 + h2) ){
		intersectionH = (y + h) - y2;
	}
	else{
		intersectionH = (y2 + h2) - y;
	}

	unsigned char* object1 = getBytes(filename1);
	unsigned char* object2 = getBytes(filename2);

	for (int i = 0; i < intersectionH; ++i){

		for (int j = 0; j < intersectionW; ++j){

			if ((object1[i*j] << ((int)(w - intersectionW)*32) ) & object2[i*j]){
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

void LoadStatic_Background() {
	int count = 0;
	char art_number[20];
	//char Static_Background_Name[50] = { 'A', 'r', 't', 'R', 'e', 's', 'o', 'u', 'r', 'c', 'e', '/', 'S', 't', 'a', 't', 'i', 'c', '_', 'B', 'a', 'c', 'k', 'g', 'r', 'o', 'u', 'n', 'd', (char) art_number, '.', 't', 'g', 'a', };
	//printf("Static_Background_Name: %s\n", Static_Background_Name);
	for (int y = 0; y < 40; y++)
	{
		for (int x = 0; x < 40; x++)
		{
			// ******!!!!!!!!!!!!!!!****** int convertion has bug ***************!!!!!!!!!!*********
			sprintf(art_number, "%d", count);
			//printf("art_number: %s\n", art_number);
			char Static_Background_Name[50] = { 'A', 'r', 't', 'R', 'e', 's', 'o', 'u', 'r', 'c', 'e', '/', 'S', 't', 'a', 't', 'i', 'c', '_', 'B', 'a', 'c', 'k', 'g', 'r', 'o', 'u', 'n', 'd', (char)art_number, '.', 't', 'g', 'a', };
			try {
				Static_BackGround[count] = glTexImageTGAFile(Static_Background_Name, &Static_BackGroundSize[0], &Static_BackGroundSize[1]);
				count++;
			}
			catch (exception e) {

			}
			//printf("curcount: %d\n", count);
			
		}
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

	
	


	/* Load the texture */
	Enemy_Right = glTexImageTGAFile("ArtResource/Enemy_Right.tga", &spriteSize[0], &spriteSize[1]);
	Enemy_Left = glTexImageTGAFile("ArtResource/Enemy_Left.tga", &spriteSize[0], &spriteSize[1]);
	spriteTex_Current = Enemy_Right;
	kbState = SDL_GetKeyboardState(NULL);


	//BackGround_bottom = glTexImageTGAFile("Background_bottom.tga", NULL, NULL);
	BackGround[0] = glTexImageTGAFile("ArtResource/13.tga", NULL, NULL);
	BackGround[1] = glTexImageTGAFile("ArtResource/14.tga", NULL, NULL);
	BackGround[2] = glTexImageTGAFile("ArtResource/4.tga", NULL, NULL);
	BackGround[3] = glTexImageTGAFile("ArtResource/Static_Background1.tga", NULL, NULL);

	player_Walking_Left[0] = glTexImageTGAFile("ArtResource/Left.tga", NULL, NULL);
	player_Walking_Left[1] = glTexImageTGAFile("ArtResource/Left_2.tga", NULL, NULL);
	player_Walking_Right[0] = glTexImageTGAFile("ArtResource/Right.tga", NULL, NULL);
	player_Walking_Right[1] = glTexImageTGAFile("ArtResource/Right_2.tga", NULL, NULL);

	LoadStatic_Background();
	
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
			
			CollisionResolution(player.positionX, player.positionY, spriteSize[0], spriteSize[1], 936, 936, 108, 108);
			/*
			if (AABB(player.positionX, player.positionY, spriteSize[0], spriteSize[1], 468, 936, 36*3, 36*3))
			{
				if (kbState[SDL_SCANCODE_LEFT]) {
					if (player.positionX > 426 && player.positionX + spriteSize[0] > 426 + spriteSize[0]) {
						player.positionX += 426 + spriteSize[0] - player.positionX;
					}
				}
				if (kbState[SDL_SCANCODE_RIGHT]) {
					if (player.positionX < 426 && player.positionX + spriteSize[0] > 426 && player.positionX + spriteSize[0] < 426 + spriteSize[0]) {
						player.positionX -= player.positionX + spriteSize[0] - 426;
					}
				}
				if (kbState[SDL_SCANCODE_UP]) {
					if (player.positionY > 351 && player.positionY + spriteSize[1] > 351 + spriteSize[1]) {
						player.positionY += 351 + spriteSize[1] - player.positionY;
					}
				}
				if (kbState[SDL_SCANCODE_DOWN]) {
					if (player.positionY < 351 && player.positionY + spriteSize[1] > 351 && player.positionY + spriteSize[1] < 351 + spriteSize[1]) {
						player.positionY -= player.positionY + spriteSize[1] - 351;
					}
				}
			}
			*/
			//printf("physicsDeltaMs: %f\n", physicsDeltaMs);
			//printf("lastPhysicsFrameMs: %f\n", lastPhysicsFrameMs);
			//printf("curFrameNS: %f\n", curFrameNS);
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

		for (int y = 0; y < 40; y++)
		{
			for (int x = 0; x < 40; x++)
			{
				int tileNum = getTile(x, y);
				// !!!!!!!!!! NEED Logic Checking !!!!!!!!!! 
				if (camera.positionY / 36 <= y + 11 && y <= camera.positionY / 36 + 15		//display image on-screen only
					&& camera.positionX / 36 <= x + 1 && x <= camera.positionX / 36 + 19) {// 
					glDrawSprite(BackGround[tileNum], 36 * x - camera.positionX, 36 * y - camera.positionY, 36, 36);

				}
			}
		}

		//Static background drawing (Layer 1)
		/*
		for (int y = 0; y < 40; y++)
		{
			for (int x = 0; x < 40; x++)
			{
				// !!!!!!!!!! NEED Logic Checking !!!!!!!!!! 
				if (camera.positionY / 36 <= y + 11 && y <= camera.positionY / 36 + 15		//display image on-screen only
					&& camera.positionX / 36 <= x + 1 && x <= camera.positionX / 36 + 19) {// 
					try {
						glDrawSprite(Static_BackGround[getStaticBackground(x, y)], 36 * x - camera.positionX, 36 * y - camera.positionY, 36, 36);
					}
					catch (exception e) {

					}
				}
			}
		}
		*/
		
		//Destroyable background drawing (Layer 2)

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

		//Camera drawing (Layer 3)

		if (camera.positionY <= player.positionY && player.positionY <= camera.positionY + 480		//display image on-screen only
			&& camera.positionX <= player.positionX && player.positionX <= camera.positionX + 640) {
			//printf("\n !!!ProjectileArrayIndex: %d", ProjectileArrayIndex);
			if (sprite1_Alive) {
				glDrawSprite(Enemy_Left, EnemyOne.positionX - camera.positionX, EnemyOne.positionY - camera.positionY, spriteSize[0], spriteSize[1]);
				printf("\nCurrent player.positionX: %f", player.positionX);
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
