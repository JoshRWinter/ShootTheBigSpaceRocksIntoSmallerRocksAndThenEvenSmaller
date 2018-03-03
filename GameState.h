#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <list>

#include "stbsrisrates.h"

#define STATE_HISTORY 256

enum class AsteroidType : std::uint8_t
{
	BIG = 1,
	MED = 2,
	SMALL = 3
};

struct Controls
{
	Controls()
		: left(false), right(false), up(false), down(false), fire(false), angle(0.0f), pause(false)
	{}

	bool left, right, up, down, fire;
	float angle;
	bool pause;
};

struct Entity
{
	Entity(float, float, float, float, float = 0.0f, float = 0.0f, float = 0.0f);
	bool collide(const Entity&, float = 0.0f) const;

	float x, y, w, h, rot, xv, yv;
};

#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 16
#define PLAYER_SPEEDUP 1
#define PLAYER_MAX_SPEED 4
struct Player : Entity
{
	Player(int);

	static void step_server(Player&, const Controls&);

	const int id;
	bool shooting;
	int health;
};

struct Asteroid : Entity
{
	Asteroid(AsteroidType);

	static int last_id;
	static int size(AsteroidType);

	AsteroidType type;
	int id;
	int health;
};

#define BULLET_SIZE 5
#define BULLET_SPEED 2
struct Bullet : Entity
{
	Bullet(int, int, float);

	float ttl;
};

struct GameState
{
	GameState() : stepno(0) {}

	const std::vector<int> &diff_players(const GameState&) const;

	static GameState blank;
	std::vector<Asteroid> asteroid_list;
	std::vector<Bullet> bullet_list;
	std::vector<Player> player_list;
	unsigned stepno;

private:
	static std::vector<int> diffs;
};

#endif // GAMESTATE_H
