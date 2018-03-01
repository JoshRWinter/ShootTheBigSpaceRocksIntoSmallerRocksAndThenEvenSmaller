#include "Asteroids.h"

Asteroids::Asteroids(const std::string &addr, std::int32_t sec)
	: udp_secret(sec)
	, udp(addr, SERVER_PORT)
	, last_step(0)
{
	log("setting to " + addr);
	if(!udp)
		throw std::runtime_error("could not initialize udp socket");
}

void Asteroids::send(bool up, bool down, bool left, bool right, bool fire, bool pause, float angle)
{
	lmp::ClientInfo info(net_buffer, 0);
	info.up = up;
	info.down = down;
	info.left = left;
	info.right = right;
	info.fire = fire;
	info.pause = pause;
	info.angle = angle;
	info.stepno = last_step;
	info.serialize();

	udp.send(net_buffer.data(), info.size);
}
