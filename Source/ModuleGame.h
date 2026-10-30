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
void restart();
void drawGameOver();
enum gameState { PLAYING, GAME_OVER };

const int bumperPoints = 100;


void drawScorePanel();

//end of n

class PhysBody;
class PhysicEntity;

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
	void drawMap();

	void buildWalls();

	//n
	void spawnBall();

	gameState state;
	int ballsLeft; 
	PhysBody* currentBall; 

	void ballLost();

	void restart();
	void drawGameOver();

	void drawScorePanel();
	int score;

	// end of "n"

	std::vector<PhysBody*> walls; //to free them in CleanUp

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

	// ---- PINBALL (comentado) ----
	// Para usar el pinball: descomenta todo esto y comenta el bloque de PLANTILLA de arriba
	/*
	struct Flipper {
		PhysBody* body = NULL;
		bool left = false;
		float rest_angle = 0.0f;
		float pressed_angle = 0.0f;
		float angle = 0.0f;
		float angular_speed = 0.0f;
		float omega = 0.0f;
		bool was_pressed = false;
	};

	struct Popup { float x, y; float timer; int value; bool bonus; };
	struct Bumper { PhysBody* body; int x, y, radius; };
	struct Diamond { PhysBody* body; int x, y; };
	struct Kicker { PhysBody* body; int x, y; bool mirrored; };
	struct Target { PhysBody* body; int x, y, half_w, half_h; bool lit; };
	struct Mover {
		PhysBody* body;
		float cx, cy, ax, ay, speed, phase;
		bool spinner;
		int half_w, half_h, texture;
	};

	void BuildTable();
	void LoadAssets();
	void UnloadAssets();
	void SpawnBall();
	void RespawnBall();
	void Restart();
	void GameOver();
	void AddScore(int value, int x, int y, bool bonus = false);
	void UpdateFlipper(Flipper& flipper, bool pressed, float dt);
	void UpdateSpring(float dt);
	void UpdateMovers(float dt);
	void UpdateBallSafety(float dt);
	void UpdateEffects(float dt);
	PhysBody* AddBody(PhysBody* body, int kind, int points = 0);

	GameState state;
	int score, highscore, hits;
	std::vector<PhysBody*> bodies;
	PhysBody* ball;
	PhysBody* Spring;
	PhysBody* drain;
	Flipper flipper_left, flipper_right;
	std::vector<Bumper> bumpers;
	std::vector<Diamond> diamonds;
	std::vector<Kicker> kickers;
	std::vector<Target> targets;
	std::vector<Mover> movers;
	SpringState spring_state;
	float spring_timer, ball_stuck_timer, ball_idle_timer, mover_time, bank_timer;
	bool drain_hit;
	std::vector<Popup> popups;
	float trail_x[TRAIL_LENGTH];
	float trail_y[TRAIL_LENGTH];
	int trail_count;

	Texture2D tex_background, tex_table, tex_ball;
	Texture2D tex_bumper, tex_bumper_hit, tex_diamond, tex_diamond_hit;
	Texture2D tex_flipper, tex_kicker, tex_kicker_hit;
	Texture2D tex_target, tex_target_lit, tex_spring;
	Texture2D tex_platform_wide, tex_platform_small, tex_spinner;
	uint32 fx_bumper, fx_kicker, fx_target, fx_flipper, fx_launch, fx_wall, fx_gameover, fx_bonus;
	*/
};