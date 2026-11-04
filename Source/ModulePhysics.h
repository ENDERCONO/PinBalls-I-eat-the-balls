#pragma once

#include "Module.h"
#include "Globals.h"
#include <vector>
#include "box2d/box2d.h"

// Gravity in m/s^2. Screen Y grows DOWNWARDS and we convert meters <-> pixels without flipping Y,
// so a POSITIVE Y gravity makes bodies fall down on screen. That is why the world receives -GRAVITY_Y.
#define GRAVITY_X 0.0f
#define GRAVITY_Y -7.0f

// Units: Box2D works in meters (float), raylib draws in pixels (int)
#define PIXELS_PER_METER 50.0f // if touched change METER_PER_PIXEL too
#define METER_PER_PIXEL 0.02f  // this is 1 / PIXELS_PER_METER !

#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * (m)))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * (p))


//Estos valores del enum los vamos a usar para saber con que colisions
enum BodyType
{
	BODY_NONE,
	BODY_BALL,
	BODY_BUMPER,
	BODY_SLING,
	BODY_FLIPPER,
	BODY_KICKER,
	BODY_DRAIN,
	BODY_WALL
};

// Small class to return to other modules to track position and rotation of physics bodies
class PhysBody
{
public:
	PhysBody() : body(NULL), listener(NULL), type(BODY_NONE), points(0)
	{
	}

	void GetPosition(int& x, int& y) const;
	float GetRotation() const;
	bool Contains(int x, int y) const;
	int RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const;

public:
	b2Body* body;
	// Module that listens to the collisions of this body
	Module* listener;
	BodyType type;
	int points;
};

// Module --------------------------------------
class ModulePhysics : public Module, public b2ContactListener
{
public:
	ModulePhysics(Application* app, bool start_enabled = true);
	~ModulePhysics();

	bool Start();
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

	PhysBody* CreateCircle(int x, int y, int radius);
	PhysBody* CreateRectangle(int x, int y, int half_width, int half_height);
	PhysBody* CreateRectangleSensor(int x, int y, int half_width, int half_height);
	PhysBody* CreateChain(int x, int y, const int* points, int size, bool loop = true, float restitution = 0.0f, float friction = 0.2f);

	void DestroyBody(PhysBody* pbody);

	//Uni solucion encaontrada, ya que no nos apetece cambiar losb2Body a mano. 
	//Entonces lo que hacemos, es convertirlos en PhysBody, para que se detecten al colisionar
	PhysBody* WrapBody(b2Body* b, BodyType type);





	//funciones que usamos para cambiar el menu de fisicas


		// Ajustes que se cambian desde el menu de fisicas
	//cambio de gravedad
	void  SetGravityY(float gy);
	//se lee la gravedad actual
	float GetGravityY() const;
	//cambiuamos el rebote del cuerpo
	void  SetRestitution(PhysBody* pbody, float restitution);



	// N Devuelve el cuerpo fisico de cada flipper
	b2Body* GetLeftFlipperBody()  const { return flipper_joint ? flipper_joint->GetBodyB() : NULL; }
	b2Body* GetRightFlipperBody() const { return flipper_joint_right ? flipper_joint_right->GetBodyB() : NULL; }

	// N Devuelve todos los bumpers (para que ModuleGame dibuje un sprite encima de cada uno)
	const std::vector<b2Body*>& GetBumpers() const { return bumpers; }

	void BeginContact(b2Contact* contact) override;
	int bumperHits;//for countig points


	//N variables par los menus
	//control de si el menu esta abietro
	bool  paused = false;
	//fps
	int   targetFPS = 60;
	//velocidad a la que el bumper lanza la bola
	float bumperKick = 17.0f;


private:

	bool debug;
	b2World* world;

	// Static body with no fixtures, needed to create joints like the mouse joint
	b2Body* ground;
	b2MouseJoint* mouse_joint;

	//Creacion de los Joints para poderlos modificar
	b2RevoluteJoint* flipper_joint;
	b2RevoluteJoint* flipper_joint_right;
	b2PrismaticJoint* kicker_joint;
	//Trianfgulo
	b2Body* slingL;
	b2Body* slingR;
	//Intento de ojeto que relentiza
	//Creamos un objeto y luego ya le damos el valor de otro objeto
	b2Body* frictionBody;



	//N Lista de bumpers (antes habia uno solo, bumperBody)
	std::vector<b2Body*> bumpers;
	//crea un bumper estatico circular en pixeles y lo guarda en la lista
	b2Body* CreateBumper(int x, int y, int radius);
	//true si el cuerpo es uno de los bumpers
	bool IsBumper(b2Body* b) const;

	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;

	//Es la lista de los cuerpos, la usamos para poder despues eliminarlos al dejar de compilar
	std::vector<PhysBody*> table_bodies;
};