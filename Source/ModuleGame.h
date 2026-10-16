#pragma once

#include "Globals.h"
#include "Module.h"

#include "p2Point.h"

#include "raylib.h"
#include <vector>

class PhysBody;

//Main
#define TABLE_W 500
#define TABLE_H 720
#define TABLE_X ((SCREEN_WIDTH - TABLE_W) / 2)

//Ball
#define BALL_RADIUS         12
#define BALL_MAX_SPEED      30.0f //max speed for the ball
#define TRAIL_LENGTH        7 //number of old positions for the ball's trail animation

//Flippers
#define FLIPPER_LENGTH      90
#define FLIPPER_SWING_DEG   45.0f // Max angle of rotation
#define FLIPPER_RISE_TIME   0.07f // rotation time when flipping
#define FLIPPER_SPRING_FREQ 22.0f // bounce back speed
#define FLIPPER_DAMPING     0.55f // bounce when dropping down

//Spring
#define Spring_X           472
#define Spring_REST_Y      650 // idle spring height
#define Spring_COMPRESSED_Y 690 // Spring height before launch
#define Spring_SPRING_K    150.0f // Spring constant (k) in N/m of spring
#define Spring_LAUNCH_DELAY 1.0f // Time before initial Launch

//Material restitution when collisioning
#define WALL_RESTITUTION    0.25f
#define BUMPER_RESTITUTION  0.9f
#define TARGET_RESTITUTION  0.5f
#define DIAMOND_RESTITUTION 1.1f
#define MOVER_RESTITUTION   0.6f
#define SLINGSHOT_RESTITUTION 1.4f // Since it's larger than 1 it adds speed and force to it

//Points
#define POINTS_BUMPER       100
#define POINTS_TARGET       50
#define POINTS_TARGET_LIT   10      
#define POINTS_BANK_BONUS   500     
#define POINTS_DIAMOND      30
#define POINTS_MOVER        20
#define POINTS_SLINGSHOT    10

class PhysicEntity;

enum GameState //To pause game when ended and restart
{
	STATE_PLAYING,
	STATE_GAME_OVER
};

enum SpringState //State of the Spring
{
	Spring_LOADED, // compressed, waiting
	Spring_RELEASED // the spring is pushing / resting
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

private:

	struct Flipper {
		PhysBody* Body = NULL;
		bool left = false;
		float rest_angle = 0.0f; //radians
		float pressed_angle = 0.0f; //radians
		float angle = 0.0f; //current angle of the flipper
		float angular_speed = 0.0f; //current angular speed
	};

	struct Popup {
		float x, y;
		float timer;
		int value;
		bool bonus;
	};

	struct Particle
	{
		float x, y, vx, vy;
		float life;
		Color color;
	};

	struct Bumper {
		PhysBody* body;
		int x, y, radius; //Window px
	};

	struct Diamond {
		PhysBody* body;
		int x, y; //center pos in window px
	};

	struct Kicker {
		PhysBody* body;
		int x, y; //window px
		bool mirrored;
	};

	struct Target {
		PhysBody* body;
		int x, y, half_w, half_h; //center in window px
		bool lit; //when hit for no multiple hits at the same time
	};
	struct Mover {
		PhysBody* body;
		float cx, cy; //center of movement in window px
		float ax, ay; //amplitude of the movement in px
		float speed; //angular speed in rad/s
		float phase;
		bool spinner; //if true it rotates forever and slides on false
		int half_w, half_h; //px
		int texture; //0 = wide platform, 1 = small platform, 2 = spinner platform
	};

	//Init
	void BuildTable();
	void LoadAssets();
	void UnloadAssets();

	//Main Game functions
	void SpawnBall();
	void RespawnBall();
	void Restart();
	void GameOver();
	void AddScore(int value, int x, int y, bool bonus = false);
	void SpawnParticles(int x, int y, int count, Color color);

	//Updates every frame
	void UpdateFlipper(Flipper& flipper, bool pressed, float dt);
	void UpdateSpring(float dt);
	void UpdateMovers(float dt);
	void UpdateBallSafety(float dt);
	void UpdateEffects(float dt);

	//Drawing textures
	void DrawSprite(Texture2D tex, float x, float y, float w, float h, float ox, float oy, float rotation_deg = 0.0f, bool flip_x = false, bool flip_y = false);
	void DrawBackground();
	void DrawTable();
	void DrawFlippers();
	void DrawBall();
	void DrawUI();
	void DrawPanel(int x, int y, int w, int h);
	void DrawGameOver();

	PhysBody* AddBody(PhysBody* body, int kind, int points = 0);

private:
	GameState state;
	int score;
	int highscore;
	int hits;

	//Physic objects
	std::vector<PhysBody*> bodies; //every body this module creates so we can then free them in memory
	PhysBody* ball;
	PhysBody* Spring;
	PhysBody* drain;
	Flipper flipper_left;
	Flipper flipper_right;
	std::vector<Bumper> bumpers;
	std::vector<Diamond> diamonds;
	std::vector<Kicker> kickers;
	std::vector<Target> targets;
	std::vector<Mover> movers;

	SpringState spring_state;
	float spring_timer;
	float ball_stuck_timer;
	float ball_idle_timer;
	float mover_time;
	float bank_timer;
	bool drain_hit;

	std::vector<Popup> popups;
	std::vector<Particle> particles;
	float trail_x[TRAIL_LENGTH];
	float trail_y[TRAIL_LENGTH];
	int trail_count;

	// Textures (pixel art, half the size they are drawn at)
	Texture2D tex_background;
	Texture2D tex_table;
	Texture2D tex_ball;
	Texture2D tex_bumper, tex_bumper_hit;
	Texture2D tex_diamond, tex_diamond_hit;
	Texture2D tex_flipper;
	Texture2D tex_kicker, tex_kicker_hit;
	Texture2D tex_target, tex_target_lit;
	Texture2D tex_spring;
	Texture2D tex_platform_wide, tex_platform_small, tex_spinner;

	// Sounds
	uint32 fx_bumper;
	uint32 fx_kicker;
	uint32 fx_target;
	uint32 fx_flipper;
	uint32 fx_launch;
	uint32 fx_wall;
	uint32 fx_gameover;
	uint32 fx_bonus;
};
