#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>

#include "stbsrisrates.h"

#define STATE_HISTORY 256

struct Bullet;

enum class AsteroidType : std::uint8_t
{
	BIG = 1,
	MED = 2,
	SMALL = 3,
	NONE = 4
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

	enum class Type : std::int32_t
	{
		PLAYER,
		ASTEROID,
		BULLET
	};

	struct Reference
	{
		Reference() = default;
		Reference(Type t, std::int32_t i) : type(t), id(i) {}

		Type type;
		std::int32_t id;
	};
};

#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 16
#define PLAYER_SPEEDUP 1
#define PLAYER_MAX_SPEED 4
#define PLAYER_TIMER_FIRE 6
struct Player : Entity
{
	Player(int);

	void step(bool, const Controls&, std::vector<Bullet>&, float delta);

	int id;
	bool shooting;
	int health;
	float timer_fire;
};

struct Asteroid : Entity
{
	Asteroid(AsteroidType, mersenne&, const Asteroid*, int = ++last_id);

	static void step(bool, std::vector<Asteroid>&, std::vector<Player>&, mersenne&, float);
	static AsteroidType next(AsteroidType);

	static int last_id;
	static int size(AsteroidType);
	static int durability(AsteroidType);

	AsteroidType type;
	int id;
	int health;
};

struct Particle;
#define BULLET_SIZE 5
#define BULLET_SPEED 20
struct Bullet : Entity
{
	Bullet(int, int, float);

	static void step(bool, std::vector<Bullet>&, std::vector<Asteroid>&, std::vector<Particle>*, mersenne&);

	float ttl;
};

#define PARTICLE_SPEED 11.0, 15.0
#define PARTICLE_TTL 5, 7
struct Particle : Entity
{
	Particle(float, float, mersenne&);
	static void create(std::vector<Particle> &particle_list, float x, float y, int, mersenne&);
	static void step(std::vector<Particle>&, float);

	float ttl;
};

struct GameState
{
	GameState() : stepno(0) {}

	void diff_players(const GameState&, std::vector<const Player*>&) const;
	void diff_asteroids(const GameState&, std::vector<const Asteroid*>&) const;
	void diff_removed(const GameState&, std::vector<Entity::Reference>&) const;

	static GameState blank;
	std::vector<Asteroid> asteroid_list;
	std::vector<Bullet> bullet_list;
	std::vector<Player> player_list;
	unsigned stepno;
};

#endif // GAMESTATE_H
