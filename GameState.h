#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>

#include "stbsrisrates.h"

#define STATE_HISTORY 256

struct Bullet;
struct GameState;
struct Particle;

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
		: x(0)
		, y(0)
		, fire(false)
		, angle(0.0f)
		, pause(false)
	{}

	float x;
	float y;
	bool fire;
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
		SHIP
	};

	struct Reference
	{
		Reference() = default;
		Reference(Type t, std::int32_t i) : type(t), id(i) {}

		Type type;
		std::int32_t id;
	};
};

#define PLAYER_WIDTH 30
#define PLAYER_HEIGHT 30
#define PLAYER_SPEEDUP 1
#define PLAYER_MAX_SPEED 4
#define PLAYER_TIMER_FIRE 4
#define PLAYER_TIMER_IDLE 210
struct Player : Entity
{
	Player(int);

	void step(bool, const Controls&, GameState&, float delta, mersenne&);
	bool diff(const Player&) const;

	int id;
	bool shooting;
	float health;
	float timer_fire;
	float timer_idle;
	float percent_repair;
	int repairing_id;
};

struct Asteroid : Entity
{
	Asteroid(AsteroidType, mersenne&, const Asteroid*, int = ++last_id);

	bool diff(const Asteroid&) const;

	static void step(bool, GameState&, std::vector<Particle>*, mersenne&, float);
	static AsteroidType next(AsteroidType);

	static int last_id;
	static int size(AsteroidType);
	static int durability(AsteroidType);
	static int score(AsteroidType);

	AsteroidType type;
	int id;
	int health;
	float rotv;
};

#define BULLET_SIZE 5
#define BULLET_SPEED 20
#define BULLET_TTL 200
struct Bullet : Entity
{
	Bullet(int, int, float);

	static void step(bool, GameState&, std::vector<Particle>*, mersenne&);

	float ttl;
};

#define SHIP_WIDTH 90
#define SHIP_HEIGHT 77
#define SHIP_SPEED 1
struct Ship : Entity
{
	Ship(mersenne&, int = ++last_id);

	static void step(bool step, GameState&, std::vector<Particle>*, float, mersenne&);
	bool diff(const Ship&) const;
	static int last_id;

	int id;
	int health;
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

#define FIREWORK_SIZE 8
#define FIREWORK_TTL 60, 90
#define FIREWORK_SPEED 1.0, 2.4
#define FIREWORK_COUNT 13, 20
struct Firework : Entity
{
	struct Color
	{
		Color(mersenne &random)
		{
			const int c = random(0, 5);

			switch(c)
			{
				case 0:
					r = 240; g = b = 0; // red
					break;
				case 1:
					r = b = 0; g = 240; // green
					break;
				case 2:
					r = g = 0; b = 240; // blue
					break;
				case 3:
					r = g = 240; b = 0; // yellow
					break;
				case 4:
					r = b = 240; g = 0; // violet
					break;
				case 5:
					r = 0; g = b = 240; // cyan
					break;
			}
		}

		Color(const Color &rhs)
		{
			r = rhs.r;
			g = rhs.g;
			b = rhs.b;
		}

		int r, g, b;
	};

	Firework(float, float, const Color&, mersenne&);
	static void create(std::vector<Firework>&, float x, float y, mersenne&);
	static void step(std::vector<Firework>&, float);

	Color color;
	float ttl;
};

struct GameState
{
	GameState();
	GameState(const GameState&);
	void operator=(const GameState&) = delete;

	void reset();

	static GameState blank;
	std::vector<Asteroid> asteroid_list;
	std::vector<Bullet> bullet_list;
	std::vector<Player> player_list;
	std::vector<Ship> ship_list;
	unsigned stepno;
	int score;
	bool paused;
};

template <typename T> void compile_diff(const std::vector<T> &old_list, const std::vector<T> &new_list, std::vector<const Entity*> &delta)
{
	for(const T &ent_new : new_list)
	{
		bool found = false;

		for(const T &ent_old : old_list)
		{
			if(ent_new.id != ent_old.id)
				continue;

			found = true;
			if(ent_new.diff(ent_old))
				delta.push_back(&ent_new);
		}

		if(!found)
			delta.push_back(&ent_new);
	}
}

template <typename T> void compile_remove_diff(const Entity::Type ent_type, const std::vector<T> &old_list, const std::vector<T> &new_list, std::vector<Entity::Reference> &removed)
{
	for(const T &ent_old : old_list)
	{
		bool found = false;

		for(const T &ent_new : new_list)
		{
			if(ent_old.id == ent_new.id)
			{
				found = true;
				break;
			}
		}

		if(!found)
			removed.push_back({ent_type, ent_old.id});
	}
}

#endif // GAMESTATE_H
