
#include "GameStateLevel1.h"
#include "CDT.h"
#include <iostream>

#include <irrKlang.h>
using namespace irrklang; 
ISoundEngine* SoundEngine;
// -------------------------------------------
// Defines
// -------------------------------------------

#define MESH_MAX					32				// The total number of Mesh (Shape)
#define TEXTURE_MAX					32				// The total number of texture
#define GAME_OBJ_INST_MAX			1024			// The total number of different game object instances
#define PLAYER_INITIAL_NUM			3				// initial number of ship lives
#define NUM_ASTEROID				30
#define SHIP_ACC_FWD				150.0f			// ship forward acceleration (in m/s^2)
#define SHIP_ACC_BWD				-180.0f			// ship backward acceleration (in m/s^2)
#define SHIP_ROT_SPEED				(2.0f * PI)		// ship rotation speed (degree/second)
#define HOMING_MISSILE_ROT_SPEED	(PI / 4.0f)		// homing missile rotation speed (degree/second)
#define BULLET_SPEED				300.0f			
#define ASTEROID_SPEED				250.0f	
#define MAX_SHIP_VELOCITY			200.0f

#define GIRL_SCALE					2

#define WINDOW_EDGE					25.0f

enum GAMEOBJ_TYPE
{
	// list of game object types
	//layer
	TYPE_START,
	TYPE_OVER,
	TYPE_GIRL,
	TYPE_BACKGROUND,
	TYPE_HEART,
	TYPE_POOP,
	TYPE_RAINBOW_POOP,
	TYPE_BULLET,
	TYPE_ASTEROID,
	TYPE_MISSILE
	
};

#define FLAG_INACTIVE		0
#define FLAG_ACTIVE			1


// -------------------------------------------
// Structure definitions
// -------------------------------------------

struct GameObj
{
	CDTMesh*		mesh;
	CDTTex*			tex;
	int				type;				// enum GAMEOBJ_TYPE
	int				flag;				// 0 - inactive, 1 - active
	glm::vec3		position;			// usually we will use only x and y
	glm::vec3		velocity;			// usually we will use only x and y
	glm::vec3		scale;				// usually we will use only x and y
	float			orientation;		// 0 radians is 3 o'clock, PI/2 radian is 12 o'clock
	glm::mat4		modelMatrix;
	int				frame;
	int				direction;
};


// -------------------------------------------
// Level variable, static - visible only in this file
// -------------------------------------------

static CDTMesh		sMeshArray[MESH_MAX];							// Store all unique shape/mesh in your game
static int			sNumMesh;
static CDTTex		sTexArray[TEXTURE_MAX];							// Corresponding texture of the mesh
static int			sNumTex;
static GameObj		sGameObjInstArray[GAME_OBJ_INST_MAX];			// Store all game object instance
static int			sNumGameObj;

static GameObj*		sStart;
static GameObj*		sOver;
static GameObj*		sPlayer;	
static GameObj*		sHeart;	// Pointer to the Player game object instance
static GameObj*		sBackground;
static GameObj*		sPOOP;
static GameObj*		sRAINBOW_POOP;

static int			sHeartLives;									// The number of lives left
static int			sScore;

// functions to create/destroy a game object instance
static GameObj*		gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient);
static void			gameObjInstDestroy(GameObj &pInst);


// -------------------------------------------
// Game object instant functions
// -------------------------------------------

GameObj* gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient)
{
	// loop through all object instance array to find the free slot
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++){
		GameObj* pInst = sGameObjInstArray + i;

		if (pInst->flag == FLAG_INACTIVE){
			
			pInst->mesh			= sMeshArray + type;
			pInst->tex			= sTexArray + type;
			pInst->type			= type;
			pInst->flag			= FLAG_ACTIVE;
			pInst->position		= pos;
			pInst->velocity		= vel;
			pInst->scale		= scale;
			pInst->orientation	= orient;
			pInst->modelMatrix  = glm::mat4(1.0f);
			pInst->frame		= 0;
			pInst->direction	= 0;

			sNumGameObj++;
			std::cout << "GameObj(" << sNumGameObj << ")\n";
			return pInst;
		}
	}

	// Cannot find empty slot => return 0
	return NULL;
}

void gameObjInstDestroy(GameObj &pInst)
{
	// Lazy deletion, not really delete the object, just set it as inactive
	if (pInst.flag == FLAG_INACTIVE)
		return;

	sNumGameObj--;
	pInst.flag = FLAG_INACTIVE;
}


// -------------------------------------------
// Game states function
// -------------------------------------------

void GameStateLevel1Load(void){

	// clear the Mesh array
	memset(sMeshArray, 0, sizeof(CDTMesh) * MESH_MAX);
	sNumMesh = 0;

	//+ clear the Texture array
	memset(sMeshArray, 0, sizeof(CDTTex) * TEXTURE_MAX);
	sNumMesh = 0;

	//+ clear the game object instance array
	memset(sGameObjInstArray, 0, sizeof(GameObj) * GAME_OBJ_INST_MAX);
	sNumMesh = 0;

	// Set the ship object instance to NULL
	sPlayer = NULL;
	

	// --------------------------------------------------------------------------
	// Create all of the unique meshes/textures and put them in MeshArray/TexArray
	//		- The order of mesh should follow enum GAMEOBJ_TYPE 
	/// --------------------------------------------------------------------------

	// Temporary variable for creating mesh
	CDTMesh* pMesh;
	CDTTex* pTex;
	std::vector<CDTVertex> vertices;
	CDTVertex v1, v2, v3, v4;

	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f / 10.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f / 10.0f; v3.v = 1.0f / 4.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f / 4.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("gameover.png");
	//bgSprite = CreateMesh(vertices);
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f / 10.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f / 10.0f; v3.v = 1.0f / 4.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f / 4.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("canva.png");
	// Create Ship mesh/texture
	//walking
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f / 10.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f / 10.0f; v3.v = 1.0f / 4.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f / 4.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("walking.png");

	//+ Create Background mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Green forest (Anime Background).png");

	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("bar.png");

	//+ Create Poop mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("poop_emoji.png");

	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("rainbow-poop.png");

	//sGameObjInstArray


	printf("Level1: Load\n");
}

float randomFloat()
{
	return (float)(rand()) / (float)(RAND_MAX);
}

int randomInt(int a, int b)
{
	if (a > b)
		return randomInt(b, a);
	if (a == b)
		return a;
	return a + (rand() % (b - a));
}

float randomFloat(int a, int b)
{
	if (a > b)
		return randomFloat(b, a);
	if (a == b)
		return a;

	return (float)randomInt(a, b) + randomFloat();
}

void GameStateLevel1Init(void){
	/*sStart = gameObjInstCreate(TYPE_START, glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1245.0f, 713.0f, 1.0f), 0.0f);*/
	//+ Create the background instance
	//	- Creation order is important when rendering, so we should create the background first
	sBackground = gameObjInstCreate(TYPE_BACKGROUND, glm::vec3(0.0f, 0.0f , 0.0f),
								glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1245.0f, 713.0f, 1.0f), 0.0f);
	//bgSprite.modelMatrix = glm::scale(bgSprite.modelMatrix, glm::vec3(9.0f, 9.0f, 0.0f));
	
	// Create player game object instance
	//	- the position.z should be set to 0
	//	- the scale.z should be set to 1
	//	- the velocity.z should be set to 0
	sPlayer = gameObjInstCreate(TYPE_GIRL, glm::vec3(0.0f,-GetWindowHeight()/4, 0.0f),
									glm::vec3(0.0f,0.0f,0.0f),glm::vec3(64.0f * GIRL_SCALE, 128.0f * GIRL_SCALE,1.0f),0.0f);

	sHeart = gameObjInstCreate(TYPE_HEART, glm::vec3(0.0f, 250.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(447.0f, 35.0f, 1.0f), 0.0f);


	SoundEngine = createIrrKlangDevice();

	//+ Create all asteroid instance, NUM_ASTEROID, with random pos and velocity
	//	- int a = rand() % 30 + 20;							// a is in the range 20-50
	//	- float b = (float)rand()/(float)(RAND_MAX);		// b is the range 0..1

	
	int num_poop = rand() % 10 + 5; // number of poop

	for (int i = 0; i < num_poop; i++)
	{
		//float fast_drop = (float)rand() / (float)(RAND_MAX / (ASTEROID_SPEED)) - 100;
		float randompX = (float)rand() / (float)(RAND_MAX / GetWindowWidth()) - (GetWindowWidth() / 2);
		gameObjInstCreate(TYPE_POOP, glm::vec3(randompX, GetWindowHeight()/2 , 0.0f),
			glm::vec3(0.0f, randomFloat(-50, -250), 0.0f), glm::vec3(32.0f, 32.0f, 1.0f), 0.0f);
	}
	for (int i = 0; i < num_poop; i++) {
		//float fast_drop = (float)rand() / (float)(RAND_MAX / (ASTEROID_SPEED)) - 100;
		float randompX = (float)rand() / (float)(RAND_MAX / GetWindowWidth()) - (GetWindowWidth() / 2);

		gameObjInstCreate(TYPE_RAINBOW_POOP, glm::vec3(randompX, GetWindowHeight() / 2, 0.0f),
			glm::vec3(0.0f, randomFloat(-50, -100), 0.0f), glm::vec3(32.0f, 32.0f, 1.0f), 0.0f);
	}
	//+ reset the score and player life
	printf("Level1: Init\n");
}


void GameStateLevel1Update(double dt, long frame, int &state){

	//-----------------------------------------
	// Get user input
	//-----------------------------------------
	
	// Moving the Player
	//	- WS accelereate/deaccelerate the ship
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){

		// find acceleration vector
		//SoundEngine->play2D("bamboo.ogg", false);
		glm::vec3 acc = glm::vec3(SHIP_ACC_FWD * glm::cos(sPlayer->orientation + PI / 2.0f), 
								  SHIP_ACC_FWD * glm::sin(sPlayer->orientation + PI / 2.0f), 0);

		// use acceleration to change velocity
		sPlayer->velocity += acc * glm::vec3(dt, dt, 0.0f);

		//+ velocity cap to MAX_SHIP_VELOCITY
		sPlayer->direction = 0;


	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		sPlayer->direction = 3;

		glm::vec3 acc = glm::vec3(SHIP_ACC_BWD * glm::cos(sPlayer->orientation + PI / 2.0f),
								  SHIP_ACC_BWD * glm::sin(sPlayer->orientation + PI / 2.0f), 0);

		sPlayer->velocity += acc * glm::vec3(dt, dt, 0.0f);
	}

	sPlayer->velocity = glm::vec3(0.0f, 0.0f, 0.0f);

	//+ AD: turn the ship
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		sPlayer->direction = 2;

		sPlayer->velocity = glm::vec3(-100.0f, 0.0f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		sPlayer->direction = 1;

		sPlayer->velocity = glm::vec3(100.0f, 0.0f, 0.0f);
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		printf("Mouse at x: %f y: %f\n", xpos, ypos);
	}
	// Fire bullet/missile using JK
	//	- create the bullet at the ship's position
	//	- bullet direction is the same as the ship's orientation
	//	- may use if(frame % n == 0) too slow down the bullet creation
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS){
		//gameObjInstDestroy(*sStart);
		//+ find the bullet velocity vector

		//+ call gameObjInstCreate() to create a bullet

		
	}


	// Cam zoom UI, for Debugging
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS){
		ZoomIn(0.1f);
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
		ZoomOut(0.1f);
	}


	//---------------------------------------------------------
	// Update all game obj position using velocity 
	//---------------------------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++){
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		if (pInst->type == TYPE_GIRL){

			//+ for ship: add some friction to slow it down

			//+ use velocity to update the position
			sPlayer->position += sPlayer->velocity * (float)dt;
		}
		else if ((pInst->type == TYPE_POOP)){
			
			//+ use velocity to update the position
			pInst->position += pInst->velocity * (float)dt;

		}
		if ((pInst->type == TYPE_RAINBOW_POOP)) {

			//+ use velocity to update the position
			pInst->position += pInst->velocity * (float)dt;

		}
	}


	//-----------------------------------------
	// Update some game obj behavior
	//	- wrap ship around the screen
	//	- destroy bullet that go out of the screen
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++){
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		if ((pInst->type == TYPE_GIRL) || (pInst->type == TYPE_POOP) || (pInst->type == TYPE_RAINBOW_POOP)){

			//+ wrap the ship and asteroid around the screen 
			if (pInst->position.x < -GetWindowWidth() / 2 - WINDOW_EDGE) {
				pInst->position.x = GetWindowWidth() / 2 + WINDOW_EDGE;
			}
			if (pInst->position.x > GetWindowWidth() / 2 + WINDOW_EDGE) {
				pInst->position.x = -GetWindowWidth() / 2 - WINDOW_EDGE;
			}
			if (pInst->position.y < -GetWindowHeight() / 2 - WINDOW_EDGE) {
				pInst->position.y = GetWindowHeight() / 2 + WINDOW_EDGE;
			}
			if (pInst->position.y > GetWindowHeight() / 2 + WINDOW_EDGE) {
				pInst->position.y = -GetWindowHeight() / 2 - WINDOW_EDGE;
			}
		}
	}

	//-----------------------------------------
	// Check for collsion, O(n^2)
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++){
		GameObj* pInst1 = sGameObjInstArray + i;

		// skip inactive object
		if (pInst1->flag == FLAG_INACTIVE)
			continue;

		// if pInst1 is an asteroid
		if (pInst1->type == TYPE_POOP || pInst1->type == TYPE_RAINBOW_POOP){
			
			// compare pInst1 with all game obj instances 
			for (int j = 0; j < GAME_OBJ_INST_MAX; j++){
				GameObj* pInst2 = sGameObjInstArray + j;

				// skip inactive object
				if (pInst2->flag == FLAG_INACTIVE)
					continue;

				// skip asteroid object
				if (pInst2->type == TYPE_POOP)
					continue;
				if (pInst2->type == TYPE_RAINBOW_POOP)
					continue;

				if (pInst2->type == TYPE_GIRL){
					
					bool collide = true;

					//+ Check for collsion
					float distance = glm::length(pInst2->position - pInst1->position);
					if (distance > (pInst1->scale.x + pInst2->scale.x) / 2.0f)
					{
						collide = false;
 						//SoundEngine->stopAllSounds();
					}
					if (collide){
						SoundEngine->play2D("bamboo.ogg", false);
						//+ Update game behavior and sGameObjInstArray
						sHeart->scale -= glm::vec3(20.0f, 0.0f, 1.0f);
						gameObjInstDestroy(*pInst1);
						GameObj* pInst2 = sGameObjInstArray + j;
						sHeartLives--;
						
						break;
					}	
					if (sHeart->scale.x == 0.0f) {
						sOver = gameObjInstCreate(TYPE_OVER, glm::vec3(0.0f, 0.0f, 0.0f),
							glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1245.0f, 713.0f, 1.0f), 0.0f);
					}
					/*if (collide == false) {
						SoundEngine->drop();
					}*/
					
				}
				
			}
		}
	}



	//-----------------------------------------
	// Update animations
	//-----------------------------------------

	if ((frame % 10) == 0 && ++(sPlayer->frame) == 10)
		sPlayer->frame = 0;
	

	//-----------------------------------------
	// Update modelMatrix of all game obj
	//-----------------------------------------
	
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++  ){
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		glm::mat4 rMat = glm::mat4(1.0f);
		glm::mat4 sMat = glm::mat4(1.0f);
		glm::mat4 tMat = glm::mat4(1.0f);

		// Compute the scaling matrix
		sMat = glm::scale(glm::mat4(1.0f), pInst->scale);

		//+ Compute the rotation matrix, we should rotate around z axis
		rMat = glm::rotate(glm::mat4(1.0f), pInst->orientation, glm::vec3(0, 0, 1));

		//+ Compute the translation matrix
		tMat = glm::translate(glm::mat4(1.0f), pInst->position);

		// Concatenate the 3 matrix to from Model Matrix
		pInst->modelMatrix = tMat * sMat * rMat;
	}

	//printf("Life> %i\n", sPlayerLives);
	//printf("Score> %i\n", sScore);
}

void GameStateLevel1Draw(void){

	// Clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw all game object instance in the sGameObjInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++){
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		// 4 steps to draw sprites on the screen
		//	1. SetRenderMode()
		//	2. SetTexture()
		//	3. SetTransform()
		//	4. DrawMesh()

		SetRenderMode(CDT_TEXTURE, 1.0f);

		if (pInst->type == TYPE_GIRL)
			SetTexture(*pInst->tex, pInst->frame / 10.0f, pInst->direction / 4.0f);
		else
			SetTexture(*pInst->tex, 0.0f, 0.0f);

		SetTransform(pInst->modelMatrix);
		DrawMesh(*pInst->mesh);
	}

	// Swap the buffer, to present the drawing
	glfwSwapBuffers(window);
}

void GameStateLevel1Free(void){

	//+ call gameObjInstDestroy for all object instances in the sGameObjInstArray

	// reset camera
	ResetCam();
	SoundEngine->drop();
	printf("Level1: Free\n");
}

void GameStateLevel1Unload(void){

	// Unload all meshes in MeshArray
	for (int i = 0; i < sNumMesh; i++){
		UnloadMesh(sMeshArray[i]);
	}

	//+ Unload all textures in TexArray


	printf("Level1: Unload\n");
}
