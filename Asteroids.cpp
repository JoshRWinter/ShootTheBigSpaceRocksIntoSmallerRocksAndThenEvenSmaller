#include "Asteroids.h"

Asteroids::Asteroids(const std::string &addr, std::int32_t sec)
	: udp_secret(sec)
	, udp(addr, SERVER_PORT)
	, last_step(0)
{
	if(!udp)
		throw std::runtime_error("could not initialize udp socket");
}

const GameState &Asteroids::step()
{
	recv();

	// process players
	for(Player &player : state.player_list)
		player.step_client(state.bullet_list);

	// process boolets
	Bullet::step_client(state.bullet_list);

	return state;
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
		// log("doot " + std::to_string(time(NULL)));
		// pop ServerInfo
		const lmp::ServerInfo *const info = buffer.pop<lmp::ServerInfo>();
		if(info == NULL)
		{
			log("no server info present in net buffer");
			continue;
		}

		integrate(*info);

		// pop Player lumps
		const lmp::Player *player;
		while((player = buffer.pop<lmp::Player>()))
		{
			integrate(*player);
		}

		// pop remove lumps
		const lmp::Remove *removed;
		while((removed = buffer.pop<lmp::Remove>()))
		{
			integrate(*removed);
		}
	}
}

void Asteroids::integrate(const lmp::ServerInfo &info)
{
	last_step = info.stepno;
}

void Asteroids::integrate(const lmp::Player &lump)
{
	for(Player &player : state.player_list)
	{
		if(player.id != lump.id)
			continue;

		player.x = lump.x;
		player.y = lump.y;
		player.xv = lump.xv;
		player.yv = lump.yv;
		player.rot = lump.rot;
		player.shooting = lump.shooting;
		player.health = lump.health;

		return;
	}

	// not in the list
	state.player_list.push_back(lump.id);
	integrate(lump);
}

void Asteroids::integrate(const lmp::Remove &lump)
{
	switch(lump.ref.type)
	{
		case Entity::Type::PLAYER:
		{
			// find it in player list
			for(auto it = state.player_list.begin(); it != state.player_list.end(); ++it)
			{
				if((*it).id == lump.ref.id)
				{
					state.player_list.erase(it);
					break;
				}
			}
		}

		default:
			break;
	}
}
