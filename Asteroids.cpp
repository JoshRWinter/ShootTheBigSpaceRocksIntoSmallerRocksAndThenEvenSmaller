#include "Asteroids.h"

Asteroids::Asteroids(const std::string &addr, std::int32_t sec)
	: udp_secret(sec)
	, udp(addr, SERVER_PORT)
	, last_step(0)
{
	if(!udp)
		throw std::runtime_error("could not initialize udp socket");
}

void Asteroids::step()
{
	recv();
}

void Asteroids::input(const Controls &controls)
{
	lmp::netbuf net_buffer;

	lmp::ClientInfo info;
	info.secret = udp_secret;
	info.up = controls.up;
	info.down = controls.down;
	info.left = controls.left;
	info.right = controls.right;
	info.fire = controls.fire;
	info.pause = controls.pause;
	info.angle = controls.angle;
	info.stepno = last_step;

	net_buffer.push(info);

	udp.send(net_buffer.raw.data(), net_buffer.size);
}

void Asteroids::recv()
{
	lmp::netbuf buffer;

	while(lmp::netbuf::get(buffer, udp))
	{
		const lmp::ServerInfo *const info = buffer.pop<lmp::ServerInfo>();
		if(info == NULL)
		{
			log("no server info present in net buffer");
			continue;
		}

		integrate(*info);
	}
}

void Asteroids::integrate(const lmp::ServerInfo &info)
{
	last_step = info.stepno;
}
