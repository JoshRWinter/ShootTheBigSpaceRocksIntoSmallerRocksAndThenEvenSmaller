#include "Asteroids.h"

Asteroids::Asteroids(const std::string &addr, std::int32_t sec)
	: udp_secret(sec)
	, udp(addr, SERVER_PORT)
	, last_step(0)
	, my_id(0)
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
	Bullet::step(false, state.bullet_list, state.asteroid_list, random);

	// process asteroids
	Asteroid::step(false, state.asteroid_list, state.player_list, random);

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

void Asteroids::adjust_coords(float &x, float &y) const
{
	// find "me"
	const Player *me = NULL;
	for(const Player &p : state.player_list)
	{
		if(p.id == my_id)
		{
			me = &p;
			break;
		}
	}
	if(me == NULL)
		return;

	const float player_center_x = me->x + (PLAYER_WIDTH / 2);
	const float player_center_y = me->y + (PLAYER_HEIGHT / 2);

	x = (x - player_center_x) + (WINDOW_WIDTH / 2);
	y = (y - player_center_y) + (WINDOW_HEIGHT / 2);
}

void Asteroids::recv()
{
	lmp::netbuf buffer;

	while(lmp::netbuf::get(buffer, udp))
	{
		// log("doot size " + std::to_string(buffer.size) + " : " + std::to_string(time(NULL)));
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

		// pop asteroid lumps
		const lmp::Asteroid *asteroid;
		while((asteroid = buffer.pop<lmp::Asteroid>()))
		{
			integrate(*asteroid);
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
	my_id = info.my_id;
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

void Asteroids::integrate(const lmp::Asteroid &lump)
{
	for(Asteroid &aster : state.asteroid_list)
	{
		if(lump.id != aster.id)
			continue;

		aster.x = lump.x;
		aster.y = lump.y;
		aster.xv = lump.xv;
		aster.yv = lump.yv;

		return;
	}

	// not in the list
	state.asteroid_list.push_back({lump.aster_type, random, NULL, lump.id});
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

			break;
		}

		case Entity::Type::ASTEROID:
		{
			// find it in the asteroid list
			for(auto it = state.asteroid_list.begin(); it != state.asteroid_list.end(); ++it)
			{
				if((*it).id == lump.ref.id)
				{
					state.asteroid_list.erase(it);
					break;
				}
			}

			break;
		}

		default:
			hcf("invalide remove id " + std::to_string(int(lump.ref.type)));
			break;
	}
}
