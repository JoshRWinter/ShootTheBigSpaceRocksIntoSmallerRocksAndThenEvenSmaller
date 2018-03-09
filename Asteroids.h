#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <chrono>
#include <queue>

#include <QWidget>

#include "network.h"
#include "Lump.h"
#include "GameState.h"

// #define NETWORK_METRICS

struct Announcement
{
	Announcement(const std::string &msg)
		: say(msg)
		, timer(300)
	{}

	const std::string say;
	int timer;
};

class Asteroids
{
public:
	Asteroids(const std::string&, std::int32_t);
	void step();
	void input(const Controls&);
	void adjust_coords(const QWidget*, float&, float&) const;
	const Player *me() const;

	std::queue<Announcement> announcements;
	float delta;

	GameState state;
	std::vector<Particle> particle_list;
	std::uint8_t my_id;
	unsigned score;
	unsigned repair;

private:
	mersenne random;
	const std::int32_t udp_secret;
	net::udp udp;
	std::uint32_t last_step;
	std::chrono::time_point<std::chrono::high_resolution_clock> time_last_step;

	void recv();
	void integrate(const lmp::ServerInfo&);
	void integrate(const lmp::Player&);
	void integrate(const lmp::Asteroid&);
	void integrate(const lmp::Ship&);
	void integrate(const lmp::Remove&);
};

#endif // ASTEROIDS_H
