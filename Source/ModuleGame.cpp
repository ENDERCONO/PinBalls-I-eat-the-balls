#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

#include <algorithm>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// =====================================================================
// PINBALL (desactivado). Para usarlo: cambia "#if 0" por "#if 1"
// y pon "#if 1" -> "#if 0" en el bloque de PLANTILLA de más abajo.
// =====================================================================
#if 0

static int TX(int x) //table coords to window px
{
	return TABLE_X + x;
}

static float Snap(float v) //since we draw at half res
{
	return floorf(v / PIXEL_SCALE + 0.5f) * PIXEL_SCALE;
}

static void DrawPixelText(const char* text, int x, int y, int size, Color color)
{
	DrawText(text, x + 2, y + 2, size, Color{ 0, 0, 0, 200 });
	DrawText(text, x, y, size, color);
}

static void DrawPixelTextCentered(const char* text, int center_x, int y, int size, Color color)
{
	DrawPixelText(text, center_x - MeasureText(text, size) / 2, y, size, color);
}

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	state = STATE_PLAYING;
	score = 0;
	highscore = 0;
	hits = 0;

	ball = NULL;
	Spring = NULL;
	drain = NULL;

	spring_state = SPRING_LOADED;
	spring_timer = 0.0f;
	ball_stuck_timer = 0.0f;
	ball_idle_timer = 0.0f;
	mover_time = 0.0f;
	bank_timer = 0.0f;
	drain_hit = false;
	trail_count = 0;

	tex_background = Texture2D{ 0 };
	tex_table = Texture2D{ 0 };
	tex_ball = Texture2D{ 0 };
	tex_bumper = tex_bumper_hit = Texture2D{ 0 };
	tex_diamond = tex_diamond_hit = Texture2D{ 0 };
	tex_flipper = Texture2D{ 0 };
	tex_kicker = tex_kicker_hit = Texture2D{ 0 };
	tex_target = tex_target_lit = Texture2D{ 0 };
	tex_spring = Texture2D{ 0 };
	tex_platform_wide = tex_platform_small = tex_spinner = Texture2D{ 0 };

	fx_bumper = fx_kicker = fx_target = fx_flipper = fx_launch = fx_wall = fx_gameover = fx_bonus = 0;
}

ModuleGame::~ModuleGame() {}

bool ModuleGame::Start() {
	LOG("Loading game");

	App->renderer->camera.x = App->renderer->camera.y = 0;

	LoadAssets();
	BuildTable();
	Restart();

	return true;
}

bool ModuleGame::CleanUp() {
	LOG("Unloading game");

	if (ball != NULL)
	{
		App->physics->DestroyBody(ball);
		ball = NULL;
	}

	for (PhysBody* body : bodies)
		App->physics->DestroyBody(body);

	bodies.clear();
	bumpers.clear();
	diamonds.clear();
	kickers.clear();
	targets.clear();
	movers.clear();
	Spring = NULL;
	drain = NULL;
	flipper_left.body = NULL;
	flipper_right.body = NULL;

	UnloadAssets();

	return true;
}

void ModuleGame::LoadAssets() {
	tex_background = LoadTexture("Assets/background.png");
	tex_table = LoadTexture("Assets/table.png");
	tex_ball = LoadTexture("Assets/ball.png");
	tex_bumper = LoadTexture("Assets/bumper.png");
	tex_bumper_hit = LoadTexture("Assets/bumper_hit.png");
	tex_diamond = LoadTexture("Assets/diamond.png");
	tex_diamond_hit = LoadTexture("Assets/diamond_hit.png");
	tex_flipper = LoadTexture("Assets/flipper.png");
	tex_kicker = LoadTexture("Assets/kicker.png");
	tex_kicker_hit = LoadTexture("Assets/kicker_hit.png");
	tex_target = LoadTexture("Assets/target.png");
	tex_target_lit = LoadTexture("Assets/target_lit.png");
	tex_spring = LoadTexture("Assets/plunger.png");
	tex_platform_wide = LoadTexture("Assets/platform_wide.png");
	tex_platform_small = LoadTexture("Assets/platform_small.png");
	tex_spinner = LoadTexture("Assets/spinner.png");

	Texture2D all[] = { tex_background, tex_table, tex_ball, tex_bumper, tex_bumper_hit, tex_diamond, tex_diamond_hit,
		tex_flipper, tex_kicker, tex_kicker_hit, tex_target, tex_target_lit, tex_spring,
		tex_platform_wide, tex_platform_small, tex_spinner };
	for (Texture2D t : all)
		SetTextureFilter(t, TEXTURE_FILTER_POINT);

	fx_bumper = App->audio->LoadFx("Assets/bumper.wav");
	fx_kicker = App->audio->LoadFx("Assets/slingshot.wav");
	fx_target = App->audio->LoadFx("Assets/bonus.wav");
	fx_flipper = App->audio->LoadFx("Assets/flipper.wav");
	fx_launch = App->audio->LoadFx("Assets/launch.wav");
	fx_wall = App->audio->LoadFx("Assets/wall.wav");
	fx_gameover = App->audio->LoadFx("Assets/gameover.wav");
	fx_bonus = App->audio->LoadFx("Assets/bank.wav");
}

void ModuleGame::UnloadAssets() {
	Texture2D all[] = { tex_background, tex_table, tex_ball, tex_bumper, tex_bumper_hit, tex_diamond, tex_diamond_hit,
		tex_flipper, tex_kicker, tex_kicker_hit, tex_target, tex_target_lit, tex_spring,
		tex_platform_wide, tex_platform_small, tex_spinner };
	for (Texture2D t : all)
		UnloadTexture(t);
}

PhysBody* ModuleGame::AddBody(PhysBody* body, int kind, int points) {
	body->kind = kind;
	body->points = points;
	bodies.push_back(body);
	return body;
}

void ModuleGame::BuildTable() {
	ModulePhysics* physics = App->physics;
	std::vector<int> shell;
	shell.push_back(490); shell.push_back(700);
	shell.push_back(490); shell.push_back(250);
	for (int i = 1; i <= 36; ++i) {
		float a = (float)i * PI / 36.0f;
		shell.push_back((int)(250.0f + 240.0f * cosf(a)));
		shell.push_back((int)(250.0f - 240.0f * sinf(a)));
	}
	shell.push_back(10);  shell.push_back(520);
	shell.push_back(126); shell.push_back(591);
	AddBody(physics->CreateChain(TABLE_X, 0, shell.data(), (int)shell.size(), false, WALL_RESTITUTION, 0.1f), KIND_WALL);

	const int lane_wall[] = { 455, 700, 455, 200 };
	AddBody(physics->CreateChain(TABLE_X, 0, lane_wall, 4, false, WALL_RESTITUTION, 0.1f), KIND_WALL);

	const int right_guide[] = { 455, 520, 338, 591 };
	AddBody(physics->CreateChain(TABLE_X, 0, right_guide, 4, false, WALL_RESTITUTION, 0.1f), KIND_WALL);

	const int gate_line[] = { 455, 200, 490, 150 };
	PhysBody* gate = AddBody(physics->CreateChain(TABLE_X, 0, gate_line, 4, false, WALL_RESTITUTION, 0.1f), KIND_GATE);
	gate->gate_nx = -50.0f / 61.0f;
	gate->gate_ny = -35.0f / 61.0f;

	drain = AddBody(physics->CreateRectangleSensor(TABLE_X + TABLE_W / 2, TABLE_H + 40, TABLE_W / 2 + 20, 28), KIND_DRAIN);
	drain->listener = this;

	Spring = AddBody(physics->CreateSPRING(TX(SPRING_X), SPRING_COMPRESSED_Y, 15, 10, SPRING_COMPRESSED_Y - SPRING_REST_Y), KIND_SPRING);

	const int flipper_shape[] = { -10, -6, -10, 6, 0, 13, FLIPPER_LENGTH, 7, FLIPPER_LENGTH, -7, 0, -13 };

	flipper_left.left = true;
	flipper_left.rest_angle = 30.0f * DEGTORAD;
	flipper_left.pressed_angle = (30.0f - FLIPPER_SWING_DEG) * DEGTORAD;
	flipper_left.body = AddBody(physics->CreatePolygon(TX(126), 600, flipper_shape, 12, b2_kinematicBody, 0.2f, 0.4f, 30.0f), KIND_FLIPPER);

	flipper_right.left = false;
	flipper_right.rest_angle = -210.0f * DEGTORAD;
	flipper_right.pressed_angle = (-210.0f + FLIPPER_SWING_DEG) * DEGTORAD;
	flipper_right.body = AddBody(physics->CreatePolygon(TX(338), 600, flipper_shape, 12, b2_kinematicBody, 0.2f, 0.4f, -210.0f), KIND_FLIPPER);

	const int kicker_left_shape[] = { 40, 430, 40, 500, 100, 535 };
	const int kicker_right_shape[] = { 425, 430, 425, 500, 365, 535 };

	Kicker kl = { AddBody(physics->CreatePolygon(TABLE_X, 0, kicker_left_shape, 6, b2_staticBody, SLINGSHOT_RESTITUTION, 0.0f), KIND_BOUNCY, POINTS_SLINGSHOT), TX(36), 426, false };
	Kicker kr = { AddBody(physics->CreatePolygon(TABLE_X, 0, kicker_right_shape, 6, b2_staticBody, SLINGSHOT_RESTITUTION, 0.0f), KIND_BOUNCY, POINTS_SLINGSHOT), TX(361), 426, true };
	kickers.push_back(kl);
	kickers.push_back(kr);

	const int bumper_pos[][2] = { { 170, 205 }, { 300, 205 }, { 235, 268 } };
	for (int i = 0; i < 3; ++i) {
		Bumper b;
		b.x = TX(bumper_pos[i][0]);
		b.y = bumper_pos[i][1];
		b.radius = 22;
		b.body = AddBody(physics->CreateCircle(b.x, b.y, b.radius, b2_staticBody, BUMPER_RESTITUTION, 0.0f), KIND_BUMPER, POINTS_BUMPER);
		bumpers.push_back(b);
	}

	const int diamond_shape[] = { 0, -20, 20, 0, 0, 20, -20, 0 };
	const int diamond_pos[][2] = { {78, 255}, {392, 255} };
	for (int i = 0; i < 2; ++i) {
		Diamond d;
		d.x = TX(diamond_pos[i][0]);
		d.y = diamond_pos[i][1];
		d.body = AddBody(physics->CreatePolygon(d.x, d.y, diamond_shape, 8, b2_staticBody, DIAMOND_RESTITUTION, 0.0f), KIND_BOUNCY, POINTS_DIAMOND);
		diamonds.push_back(d);
	}

	const int target_pos[][2] = { {22, 310}, {22, 360}, {443, 310}, {443, 360} };
	for (int i = 0; i < 4; ++i) {
		Target t;
		t.x = TX(target_pos[i][0]);
		t.y = target_pos[i][1];
		t.half_w = 6;
		t.half_h = 22;
		t.lit = false;
		t.body = AddBody(physics->CreateRectangle(t.x, t.y, t.half_w, t.half_h, b2_staticBody, TARGET_RESTITUTION, 0.1f), KIND_TARGET, POINTS_TARGET);
		targets.push_back(t);
	}

	{
		Mover m;
		m.cx = (float)TX(232); m.cy = 490.0f; m.ax = 70.0f; m.ay = 0.0f;
		m.speed = 1.3f; m.phase = 0.0f; m.spinner = false;
		m.half_w = 32; m.half_h = 5; m.texture = 0;
		m.body = AddBody(physics->CreateRectangle((int)m.cx, (int)m.cy, m.half_w, m.half_h, b2_kinematicBody, MOVER_RESTITUTION, 0.05f, 8.0f), KIND_MOVER, POINTS_MOVER);
		movers.push_back(m);
	}
	{
		Mover m;
		m.cx = (float)TX(385); m.cy = 350.0f; m.ax = 0.0f; m.ay = 50.0f;
		m.speed = 1.0f; m.phase = 0.0f; m.spinner = false;
		m.half_w = 24; m.half_h = 5; m.texture = 1;
		m.body = AddBody(physics->CreateRectangle((int)m.cx, (int)m.cy, m.half_w, m.half_h, b2_kinematicBody, MOVER_RESTITUTION, 0.05f, -12.0f), KIND_MOVER, POINTS_MOVER);
		movers.push_back(m);
	}
	{
		Mover m;
		m.cx = (float)TX(118); m.cy = 345.0f; m.ax = 0.0f; m.ay = 0.0f;
		m.speed = 2.2f; m.phase = 0.0f; m.spinner = true;
		m.half_w = 40; m.half_h = 5; m.texture = 2;
		m.body = AddBody(physics->CreateRectangle((int)m.cx, (int)m.cy, m.half_w, m.half_h, b2_kinematicBody, MOVER_RESTITUTION, 0.2f, 0.0f), KIND_MOVER, POINTS_MOVER);
		movers.push_back(m);
	}
}

void ModuleGame::SpawnBall() {
	if (ball != NULL) {
		App->physics->DestroyBody(ball);
		ball = NULL;
	}
	ball = App->physics->CreateCircle(TX(SPRING_X), SPRING_COMPRESSED_Y - 10 - BALL_RADIUS - 1, BALL_RADIUS, b2_dynamicBody, 0.3f, 0.1f, true);
	ball->kind = KIND_BALL;
	ball->listener = this;
	ball->body->SetSleepingAllowed(false);
	trail_count = 0;
	ball_stuck_timer = 0.0f;
	ball_idle_timer = 0.0f;
}

void ModuleGame::RespawnBall() {
	spring_state = SPRING_LOADED;
	spring_timer = SPRING_LAUNCH_DELAY;
	Spring->SetPosition(TX(SPRING_X), SPRING_COMPRESSED_Y);
	Spring->SetLinearVelocity(0.0f, 0.0f);

	SpawnBall();
}

void ModuleGame::Restart() {
	state = STATE_PLAYING;
	score = 0;
	hits = 0;
	drain_hit = false;
	bank_timer = 0.0f;
	popups.clear();
	particles.clear();

	for (Target& t : targets)
		t.lit = false;

	Flipper* flippers[2] = { &flipper_left, &flipper_right };
	for (Flipper* f : flippers) {
		f->angle = f->rest_angle;
		f->angular_speed = 0.0f;
		f->was_pressed = false;
		f->body->body->SetTransform(f->body->body->GetPosition(), f->rest_angle);
		f->body->SetAngularVelocity(0.0f);
	}
	RespawnBall();
}

void ModuleGame::GameOver() {
	state = STATE_GAME_OVER;
	if (score > highscore)
		highscore = score;
	App->physics->DestroyBody(ball);
	ball = NULL;
	flipper_left.body->SetAngularVelocity(0.0f);
	flipper_right.body->SetAngularVelocity(0.0f);
	App->audio->PlayFx(fx_gameover);
}

void ModuleGame::AddScore(int value, int x, int y, bool bonus) {
	score += value;
	hits++;
	Popup p = { (float)x, (float)y, bonus ? 1.4f : 0.8f, value, bonus };
	popups.push_back(p);
}

void ModuleGame::UpdateFlipper(Flipper& flipper, bool pressed, float dt)
{
	if (pressed && flipper.was_pressed == false)
		App->audio->PlayFx(fx_flipper);

	if (pressed)
	{
		float speed = (FLIPPER_SWING_DEG * DEGTORAD) / FLIPPER_RISE_TIME;
		float direction = (flipper.pressed_angle > flipper.rest_angle) ? 1.0f : -1.0f;

		flipper.angle += direction * speed * dt;

		if ((direction > 0.0f && flipper.angle >= flipper.pressed_angle) ||
			(direction < 0.0f && flipper.angle <= flipper.pressed_angle))
			flipper.angle = flipper.pressed_angle;

		flipper.angular_speed = 0.0f;
	}
	else
	{
		float k = FLIPPER_SPRING_FREQ * FLIPPER_SPRING_FREQ;
		float c = 2.0f * FLIPPER_DAMPING * FLIPPER_SPRING_FREQ;

		flipper.angular_speed += (k * (flipper.rest_angle - flipper.angle) - c * flipper.omega) * dt;
		flipper.angle += flipper.angular_speed * dt;

		if (fabsf(flipper.rest_angle - flipper.angle) < 0.0005f && fabsf(flipper.omega) < 0.01f)
		{
			flipper.angle = flipper.rest_angle;
			flipper.omega = 0.0f;
		}
	}

	float current_angle = flipper.body->GetRotation();
	flipper.body->SetAngularVelocity((flipper.angle - current_angle) / dt);

	flipper.was_pressed = pressed;
}

#endif // PINBALL


// =====================================================================
// PLANTILLA (activa)
// =====================================================================
#if 1

// Base class for everything in the scene that has a physics body
class PhysicEntity
{
protected:
	PhysicEntity(PhysBody* _body)
		: body(_body)
	{
	}

public:
	virtual ~PhysicEntity() = default;
	virtual void Update() = 0;

public:
	PhysBody* body;
};

class Circle : public PhysicEntity
{
public:
	Circle(ModulePhysics* physics, int _x, int _y, Texture2D _texture)
		: PhysicEntity(physics->CreateCircle(_x, _y, 11))
		, texture(_texture)
	{
	}

	void Update() override
	{
		int x, y;
		body->GetPosition(x, y);

		Color tint = body->Contains(GetMouseX(), GetMouseY()) ? RED : WHITE;

		Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
		Rectangle dest = { (float)x, (float)y, (float)22.0f, (float)22.0f };
		Vector2 origin = { (float)11.0f, (float)11.0f };
		DrawTexturePro(texture, source, dest, origin, body->GetRotation() * RAD2DEG, tint);
	}

private:
	Texture2D texture;
};

class Box : public PhysicEntity
{
public:
	Box(ModulePhysics* physics, int _x, int _y, Texture2D _texture)
		: PhysicEntity(physics->CreateRectangle(_x, _y, 50, 25))
		, texture(_texture)
	{
	}

	void Update() override
	{
		int x, y;
		body->GetPosition(x, y);

		Color tint = body->Contains(GetMouseX(), GetMouseY()) ? RED : WHITE;

		Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
		Rectangle dest = { (float)x, (float)y, (float)texture.width, (float)texture.height };
		Vector2 origin = { (float)texture.width / 2.0f, (float)texture.height / 2.0f };
		DrawTexturePro(texture, source, dest, origin, body->GetRotation() * RAD2DEG, tint);
	}

private:
	Texture2D texture;
};

class Chain : public PhysicEntity
{
public:
	static constexpr int points[24] = {
		-38, 80, -44, -54, -16, -60, -16, -17,
		19, -19, 19, -79, 61, -77, 57, 73,
		17, 78, 20, 16, -25, 13, -9, 72
	};

	Chain(ModulePhysics* physics, int _x, int _y)
		: PhysicEntity(physics->CreateChain(_x, _y, points, 24))
	{
	}

	void Update() override
	{
		// No texture: press F1 to see it
	}
};

class Rick : public PhysicEntity
{
public:
	static constexpr int rick_head[64] = {
		14, 36, 42, 40, 40, 0, 75, 30,
		88, 4, 94, 39, 111, 36, 104, 58,
		107, 62, 117, 67, 109, 73, 110, 85,
		106, 91, 109, 99, 103, 104, 100, 115,
		106, 121, 103, 125, 98, 126, 95, 137,
		83, 147, 67, 147, 53, 140, 46, 132,
		34, 136, 38, 126, 23, 123, 30, 114,
		10, 102, 29, 90, 0, 75, 30, 62
	};

	Rick(ModulePhysics* physics, int _x, int _y, Texture2D _texture)
		: PhysicEntity(physics->CreateChain(_x, _y, rick_head, 64))
		, texture(_texture)
	{
	}

	void Update() override
	{
		int x, y;
		body->GetPosition(x, y);

		Color tint = body->Contains(GetMouseX(), GetMouseY()) ? RED : WHITE;

		DrawTextureEx(texture, Vector2{ (float)x, (float)y }, body->GetRotation() * RAD2DEG, 1.0f, tint);
	}

private:
	Texture2D texture;
};

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	ray_on = false;
	ray.x = ray.y = 0;
	sensor = NULL;
}

ModuleGame::~ModuleGame()
{
}

bool ModuleGame::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	circle = LoadTexture("Assets/ball.png");
	box = LoadTexture("Assets/crate.png");
	rick = LoadTexture("Assets/rick_head.png");

	bonus_fx = App->audio->LoadFx("Assets/bonus.wav");

	sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, SCREEN_HEIGHT, SCREEN_WIDTH / 2, 25);
	sensor->listener = this;

	//Alejandro hoy, add textures for the table background
	texBackground = LoadTexture("Assets/background.png");
	texTable = LoadTexture("Assets/table.png");
	SetTextureFilter(texBackground, TEXTURE_FILTER_POINT); //nearest filter (no bilinear), so the pixels stay sharp
	SetTextureFilter(texTable, TEXTURE_FILTER_POINT);

	buildWalls();
	//End of alejandro hoy


	return ret;
}

bool ModuleGame::CleanUp()
{
	LOG("Unloading Intro scene");

	DestroyAllEntities();
	//Alejandro Hoy
	for (PhysBody* current : walls) { App->physics->DestroyBody(current); }
	walls.clear();
	//Alejandro Hoy End
	App->physics->DestroyBody(sensor);
	sensor = NULL;

	UnloadTexture(circle);
	UnloadTexture(box);
	UnloadTexture(rick);
	UnloadTexture(texBackground);
	UnloadTexture(texTable);

	return true;
}



//Alejandro Hoy

void ModuleGame::buildWalls() { //outer shell, launch lane wall and right slope
	//every node of the outer wall, x and y in table coordinates, going from the lane bottom up, over the arc and down the left side
	const int shell[] = {
		490, 700, 
		490, 250,
		489, 229,
		486, 208,
		481, 187,
		475, 167,
		467, 148,
		457, 130,
		446, 112,
		433, 95,
		419, 80,
		404, 66,
		387, 53,
		370, 42,
		351, 32,
		332, 24,
		312, 18,
		291, 13,
		270, 10,
		249, 10, 
		229, 10,
		208, 13,
		187, 18,
		167, 24,
		148, 32,
		129, 42,
		112, 53,
		95, 66,
		80, 80,
		66, 95,
		53, 112,
		42, 129,
		32, 148,
		24, 167,
		18, 187,
		13, 208,
		10, 229,
		10, 250,
		10, 520,
		126, 591
	};
	int shellSize = sizeof(shell) / sizeof(int); //number of int vars, 80
	walls.push_back(App->physics->CreateChain(tableX, 0, shell, shellSize, false, wallRestitution, 0.1f));

	//wall between the launching spring and pinball table
	const int laneWall[] = { 455, 700, 455, 200 };
	walls.push_back(App->physics->CreateChain(tableX, 0, laneWall, 4, false, wallRestitution, 0.1f));

	//curve to the right flipper
	const int rightGuide[] = { 455, 520, 338, 591 };
	walls.push_back(App->physics->CreateChain(tableX, 0, rightGuide, 4, false, wallRestitution, 0.1f));
}


void ModuleGame::drawMap() { //the images are half size, so the destination is double
	Rectangle backgroundSource = { 0.0f, 0.0f, (float)texBackground.width, (float)texBackground.height };
	Rectangle backgroundDest = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
	DrawTexturePro(texBackground, backgroundSource, backgroundDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

	Rectangle tableSource = { 0.0f, 0.0f, (float)texTable.width, (float)texTable.height };
	Rectangle tableDest = { (float)tableX, 0.0f, (float)tableWidth, (float)tableHeight };
	DrawTexturePro(texTable, tableSource, tableDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
}
//Alejandro Hoy END
update_status ModuleGame::Update()
{
	//Alejandro Hoy
	drawMap(); 
	
	//Alejandro Hoy END

	for (PhysBody* body : bodies_to_destroy)
	{
		for (PhysicEntity* entity : entities)
		{
			if (entity->body == body)
			{
				DestroyEntity(entity);
				break;
			}
		}
	}
	bodies_to_destroy.clear();

	if (IsKeyPressed(KEY_SPACE))
	{
		ray_on = !ray_on;
		ray.x = GetMouseX();
		ray.y = GetMouseY();
	}

	if (IsKeyPressed(KEY_ONE))
	{
		Circle* c = new Circle(App->physics, GetMouseX(), GetMouseY(), circle);
		c->body->listener = this;
		entities.push_back(c);
	}

	if (IsKeyPressed(KEY_TWO))
		entities.push_back(new Box(App->physics, GetMouseX(), GetMouseY(), box));

	if (IsKeyPressed(KEY_THREE))
		entities.push_back(new Chain(App->physics, GetMouseX(), GetMouseY()));

	if (IsKeyPressed(KEY_FOUR))
		entities.push_back(new Rick(App->physics, GetMouseX(), GetMouseY(), rick));

	if (IsKeyPressed(KEY_BACKSPACE))
		DestroyAllEntities();

	vec2i mouse;
	mouse.x = GetMouseX();
	mouse.y = GetMouseY();
	int ray_hit = ray.DistanceTo(mouse);

	vec2f normal(0.0f, 0.0f);

	for (PhysicEntity* entity : entities)
	{
		entity->Update();

		if (ray_on)
		{
			vec2f hit_normal(0.0f, 0.0f);
			int hit = entity->body->RayCast(ray.x, ray.y, mouse.x, mouse.y, hit_normal.x, hit_normal.y);
			if (hit >= 0 && hit < ray_hit)
			{
				ray_hit = hit;
				normal = hit_normal;
			}
		}
	}

	if (ray_on == true)
	{
		vec2f destination((float)(mouse.x - ray.x), (float)(mouse.y - ray.y));
		destination.Normalize();
		destination *= (float)ray_hit;

		DrawLine(ray.x, ray.y, (int)(ray.x + destination.x), (int)(ray.y + destination.y), RED);

		if (normal.x != 0.0f || normal.y != 0.0f)
		{
			DrawLine((int)(ray.x + destination.x), (int)(ray.y + destination.y), (int)(ray.x + destination.x + normal.x * 25.0f), (int)(ray.y + destination.y + normal.y * 25.0f), Color{ 100, 255, 100, 255 });
		}
	}

	return UPDATE_CONTINUE;
}

void ModuleGame::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	if (bodyA == sensor)
	{
		if (bodyB != NULL && std::find(bodies_to_destroy.begin(), bodies_to_destroy.end(), bodyB) == bodies_to_destroy.end())
			bodies_to_destroy.push_back(bodyB);
		return;
	}

	if (bodyB == sensor)
		return;

	App->audio->PlayFx(bonus_fx);
}

void ModuleGame::DestroyEntity(PhysicEntity* entity)
{
	entities.erase(std::find(entities.begin(), entities.end(), entity));
	App->physics->DestroyBody(entity->body);
	delete entity;
}

void ModuleGame::DestroyAllEntities()
{
	for (PhysicEntity* entity : entities)
	{
		App->physics->DestroyBody(entity->body);
		delete entity;
	}
	entities.clear();
	bodies_to_destroy.clear();
}

#endif // PLANTILLA