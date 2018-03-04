#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "network.h"
#include "Lump.h"
#include "GameState.h"

class Asteroids
{
public:
	Asteroids(const std::string&, std::int32_t);
	const GameState &step();
	void input(const Controls&);

private:
	void recv();
	void integrate(const lmp::ServerInfo&);
	void integrate(const lmp::Player&);
	void integrate(const lmp::Remove&);

	GameState state;

	const std::int32_t udp_secret;
	net::udp udp;
	std::uint32_t last_step;
};

#endif // ASTEROIDS_H
