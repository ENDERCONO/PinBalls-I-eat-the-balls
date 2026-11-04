#pragma once

#include "Globals.h"
#include "Module.h"

#include "p2Point.h"

#include "raylib.h"
#include <vector>

const int tableWidth = 500; //Alejandro hoy, size of table
const int tableHeight = 720;
const int tableX = (SCREEN_WIDTH - tableWidth) / 2; //table is centered in the window
const float wallRestitution = 0.25f;
//n
const int gameOverTitleY = 190;
const int gameOverTitleSize = 80;
const int buttonWidth = 320;
const int buttonHeight = 80;
const int buttonY = 400;
//end of n
//n
const int startingBalls = 3;

//N Variables para controlar los combos
//cuantos golpes admite el bumper para cumplir el bonus
const int bumperComboTarget = 3;
const int maxExtraBalls = 3;   // bolas maximas
//maximas bolas en pantalla
const int maxBalls = 5;


void restart();
void drawGameOver();
enum gameState { MENU, SETTINGS, PLAYING, GAME_OVER, WIN };

const int bumperPoints = 100;

//N puntos por tocar cosas (el bumper sigue sumando por bumperHits)
const int   wallPoints = 10;            //por tocar una pared
const int   slingPoints = 50;           //por tocar un triangulo (slingshot)
const int   otherPoints = 5;            //flippers, kicker y demas objetos sin tipo
const float wallMinSpeed = 3.0f;        //velocidad minima de la bola para que la pared puntue (y suene)
const float wallScoreCooldown = 0.1f;   //segundos entre dos toques de pared que puntuan

//N puntuacion necesaria para ganar la partida
const int   winScore = 3000;


void drawScorePanel();

//end of n

class PhysBody;
class PhysicEntity;
class b2Body; //N he necesitado añadirlo para los flippers

//Main
#define TABLE_W 500
#define TABLE_H 720
#define TABLE_X ((SCREEN_WIDTH - TABLE_W) / 2)

//Ball
#define BALL_RADIUS         12
#define BALL_MAX_SPEED      30.0f
#define TRAIL_LENGTH        7

//Flippers
#define FLIPPER_LENGTH      90
#define FLIPPER_SWING_DEG   45.0f
#define FLIPPER_RISE_TIME   0.07f
#define FLIPPER_SPRING_FREQ 22.0f
#define FLIPPER_DAMPING     0.55f

//Spring (en MAYÚSCULAS para que coincida con el .cpp del pinball)
#define SPRING_X            472
#define SPRING_REST_Y       650
#define SPRING_COMPRESSED_Y 690
#define SPRING_SPRING_K     150.0f
#define SPRING_LAUNCH_DELAY 1.0f

//Restitution
#define WALL_RESTITUTION    0.25f
#define BUMPER_RESTITUTION  0.9f
#define TARGET_RESTITUTION  0.5f
#define DIAMOND_RESTITUTION 1.1f
#define MOVER_RESTITUTION   0.6f
#define SLINGSHOT_RESTITUTION 1.4f

//Points
#define POINTS_BUMPER       100
#define POINTS_TARGET       50
#define POINTS_TARGET_LIT   10
#define POINTS_BANK_BONUS   500
#define POINTS_DIAMOND      30
#define POINTS_MOVER        20
#define POINTS_SLINGSHOT    10

enum GameState
{
	STATE_PLAYING,
	STATE_GAME_OVER
};

enum SpringState
{
	SPRING_LOADED,
	SPRING_RELEASED
};

class ModuleGame : public Module
{
public:
	ModuleGame(Application* app, bool start_enabled = true);
	~ModuleGame();

	bool Start();
	update_status Update();
	bool CleanUp();

	void OnCollision(PhysBody* bodyA, PhysBody* bodyB) override;

	// ---- PLANTILLA (activo) ----
	void DestroyEntity(PhysicEntity* entity);
	void DestroyAllEntities();

	//inside the class
	Texture2D texBackground;
	Texture2D texTable;
	//N agregado para los assets del kicker y flipper
	Texture2D texKicker;
	Texture2D texFlipper;
	//N sprite del bumper
	Texture2D texBumper;
	//N fuente tipo codigo para el comentario del menu (winScore)
	Font commentFont{};
	void drawMap();
	//N dibuja el sprite del flipper siguiendo la posicion y el angulo de su cuerpo de colision
	void drawFlipper(b2Body* body, bool mirrored);

	void buildWalls();

	//n
	void spawnBall();
	//spawn de la nuevas bolas
	void spawnExtraBall();
	gameState state;


	//Mas variables para los menus, su control
	//saber donde ir al volver
	gameState returnState;
	int   menuIndex = 0;
	int   settingsIndex = 0;
	float wallBounce = wallRestitution;

	//true si hay una partida empezada (para mostrar RESUME en vez de PLAY)
	bool  gameInProgress = false;

	//true cuando se pulsa EXIT en el menu
	bool  wantsExit = false;

	//momento del ultimo toque de pared que puntuo (para no sumar en cada frame)
	double lastWallScoreTime = 0.0;

	void updateMenu();
	void drawMenu();
	void updateSettings();
	void drawSettings();
	void openSettings(gameState from);



	int ballsLeft;
	PhysBody* currentBall;

	void ballLost();

	void restart();
	void drawGameOver();

	//victoria al llegar a winScore
	void winGame();
	void drawWin();

	void drawScorePanel();
	int score;
	//n numero de golpes que tiene el bumper
	int bumperCombo = 0;

	// N variables para tener la puntuación anterior y máxima

	int previousScore;
	int highScore;








	// end of "n"

	std::vector<PhysBody*> walls; //to free them in CleanUp
	std::vector<PhysBody*> extraBalls;   // bolas de bonus 

private:
	// ---- PLANTILLA (activo) ----
	std::vector<PhysicEntity*> entities;
	std::vector<PhysBody*> bodies_to_destroy;

	PhysBody* sensor;
	bool ray_on;
	vec2i ray;

	Texture2D circle;
	Texture2D box;
	Texture2D rick;
	uint32 bonus_fx;

	//vamos a crear las variables para guardar los sonidos
	uint32 fx_bumper, fx_sling, fx_wall, fx_flipper, fx_launch, fx_gameover;
};