#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#include "p2Point.h"

#include <math.h>

const float bumperKick = 17.0f; //speed in m/s the bumper always throws the ball at
const float bumperMinSpeed = 0.5f; //so we never divide by zero when the ball is almost still

ModulePhysics::ModulePhysics(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	world = NULL;
	ground = NULL;
	mouse_joint = NULL;
	flipper_joint = NULL;   // <-- nuevo JONATHAN
	debug = true;
	kicker_joint = NULL;
	//Friccion
	frictionBody = NULL;
	flipper_joint_right = NULL;
	bumperBody = NULL;
}

// Destructor
ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Start()
{
	LOG("Creating Physics 2D environment");

	

	// Assign the member variable (do NOT declare a local "b2World* world" here, it would shadow the member)
	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));

	world->SetContactListener(this);


	/*
	// Needed to create joints like the mouse joint
	b2BodyDef bd;
	ground = world->CreateBody(&bd);

	// Big circle in the middle of the screen
	int x = (int)(SCREEN_WIDTH / 2);
	int y = (int)(SCREEN_HEIGHT / 1.5f);
	int diameter = SCREEN_WIDTH / 2;


	/*
	// Homework: dynamic, so a joint can make it rotate
	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	b2Body* big_ball = world->CreateBody(&body);

	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(diameter) * 0.5f;

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	big_ball->CreateFixture(&fixture);

	// Homework: revolute joint to the ground at its center, with a motor to spin it
	b2RevoluteJointDef revolute;
	revolute.Initialize(ground, big_ball, big_ball->GetWorldCenter());
	revolute.enableMotor = true;
	revolute.maxMotorTorque = 100000.0f;
	revolute.motorSpeed = 0.25f * b2_pi; // rad/s
	world->CreateJoint(&revolute);
	*/







	//----------------------------------------------------------------------------------------------------------------------------------------
//Modification of Jonathan 18/10/26
//Creation of the diferents joints

	int x = (int)(SCREEN_WIDTH / 2);
	int y = (int)(SCREEN_HEIGHT / 1.5f);
	int diameter = SCREEN_WIDTH / 2;
	//Hinge Joints



	//Object 1

	float DisplaceFlipperX = PIXEL_TO_METERS(x-120);
	float DisplaceFlipperY = PIXEL_TO_METERS(y +120);

	//Body
	b2BodyDef B1;
	B1.type = b2_staticBody;
	B1.position.Set(DisplaceFlipperX, DisplaceFlipperY);
	b2Body* body1 = world->CreateBody(&B1);

	//Shape
	b2CircleShape S1;
	S1.m_radius = PIXEL_TO_METERS(1);

	//Fixture
	b2FixtureDef F1;
	F1.shape = &S1;
	body1->CreateFixture(&F1);


		//Object 2

	//Body
	b2BodyDef B2;
	B2.type = b2_dynamicBody;
	B2.position.Set(DisplaceFlipperX, DisplaceFlipperY);
	b2Body* body2 = world->CreateBody(&B2);

	//Shape
	b2PolygonShape S2;

	//Con cambio de offSet 
	S2.SetAsBox(PIXEL_TO_METERS(40), PIXEL_TO_METERS(8),
		b2Vec2(PIXEL_TO_METERS(40), 0), 0);

	//Fixture
	b2FixtureDef F2;
	F2.shape = &S2;
	F2.density = 1.0f;
	body2->CreateFixture(&F2);

		//Object 2 FLIPPER RIGHT

	//Pivot of the right flipper (static), mirrored from the left one
	float DisplaceFlipperRightX = PIXEL_TO_METERS(x + 85);

	b2BodyDef PivotRightDef;
	PivotRightDef.type = b2_staticBody;
	PivotRightDef.position.Set(DisplaceFlipperRightX, DisplaceFlipperY);
	b2Body* pivotRight = world->CreateBody(&PivotRightDef);

	b2CircleShape PivotRightShape;
	PivotRightShape.m_radius = PIXEL_TO_METERS(1);

	b2FixtureDef PivotRightFixture;
	PivotRightFixture.shape = &PivotRightShape;
	pivotRight->CreateFixture(&PivotRightFixture);

	//Body FLIPPER RIGHT
	b2BodyDef BodyRight;
	BodyRight.type = b2_dynamicBody;
	BodyRight.position.Set(DisplaceFlipperRightX, DisplaceFlipperY);
	b2Body* BodyRight2 = world->CreateBody(&BodyRight);

	//Shape FLIPPER RIGHT: positive size, the offset puts it on the left of the pivot
	b2PolygonShape ShapeRIGHT;
	ShapeRIGHT.SetAsBox(PIXEL_TO_METERS(40), PIXEL_TO_METERS(8),
		b2Vec2(PIXEL_TO_METERS(-40), 0), 0);

	//Fixture
	b2FixtureDef FixtureRight;
	FixtureRight.shape = &ShapeRIGHT;
	FixtureRight.density = 1.0f;
	BodyRight2->CreateFixture(&FixtureRight);

		// END HERE FLIPPER RIGHT


	//Objects for the Kicker
	//OBJ 3

	//Body
	b2BodyDef B3;
	B3.type = b2_staticBody;
	B3.position.Set(PIXEL_TO_METERS(x+400), PIXEL_TO_METERS(y+100));
	b2Body* body3 = world->CreateBody(&B3);

	//Shape
	b2PolygonShape S3;
	S3.SetAsBox(PIXEL_TO_METERS(50), PIXEL_TO_METERS(70));


	//Fixture
	b2FixtureDef F3;
	F3.shape = &S3;
	body3->CreateFixture(&F3);


	//OBJ 4

		//Body
	b2BodyDef B4;
	B4.type = b2_dynamicBody;
	B4.position.Set(PIXEL_TO_METERS(x+400), PIXEL_TO_METERS(y+100));
	b2Body* body4 = world->CreateBody(&B4);

	//Shape
	b2PolygonShape S4;
	S4.SetAsBox(PIXEL_TO_METERS(50), PIXEL_TO_METERS(50));


	//Fixture
	b2FixtureDef F4;
	F4.shape = &S4;
	F4.density = 1.0f;
	body4->CreateFixture(&F4);


		//Creacion de los Bumpers
	//Meterle Restituition en la Fixture Fuerzaaaaaaa!!!!!!!!!!! Tocho

	b2BodyDef BB;
	BB.type = b2_staticBody;
	BB.position.Set(PIXEL_TO_METERS(x -15), PIXEL_TO_METERS(y - 205));
	b2Body* bumper = world->CreateBody(&BB);

	//Shape
	b2CircleShape SB;
	SB.m_radius =(PIXEL_TO_METERS(30));


	//Fixture
	b2FixtureDef FB;
	FB.shape = &SB;
	FB.restitution = 1.0f; //base value, PreSolve changes it for each hit
	bumper->CreateFixture(&FB);
	bumperBody = bumper;



	//------------------------FRICTON BODY_____________



	b2BodyDef BF;
	BF.type = b2_staticBody;
	BF.position.Set(PIXEL_TO_METERS(x-600), PIXEL_TO_METERS(y - 150));
	b2Body* friction = world->CreateBody(&BF);

	//Shape
	b2PolygonShape SF;
	SF.SetAsBox(PIXEL_TO_METERS(50), PIXEL_TO_METERS(100));


	//Fixture
	b2FixtureDef FF;
	FF.shape = &SF;
	FF.restitution = 1.0f;//mas de 1 debuelve fuerza
	friction->CreateFixture(&FF);


	frictionBody = friction;




	//Joints
	//----------------------------------------------------------
	//RevoluteJoint
	
	//Conectamos los dos puntos
	b2RevoluteJointDef jointDef1;
	jointDef1.Initialize(body1, body2, body1->GetWorldCenter());

	//Seteamos lo de los angulos, aunque los angulos luego los definimos en el update con lo de los botones para que giren
	jointDef1.enableLimit = true;

	jointDef1.lowerAngle = -0.25f * b2_pi;   // -45° (arriba)
	jointDef1.upperAngle = 0.0f;             // 0° (reposo)


	//Implementamos el Torque, basicamente que tenga una reaccion al interactuar con onbjetos
	jointDef1.enableMotor = true;
	jointDef1.maxMotorTorque = 800.0f;
	jointDef1.motorSpeed = 800.0f;

	flipper_joint = (b2RevoluteJoint*)world->CreateJoint(&jointDef1);

	//---------------------------------------------------------

	//RIGHT JOINT

		//RevoluteJoint of the right flipper: mirror of the left one, so the limits are 0 to +45
	b2RevoluteJointDef jointDefRight;
	jointDefRight.Initialize(pivotRight, BodyRight2, pivotRight->GetWorldCenter());

	jointDefRight.enableLimit = true;
	jointDefRight.lowerAngle = 0.0f;            //rest
	jointDefRight.upperAngle = 0.25f * b2_pi;   //+45 (up)

	jointDefRight.enableMotor = true;
	jointDefRight.maxMotorTorque = 800.0f;
	jointDefRight.motorSpeed = 0.0f;

	flipper_joint_right = (b2RevoluteJoint*)world->CreateJoint(&jointDefRight);


	//-------------------------------------------------------------------


	//PrismaticJoint

	//Definicion del joint
	b2PrismaticJointDef jointDef2; b2Vec2 worldAxis(0.0f, 1.0f);
	jointDef2.Initialize(body3, body4, body3->GetWorldCenter(), worldAxis);

	//Seteamos los desplazamientos

	jointDef2.enableLimit = true;
	jointDef2.lowerTranslation = PIXEL_TO_METERS(-60);
	jointDef2.upperTranslation = PIXEL_TO_METERS(0);

	//Interaccion con objetos

	jointDef2.enableMotor = true;
	jointDef2.maxMotorForce = 800.0f;
	jointDef2.motorSpeed = 0.0f;

	  //Creacion del joint y guardado
	kicker_joint = (b2PrismaticJoint*)world->CreateJoint(&jointDef2);

//-----------------------------------------------------------------------------------------------------------------------------------



	return true;








}

update_status ModulePhysics::PreUpdate()
{



	//-------------------------------------------------------------------------
	//Mecanica de subida
	if (IsKeyDown(KEY_A))
	{
		flipper_joint->SetMotorSpeed(-7000.0f);
	}
	else flipper_joint->SetMotorSpeed(4.0f);

	//FLipper Right
	if (IsKeyDown(KEY_D))
	{
		flipper_joint_right->SetMotorSpeed(7000.0f); //positive turns the opposite way than the left one
	}
	else flipper_joint_right->SetMotorSpeed(-4.0f);

	//--------------------------------------------------------
	//Mecanica del Prismatic Joint
	if (IsKeyDown(KEY_W))
	{
		kicker_joint->SetMotorSpeed(-10.0f);
	}
	else kicker_joint->SetMotorSpeed(5.0f);



	//------------------------------
	//Friccion
	//Quiero que la area que rodea mi objeto sienta la relentización



	if (frictionBody != NULL)
	{
		float rad = PIXEL_TO_METERS(120);

		//Hacemos un bucle que recorra constantemento todos los cuerpoos que hay en este proyecto

		for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
		{

			if (b->GetType() != b2_dynamicBody)
				continue;

			b2Vec2 d = b->GetPosition() - frictionBody->GetPosition();

			if (d.Length() < rad)
				b->SetLinearVelocity(0.95f * b->GetLinearVelocity());


		}
	}

	//----------------------------

	world->Step(1.0f / 60.0f, 8, 3);

	return UPDATE_CONTINUE;
}

update_status ModulePhysics::PostUpdate()
{
	if (IsKeyPressed(KEY_F1))
	{
		debug = !debug;
	}

	if (!debug)
	{
		// The mouse joint only lives in debug mode
		if (mouse_joint != NULL)
		{
			world->DestroyJoint(mouse_joint);
			mouse_joint = NULL;
		}
		return UPDATE_CONTINUE;
	}

	b2Vec2 mouse_position(PIXEL_TO_METERS(GetMouseX()), PIXEL_TO_METERS(GetMouseY()));
	b2Body* body_clicked = NULL;

	// Bonus code: this will iterate all objects in the world and draw the circles
	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
	{
		for(b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
		{
			switch(f->GetType())
			{
				// Draw circles ------------------------------------------------
				case b2Shape::e_circle:
				{
					b2CircleShape* shape = (b2CircleShape*)f->GetShape();
					b2Vec2 pos = f->GetBody()->GetPosition();
					
					DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), (float)METERS_TO_PIXELS(shape->m_radius), Color{0, 0, 0, 128});

					// Radius line to see the rotation
					b2Vec2 edge = b->GetWorldPoint(b2Vec2(shape->m_radius, 0.0f));
					DrawLine(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), METERS_TO_PIXELS(edge.x), METERS_TO_PIXELS(edge.y), WHITE);
				}
				break;

				// Draw polygons ------------------------------------------------
				case b2Shape::e_polygon:
				{
					b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();
					int32 count = polygonShape->m_count;
					b2Vec2 prev, v;

					for(int32 i = 0; i < count; ++i)
					{
						v = b->GetWorldPoint(polygonShape->m_vertices[i]);
						if(i > 0)
							DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), RED);

						prev = v;
					}

					v = b->GetWorldPoint(polygonShape->m_vertices[0]);
					DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), RED);
				}
				break;

				// Draw chains contour -------------------------------------------
				case b2Shape::e_chain:
				{
					b2ChainShape* shape = (b2ChainShape*)f->GetShape();
					b2Vec2 prev, v;

					for(int32 i = 0; i < shape->m_count; ++i)
					{
						v = b->GetWorldPoint(shape->m_vertices[i]);
						if(i > 0)
							DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), GREEN);
						prev = v;
					}

					v = b->GetWorldPoint(shape->m_vertices[0]);
					DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), GREEN);
				}
				break;

				// Draw a single segment(edge) ----------------------------------
				case b2Shape::e_edge:
				{
					b2EdgeShape* shape = (b2EdgeShape*)f->GetShape();
					b2Vec2 v1, v2;

					v1 = b->GetWorldPoint(shape->m_vertex1);
					v2 = b->GetWorldPoint(shape->m_vertex2);
					DrawLine(METERS_TO_PIXELS(v1.x), METERS_TO_PIXELS(v1.y), METERS_TO_PIXELS(v2.x), METERS_TO_PIXELS(v2.y), BLUE);
				}
				break;
			}

			// TODO 1: If mouse button 1 is pressed, test if the current body contains mouse position
			// Only dynamic bodies can be dragged
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse_joint == NULL && body_clicked == NULL
				&& b->GetType() == b2_dynamicBody && f->TestPoint(mouse_position))
			{
				body_clicked = b;
			}
		}
	}

	// TODO 2: If a body was selected, create a mouse joint
	// using mouse_joint class property
	if (body_clicked != NULL)
	{
		b2MouseJointDef def;
		def.bodyA = ground;
		def.bodyB = body_clicked;
		def.target = mouse_position;
		def.maxForce = 100.0f * body_clicked->GetMass();
		b2LinearStiffness(def.stiffness, def.damping, 5.0f, 0.7f, def.bodyA, def.bodyB); // 5 Hz, damping ratio 0.7

		mouse_joint = (b2MouseJoint*)world->CreateJoint(&def);
		body_clicked->SetAwake(true);
	}

	// TODO 3: If the player keeps pressing the mouse button, update
	// target position and draw a red line between both anchor points
	if (mouse_joint != NULL && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		mouse_joint->SetTarget(mouse_position);

		b2Vec2 anchor_a = mouse_joint->GetAnchorA(); // the target
		b2Vec2 anchor_b = mouse_joint->GetAnchorB(); // the point of the body we clicked
		DrawLine(METERS_TO_PIXELS(anchor_a.x), METERS_TO_PIXELS(anchor_a.y), METERS_TO_PIXELS(anchor_b.x), METERS_TO_PIXELS(anchor_b.y), RED);
	}

	// TODO 4: If the player releases the mouse button, destroy the joint
	if (mouse_joint != NULL && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		world->DestroyJoint(mouse_joint);
		mouse_joint = NULL;
	}

	return UPDATE_CONTINUE;
}


// Called before quitting
bool ModulePhysics::CleanUp()
{
	LOG("Destroying physics world");

	// Delete the whole physics world! (this also frees every body, fixture and joint it owns)
	delete world;
	world = NULL;
	ground = NULL;
	mouse_joint = NULL;

	return true;
}

// PhysBody pointer as UserData: set it in the b2BodyDef BEFORE calling CreateBody()
PhysBody* ModulePhysics::CreateCircle(int x, int y, int radius)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(radius);

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	b->CreateFixture(&fixture);

	pbody->body = b;

	return pbody;
}

PhysBody* ModulePhysics::CreateRectangle(int x, int y, int half_width, int half_height)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(half_width), PIXEL_TO_METERS(half_height));

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	b->CreateFixture(&fixture);

	pbody->body = b;

	return pbody;
}

// Static box that detects overlaps but does not collide (a trigger)
PhysBody* ModulePhysics::CreateRectangleSensor(int x, int y, int half_width, int half_height)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_staticBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(half_width), PIXEL_TO_METERS(half_height));

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.isSensor = true;
	b->CreateFixture(&fixture);

	pbody->body = b;

	return pbody;
}

// size is the number of ints, not vertices
PhysBody* ModulePhysics::CreateChain(int x, int y, const int* points, int size, bool loop, float restitution, float friction)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);
	//Alejandro Hoy
	int count = size / 2;
	int segments = loop ? count : count - 1; //an open chain has one segment less
	//So we have hole for death

	for (int i = 0; i < segments; ++i) {
		int next = (i + 1) % count;
		b2Vec2 start(PIXEL_TO_METERS(points[i * 2]), PIXEL_TO_METERS(points[i * 2 + 1]));
		b2Vec2 end(PIXEL_TO_METERS(points[next * 2]), PIXEL_TO_METERS(points[next * 2 + 1]));

		b2EdgeShape edge;
		edge.SetTwoSided(start, end); //collides from both sides

		b2FixtureDef fixture;
		fixture.shape = &edge;
		fixture.restitution = restitution;
		fixture.friction = friction;
		b->CreateFixture(&fixture);
	}

	pbody->body = b;
	return pbody;
}//Alejandro Hoy End

void ModulePhysics::DestroyBody(PhysBody* pbody)
{
	if (pbody == NULL)
		return;

	// Box2D also destroys the joints of this body: forget the mouse joint
	if (mouse_joint != NULL && mouse_joint->GetBodyB() == pbody->body)
		mouse_joint = NULL;

	world->DestroyBody(pbody->body); // Box2D frees the body and its fixtures
	delete pbody; // we free our own wrapper
}

// Call the listeners that are not NULL
void ModulePhysics::BeginContact(b2Contact* contact)
{
	PhysBody* a = (PhysBody*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
	PhysBody* b = (PhysBody*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

	if (a != NULL && a->listener != NULL)
		a->listener->OnCollision(a, b);

	if (b != NULL && b->listener != NULL)
		b->listener->OnCollision(b, a);
}
//Alejandro Hoy
void ModulePhysics::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) { //sets the bounciness of this hit, restitution = kick / speed
	if (bumperBody == NULL) { return; }

	b2Body* bodyA = contact->GetFixtureA()->GetBody();
	b2Body* bodyB = contact->GetFixtureB()->GetBody();

	b2Body* other = NULL; //the object that hit the bumper
	if (bodyA == bumperBody) { other = bodyB; }
	else if (bodyB == bumperBody) { other = bodyA; }
	else { return; } //not a bumper hit
	
	float speed = other->GetLinearVelocity().Length();
	if (speed < bumperMinSpeed) { speed = bumperMinSpeed; }

	contact->SetRestitution(bumperKick / speed); //1 / speed * constant = constant / speed
	contact->SetRestitutionThreshold(0.0f); //Threshold for Box2d Restitution
}
//Alejandro Hoy End


// PhysBody ------------------------------------
void PhysBody::GetPosition(int& x, int& y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x);
	y = METERS_TO_PIXELS(pos.y);
}

float PhysBody::GetRotation() const
{
	return body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	// True if the point is inside ANY of the shapes of this body
	for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		if (fixture->GetShape()->TestPoint(body->GetTransform(), p))
			return true;
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const
{
	// No hit: return -1
	// Hit: fill normal_x and normal_y and return the distance between x1,y1 and the hit point
	int ret = -1;

	b2RayCastInput input;
	input.p1.Set(PIXEL_TO_METERS(x1), PIXEL_TO_METERS(y1));
	input.p2.Set(PIXEL_TO_METERS(x2), PIXEL_TO_METERS(y2));
	input.maxFraction = 1.0f;

	float closest = 1.0f;

	for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
	{
		// A chain has one child per edge
		for (int32 i = 0; i < fixture->GetShape()->GetChildCount(); ++i)
		{
			b2RayCastOutput output;
			if (fixture->GetShape()->RayCast(&output, input, body->GetTransform(), i) && output.fraction <= closest)
			{
				closest = output.fraction;

				float dx = (float)(x2 - x1);
				float dy = (float)(y2 - y1);
				ret = (int)(output.fraction * sqrtf(dx * dx + dy * dy));

				normal_x = output.normal.x;
				normal_y = output.normal.y;
			}
		}
	}

	return ret;
}
