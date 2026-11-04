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

ModuleGame::ModuleGame(Application* app, bool start_enabled) : Module(app, start_enabled) //constructor
{
	ray_on = false;
	ray.x = ray.y = 0;
	sensor = NULL;

	// N todas las variables para la puntuación
	state = MENU;
	returnState = MENU;
	ballsLeft = startingBalls;
	currentBall = NULL;

	score = 0;

	previousScore = 0;
	highScore = 0;

	//end of n
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


	// N Añadimosz  los sonidos a las variables
	fx_bumper = App->audio->LoadFx("Assets/bumper.wav");
	fx_sling = App->audio->LoadFx("Assets/slingshot.wav");
	fx_wall = App->audio->LoadFx("Assets/wall.wav");
	fx_flipper = App->audio->LoadFx("Assets/flipper.wav");
	fx_launch = App->audio->LoadFx("Assets/launch.wav");
	fx_gameover = App->audio->LoadFx("Assets/gameover.wav");







	sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, SCREEN_HEIGHT, SCREEN_WIDTH / 2, 25);
	sensor->listener = this;

	//Alejandro hoy, add textures for the table background
	texBackground = LoadTexture("Assets/background.png");
	texTable = LoadTexture("Assets/table.png");
	texKicker = LoadTexture("Assets/kicker.png"); //N agregando kicker para cargar textura
	SetTextureFilter(texBackground, TEXTURE_FILTER_POINT); //nearest filter (no bilinear), so the pixels stay sharp
	SetTextureFilter(texTable, TEXTURE_FILTER_POINT);
	SetTextureFilter(texKicker, TEXTURE_FILTER_POINT); //N kicker

	//N flippers
	texFlipper = LoadTexture("Assets/flipper.png");
	SetTextureFilter(texFlipper, TEXTURE_FILTER_POINT);

	buildWalls();
	//End of alejandro hoy

	//n
	spawnBall();



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
	UnloadTexture(texKicker);	//N kicker
	UnloadTexture(texFlipper);	//N flipper

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



	//Marcamos todas las paredes como paredes
	for (PhysBody* wall : walls) { wall->type = BODY_WALL; }







}

//n (spawn ball on start)
//n (spawn ball on start)
void ModuleGame::spawnBall() { //new ball in the launch lane, bottom right of the table
	if (state != PLAYING || currentBall != NULL) { return; } //only one ball at a time

	Circle* newBall = new Circle(App->physics, tableX + 472, 400, circle);
	newBall->body->type = BODY_BALL;
	newBall->body->listener = this;
	//newBall->body->body->SetLinearVelocity(b2Vec2(0.0f, -17.0f)); 
	entities.push_back(newBall);
	currentBall = newBall->body;
}

//Nueva bola, del bonus
void ModuleGame::spawnExtraBall()
{
	Circle* ball = new Circle(App->physics, tableX + 250, 100, circle);
	ball->body->type = BODY_BALL;       //asignamos tipo
	ball->body->listener = this;
	//ball->body->body->SetLinearVelocity(b2Vec2(1.5f, 0.0f));   
	entities.push_back(ball);
	extraBalls.push_back(ball->body);
}

// N Función para controlar el game over
void ModuleGame::ballLost() {
	currentBall = NULL;
	--ballsLeft;

	if (ballsLeft <= 0)
	{
		App->audio->PlayFx(fx_gameover);//hacemos sonar la musica
		//Assignamos el anterior puntaje al acual
		previousScore = score;
		//Miramos si el puntaje actual es mejor que el record
		if (score > highScore)
		{
			highScore = score;
		}
		gameInProgress = false;   //la partida ha terminado, Play empezara una nueva
		state = GAME_OVER;
		return;
	}
	spawnBall();
}

void ModuleGame::restart() { //start new game with 3 balls once again
	score = 0; //reset points
	bumperCombo = 0;//reseteamos el combo
	DestroyAllEntities();
	extraBalls.clear();
	state = PLAYING;
	gameInProgress = true;   //hay una partida en curso
	ballsLeft = startingBalls;
	spawnBall();
}




// N funcion para mostrar el texto en pantalla
void ModuleGame::drawScorePanel()
{
	int centerX = tableX / 2;
	//Definimos el rectangulo principal
	Rectangle panel = { 30.0f, 40.0f, (float)(tableX - 60), 420.0f };
	DrawRectangleRec(panel, Color{ 12, 10, 38, 235 });
	DrawRectangleLinesEx(panel, 4.0f, Color{ 90, 220, 255, 255 });

	const char* scoreLabel = "SCORE";
	DrawText(scoreLabel, centerX - MeasureText(scoreLabel, 30) / 2, 70, 30, Color{ 150, 170, 210, 255 });

	const char* scoreText = TextFormat("%06d", score);
	DrawText(scoreText, centerX - MeasureText(scoreText, 60) / 2, 110, 60, Color{ 255, 230, 90, 255 });

	const char* ballsLabel = "BALLS";
	DrawText(ballsLabel, centerX - MeasureText(ballsLabel, 30) / 2, 200, 30, Color{ 150, 170, 210, 255 });

	//Dibujado de bolas en el menu

	for (int i = 0; i < startingBalls; ++i) {
		int circleX = centerX + (i - 1) * 50;
		if (i < ballsLeft) { DrawCircle(circleX, 260, 14.0f, WHITE); }
		else { DrawCircleLines(circleX, 260, 14.0f, Color{ 90, 100, 140, 255 }); }
	}


	for (int i = 0; i < maxExtraBalls; ++i) {
		int extraX = centerX + (int)((i - (maxExtraBalls - 1) / 2.0f) * 24.0f);
		if ((size_t)i < extraBalls.size()) { DrawCircle(extraX, 287, 7.0f, Color{ 90, 220, 255, 255 }); }
		else { DrawCircleLines(extraX, 287, 7.0f, Color{ 90, 100, 140, 255 }); }
	}

	// N Mostrar el puntaje anterior y record

	const char* lastLabel = "LAST";
	DrawText(lastLabel, centerX - MeasureText(lastLabel, 30) / 2, 300, 30, Color{ 150, 170, 210, 255 });
	const char* lastText = TextFormat("%06d", previousScore);
	DrawText(lastText, centerX - MeasureText(lastText, 40) / 2, 335, 40, WHITE);

	const char* bestLabel = "BEST";
	DrawText(bestLabel, centerX - MeasureText(bestLabel, 30) / 2, 385, 30, Color{ 150, 170, 210, 255 });
	const char* bestText = TextFormat("%06d", MAX(highScore, score));
	DrawText(bestText, centerX - MeasureText(bestText, 40) / 2, 420, 40, Color{ 255, 230, 90, 255 });






}

void ModuleGame::drawGameOver() { //dark screen, GAME OVER and the TRY AGAIN button for pre-restart
	int centerX = SCREEN_WIDTH / 2;

	DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 175 }); //makes screen dark (175 as alpha for transparency)

	const char* title = "GAME OVER";
	DrawText(title, centerX - MeasureText(title, gameOverTitleSize) / 2, gameOverTitleY, gameOverTitleSize, Color{ 255, 70, 90, 255 });

	Rectangle button = { (float)(centerX - buttonWidth / 2), (float)buttonY, (float)buttonWidth, (float)buttonHeight };
	bool hover = CheckCollisionPointRec(GetMousePosition(), button);

	DrawRectangleRec(button, hover ? Color{ 255, 120, 210, 255 } : Color{ 22, 22, 22, 255 });
	DrawRectangleLinesEx(button, 4.0f, WHITE);

	const char* label = "TRY AGAIN";
	DrawText(label, centerX - MeasureText(label, 40) / 2, buttonY + 20, 40, WHITE);

	if ((hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ENTER)) { restart(); }
}
//end of "n"

//N flippers (se tienen que dibujar)
void ModuleGame::drawFlipper(b2Body* body, bool mirrored)
{
	//Si el cuerpo no existe
	if (body == NULL) { return; }

	//Posiciones de metros a pixeles
	b2Vec2 pos = body->GetPosition();
	float x = pos.x * PIXELS_PER_METER;
	float y = pos.y * PIXELS_PER_METER;

	//Recortar imagen (ya que hay un borde transparente)
	float src_x = 1.0f, src_y = 1.0f, src_w = 51.0f, src_h = 14.0f;

	//Hacemos que el sprite se estire hasta el tamaño de la colision
	float dest_w = 86.0f, dest_h = 18.0f;
	float pivot_x = 6.0f, pivot_y = 9.0f;

	//Recortamos e invertimos para el flipper derecho
	Rectangle source = { src_x, src_y, mirrored ? -src_w : src_w, src_h };
	Rectangle dest = { x, y, dest_w, dest_h };
	Vector2 origin = { mirrored ? (dest_w - pivot_x) : pivot_x, pivot_y };

	DrawTexturePro(texFlipper, source, dest, origin, body->GetAngle() * RAD2DEG, WHITE);
}

void ModuleGame::drawMap() { //the images are half size, so the destination is double
	Rectangle backgroundSource = { 0.0f, 0.0f, (float)texBackground.width, (float)texBackground.height };
	Rectangle backgroundDest = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
	DrawTexturePro(texBackground, backgroundSource, backgroundDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

	Rectangle tableSource = { 0.0f, 0.0f, (float)texTable.width, (float)texTable.height };
	Rectangle tableDest = { (float)tableX, 0.0f, (float)tableWidth, (float)tableHeight };
	DrawTexturePro(texTable, tableSource, tableDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

	//N kickers (los pongo aquí ya que es una posición fija)
	float kickerW = (float)texKicker.width;
	float kickerH = (float)texKicker.height;

	Rectangle kickerLeftDest = { (float)(tableX + 114), 341.0f, kickerW * 2.0f, kickerH * 2.0f };
	DrawTexturePro(texKicker, Rectangle{ 0.0f, 0.0f, kickerW, kickerH }, kickerLeftDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

	Rectangle kickerRightDest = { (float)(tableX + 308), 341.0f, kickerW * 2.0f, kickerH * 2.0f };
	DrawTexturePro(texKicker, Rectangle{ 0.0f, 0.0f, -kickerW, kickerH }, kickerRightDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

	//N flippers (llamar a la funcion y dibujar flippers)
	drawFlipper(App->physics->GetLeftFlipperBody(), false);
	drawFlipper(App->physics->GetRightFlipperBody(), true);
}
//Alejandro Hoy END
update_status ModuleGame::Update()
{
	//Alejandro Hoy
	drawMap();

	//Alejandro Hoy END

	//other n
	drawScorePanel();



	//congelar fisicas si estamos en el estado MENU
	App->physics->paused = (state == MENU || state == SETTINGS);

	//ir dibujando
	if (state == MENU)
	{
		updateMenu();
		if (wantsExit) { return UPDATE_STOP; }   // salir del juego
		drawMenu();
		return UPDATE_CONTINUE;
	}
	if (state == SETTINGS) { updateSettings(); drawSettings(); return UPDATE_CONTINUE; }

	// Esc abre el menu principal sin perder la partida
	if (state == PLAYING && IsKeyPressed(KEY_ESCAPE))
	{
		menuIndex = 0;
		state = MENU;
		App->physics->paused = true;
		drawMenu();
		return UPDATE_CONTINUE;
	}

	// TAB sigue abriendo ajustes directamente (opcional, podeis quitarlo)
	if (state == PLAYING && IsKeyPressed(KEY_TAB)) { openSettings(PLAYING); }







	//end of other n

	//n ball dying and respawning update
	bool ballDied = false;
	for (PhysBody* body : bodies_to_destroy) {
		for (PhysicEntity* entity : entities) {
			if (entity->body == body) {
				if (body == currentBall) { ballDied = true; }
				else
				{
					std::vector<PhysBody*>::iterator it = std::find(extraBalls.begin(), extraBalls.end(), body);
					if (it != extraBalls.end()) { extraBalls.erase(it); }
				}
				DestroyEntity(entity);
				break;
			}
		}
	}
	bodies_to_destroy.clear();

	if (ballDied) { ballLost(); } //after the loop
	//end of n

	if (IsKeyPressed(KEY_SPACE))
	{
		ray_on = !ray_on;
		ray.x = GetMouseX();
		ray.y = GetMouseY();
	}

	if (IsKeyPressed(KEY_ONE))
	{
		Circle* c = new Circle(App->physics, GetMouseX(), GetMouseY(), circle);
		c->body->type = BODY_BALL;
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

	//n
	if (state == GAME_OVER) { drawGameOver(); }

	//points assign n
	if (state == PLAYING)
	{
		//Hacemos sonar los trremendos sonidos
		if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) { App->audio->PlayFx(fx_flipper); }
		if (IsKeyReleased(KEY_DOWN)) { App->audio->PlayFx(fx_launch); }




		int hits = App->physics->bumperHits;
		score += hits * bumperPoints;

		bumperCombo += hits;
		if (bumperCombo >= bumperComboTarget)
		{
			bumperCombo -= bumperComboTarget;
			if ((int)extraBalls.size() < maxExtraBalls) { spawnExtraBall(); }
			App->audio->PlayFx(bonus_fx);
		}

	}
	App->physics->bumperHits = 0;
	//end of n














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


	if (bodyA->type != BODY_BALL || bodyB == NULL)
		return;

	switch (bodyB->type)
	{
	case BODY_BUMPER: App->audio->PlayFx(fx_bumper); break;
	case BODY_SLING: App->audio->PlayFx(fx_sling); break;
	case BODY_WALL:

		if (bodyA->body->GetLinearVelocity().Length() > 3.0f)
			App->audio->PlayFx(fx_wall);
		break;
	default: break;
	}
}

void ModuleGame::DestroyEntity(PhysicEntity* entity)
{
	entities.erase(std::find(entities.begin(), entities.end(), entity));
	App->physics->DestroyBody(entity->body);
	delete entity;
}

void ModuleGame::DestroyAllEntities()
{
	currentBall = NULL;
	extraBalls.clear();
	for (PhysicEntity* entity : entities)
	{
		App->physics->DestroyBody(entity->body);
		delete entity;
	}
	entities.clear();
	bodies_to_destroy.clear();
}


void ModuleGame::openSettings(gameState from)
{
	returnState = from;    // guarda desde donde vienes
	settingsIndex = 0;     // empieza en la primera fila
	state = SETTINGS;
}

void ModuleGame::updateMenu()
{
	const int options = 3;   // PLAY/RESUME, PHYSICS SETTINGS, EXIT

	if (IsKeyPressed(KEY_DOWN)) { menuIndex = (menuIndex + 1) % options; }
	if (IsKeyPressed(KEY_UP)) { menuIndex = (menuIndex + options - 1) % options; }

	// Esc dentro del menu: volver a la partida si hay una en curso
	if (IsKeyPressed(KEY_ESCAPE) && gameInProgress)
	{
		state = PLAYING;
		return;
	}

	if (IsKeyPressed(KEY_ENTER))
	{
		if (menuIndex == 0)
		{
			if (gameInProgress) { state = PLAYING; }   // RESUME
			else { restart(); }                        // partida nueva
		}
		else if (menuIndex == 1) { openSettings(MENU); }
		else { wantsExit = true; }                     // EXIT
	}
}

void ModuleGame::updateSettings()
{
	const int rows = 5;   // FPS, gravedad, bumper, paredes y BACK

	if (IsKeyPressed(KEY_DOWN)) { settingsIndex = (settingsIndex + 1) % rows; }
	if (IsKeyPressed(KEY_UP)) { settingsIndex = (settingsIndex + rows - 1) % rows; }

	int dir = 0;   // +1 derecha, -1 izquierda, 0 nada
	if (IsKeyPressed(KEY_RIGHT)) { dir = 1; }
	if (IsKeyPressed(KEY_LEFT)) { dir = -1; }

	switch (settingsIndex)
	{
	case 0:   // FPS
		if (dir != 0)
		{
			App->physics->targetFPS = MAX(30, MIN(120, App->physics->targetFPS + dir * 15));
			SetTargetFPS(App->physics->targetFPS);
		}
		break;
	case 1:   // gravedad
		if (dir != 0) { App->physics->SetGravityY(MAX(1.0f, MIN(30.0f, App->physics->GetGravityY() + dir * 1.0f))); }
		break;
	case 2:   // fuerza del bumper
		if (dir != 0) { App->physics->bumperKick = MAX(5.0f, MIN(40.0f, App->physics->bumperKick + dir * 1.0f)); }
		break;
	case 3:   // rebote de paredes
		if (dir != 0)
		{
			wallBounce = MAX(0.0f, MIN(1.2f, wallBounce + dir * 0.1f));
			for (PhysBody* w : walls) { App->physics->SetRestitution(w, wallBounce); }
		}
		break;
	case 4:   // BACK
		if (IsKeyPressed(KEY_ENTER)) { state = returnState; }
		break;
	}

	// TAB o Esc: volver atras
	if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_ESCAPE)) { state = returnState; }
}

//Dibujado del menu

void ModuleGame::drawMenu()
{
	int centerX = SCREEN_WIDTH / 2;
	DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 200 });

	const char* title = "PINBALL";
	DrawText(title, centerX - MeasureText(title, 70) / 2, 150, 70, Color{ 255, 230, 90, 255 });

	const char* items[3] = { gameInProgress ? "RESUME" : "PLAY", "PHYSICS SETTINGS", "EXIT" };
	for (int i = 0; i < 3; ++i)
	{
		bool selected = (i == menuIndex);
		const char* text = selected ? TextFormat("> %s <", items[i]) : items[i];
		DrawText(text, centerX - MeasureText(text, 36) / 2, 320 + i * 70, 36,
			selected ? Color{ 255, 120, 210, 255 } : WHITE);
	}

	const char* hint = gameInProgress ? "UP/DOWN: move    ENTER: select    ESC: back to game"
		: "UP/DOWN: move    ENTER: select";
	DrawText(hint, centerX - MeasureText(hint, 20) / 2, SCREEN_HEIGHT - 80, 20, Color{ 150, 170, 210, 255 });
}




void ModuleGame::drawSettings()
{
	int centerX = SCREEN_WIDTH / 2;
	DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 215 });

	const char* title = "PHYSICS SETTINGS";
	DrawText(title, centerX - MeasureText(title, 44) / 2, 110, 44, Color{ 255, 230, 90, 255 });

	const char* labels[4] = { "TARGET FPS", "GRAVITY", "BUMPER KICK", "WALL BOUNCE" };
	char values[4][32];
	snprintf(values[0], 32, "%d", App->physics->targetFPS);
	snprintf(values[1], 32, "%.1f", App->physics->GetGravityY());
	snprintf(values[2], 32, "%.1f", App->physics->bumperKick);
	snprintf(values[3], 32, "%.1f", wallBounce);

	for (int i = 0; i < 4; ++i)
	{
		int y = 230 + i * 70;
		Color col = (i == settingsIndex) ? Color{ 255, 120, 210, 255 } : WHITE;
		DrawText(labels[i], centerX - 260, y, 30, col);

		const char* value = TextFormat("<  %s  >", values[i]);
		DrawText(value, centerX + 260 - MeasureText(value, 30), y, 30, col);
	}

	const char* back = (settingsIndex == 4) ? "> BACK <" : "BACK";
	DrawText(back, centerX - MeasureText(back, 30) / 2, 230 + 4 * 70 + 20, 30,
		(settingsIndex == 4) ? Color{ 255, 120, 210, 255 } : WHITE);

	const char* hint = "UP/DOWN: row    LEFT/RIGHT: change    ESC / TAB: back";
	DrawText(hint, centerX - MeasureText(hint, 20) / 2, SCREEN_HEIGHT - 80, 20, Color{ 150, 170, 210, 255 });
}