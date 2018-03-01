#ifndef SERVERSTATE_H
#define SERVERSTATE_H

#include <vector>

#include "stbsrisrates.h"

enum class AsteroidType : std::uint8_t
{
	BIG = 1,
	MED = 2,
	SMALL = 3
};

struct Entity
{
	Entity() : x(0), y(0), w(0), h(0), xv(0), yv(0) {}
	float x, y, w, h, rot, xv, yv;
};

#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 16
struct Player : Entity
{
	Player(int ident)
		: id(ident)
		, health(100) {}

	int id;
	int health;
};

struct Asteroid : Entity
{
	Asteroid(AsteroidType t, int ident)
		: type(t)
		, id(ident) {}

	AsteroidType type;
	int id;
	int health;
};

#define BULLET_SIZE 5
struct Bullet : Entity
{
};

struct ServerState
{
	std::vector<Asteroid> asteroid_list;
	std::vector<Bullet> bullet_list;
	std::vector<Player> player_list;
};

#endif // SERVERSTATE_H
