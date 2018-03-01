#include "GameState.h"

Entity::Entity(float X, float Y, float W, float H, float ROT, float XV, float YV)
	: x(X)
	, y(Y)
	, w(W)
	, h(H)
	, rot(ROT)
	, xv(XV)
	, yv(YV)
{}

bool Entity::collide(const Entity &subject, float tolerance) const
{
	return
	x + w > subject.x + tolerance &&
	x < subject.x + subject.w - tolerance &&
	y + h > subject.y + tolerance &&
	y < subject.y + subject.h - tolerance;
}

// *********
// *********
// PLAYER
// *********
// *********

Player::Player(int ident)
	: Entity(-PLAYER_WIDTH / 2, -PLAYER_HEIGHT / 2, PLAYER_WIDTH, PLAYER_HEIGHT)
	, id(ident)
	, health(100)
{}

// *********
// *********
// Asteroids
// *********
// *********

int Asteroid::last_id = 0;
Asteroid::Asteroid(AsteroidType type)
	: Entity(0, 0, size(type), size(type))
	, id(++last_id)
	, health(100)
{}

int Asteroid::size(AsteroidType type)
{
	switch(type)
	{
		case AsteroidType::BIG:
			return 90;
		case AsteroidType::MED:
			return 40;
		case AsteroidType::SMALL:
			return 10;
	}

	hcf("invalid asteroid type");
}

// *********
// *********
// BOOLETS
// *********
// *********

Bullet::Bullet(int X, int Y, float ROT)
	: Entity(X, Y, BULLET_SIZE, BULLET_SIZE, ROT)
	, ttl(200)
{
	xv = cosf(rot) * BULLET_SPEED;
	yv = sinf(rot) * BULLET_SPEED;
}
