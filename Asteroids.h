#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <chrono>

#include "network.h"
#include "Lump.h"
#include "GameState.h"

class Asteroids
{
public:
	Asteroids(const std::string&, std::int32_t);
	const GameState &step();
	const std::vector<Particle> &get_particles() const;
	void input(const Controls&);
	void adjust_coords(float&, float&) const;

private:
	void recv();
	void integrate(const lmp::ServerInfo&);
	void integrate(const lmp::Player&);
	void integrate(const lmp::Asteroid&);
	void integrate(const lmp::Remove&);

	GameState state;
	std::vector<Particle> particle_list;
	mersenne random;

	const std::int32_t udp_secret;
	net::udp udp;
	std::uint32_t last_step;
	std::uint8_t my_id;
	std::chrono::time_point<std::chrono::high_resolution_clock> time_last_step;
	float delta;
};

#endif // ASTEROIDS_H
