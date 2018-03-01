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
	lmp::netbuf net_buffer;

	lmp::ClientInfo info;
	info.up = up;
	info.down = down;
	info.left = left;
	info.right = right;
	info.fire = fire;
	info.pause = pause;
	info.angle = angle;
	info.stepno = last_step;

	net_buffer.push(info);

	udp.send(net_buffer.raw.data(), net_buffer.size);
}
