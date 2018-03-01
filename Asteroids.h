#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "network.h"
#include "Lump.h"

class Asteroids
{
public:
	Asteroids(const std::string&, std::int32_t);
	void send(bool, bool, bool, bool, bool, bool, float);

private:
	const std::int32_t udp_secret;
	net::udp udp;
	std::uint32_t last_step;
};

#endif // ASTEROIDS_H
