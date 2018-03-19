#include "Asteroids.h"

Asteroids::Asteroids(const std::string &addr, std::int32_t sec)
	: delta(1.0f)
	, my_id(0)
	, score(0)
	, repair(0)
	, paused(false)
	, win(false)
	, time_last_datagram(time(NULL))
	, udp_secret(sec)
	, udp(addr, SERVER_PORT)
	, last_step(0)
	, time_last_step(std::chrono::high_resolution_clock::now())
{
	if(!udp)
		throw std::runtime_error("could not initialize udp socket");
}

void Asteroids::step()
{
	{ // timing related garbage
		const std::chrono::duration<long long, std::ratio<1, 1'000'000'000>> diff = std::chrono::high_resolution_clock::now() - time_last_step;
		const float normal_nano = 16666666.0f;
		delta = diff.count() / normal_nano;
		time_last_step = std::chrono::high_resolution_clock::now();
	}

	recv();

	if(paused)
		return;

	// process players
	for(Player &player : state.player_list)
		player.step(false, Controls(), state, delta, random);

	// process boolets
	Bullet::step(false, state, &particle_list, random);

	// process asteroids
	Asteroid::step(false, state, &particle_list, random, delta);

	// process ships
	Ship::step(false, state, &particle_list, delta, random);

	// process particles
	Particle::step(particle_list, delta);

	// process fireworks
	Firework::step(firework_list, delta);
	// generate fireworks
	if(win && random(18))
	{
		const float x = random(WORLD_LEFT, WORLD_RIGHT);
		const float y = random(WORLD_TOP, WORLD_BOTTOM);
		Firework::create(firework_list, x, y, random);
	}
}

void Asteroids::input(const Controls &controls)
{
	lmp::netbuf net_buffer;

	lmp::ClientInfo info;
	info.secret = udp_secret;
	info.x = controls.x;
	info.y = controls.y;
	info.fire = controls.fire;
	info.paused = controls.pause;
	info.angle = controls.angle;
	info.stepno = last_step;

	net_buffer.push(info);

	udp.send(net_buffer.raw.data(), net_buffer.size);
}

void Asteroids::adjust_coords(const QWidget *window, float &x, float &y) const
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

	x = (x - player_center_x) + (window->width() / 2);
	y = (y - player_center_y) + (window->height() / 2);
}

const Player *Asteroids::me() const
{
	for(const Player &p : state.player_list)
		if(p.id == my_id)
			return &p;

	return NULL;
}

bool Asteroids::timed_out() const
{
	return time(NULL) - time_last_datagram > SERVER_TIMEOUT;
}

void Asteroids::recv()
{
	lmp::netbuf buffer;

	while(lmp::netbuf::get(buffer, udp))
	{
#ifdef NETWORK_METRICS
		{
			static int bytes, last_second;
			const int cursec = time(NULL);
			if(cursec != last_second)
			{
				lprintf("%.3f kilobytes/sec", bytes / 1000.0);
				last_second = cursec;
				bytes = 0;
			}
			bytes += buffer.size;
		}
#endif // NETWORK_METRICS
		time_last_datagram = time(NULL);

		// pop ServerInfo
		const lmp::ServerInfo *const info = buffer.pop<lmp::ServerInfo>();
		if(info == NULL)
		{
			lprintf("no server info present in net buffer");
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

		// pop ship lumps
		const lmp::Ship *ship;
		while((ship = buffer.pop<lmp::Ship>()))
		{
			integrate(*ship);
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
	score = info.score;
	repair = info.repair;
	paused = info.paused == 1;
	win = info.win == 1;
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

void Asteroids::integrate(const lmp::Ship &lump)
{
	// find it in the list
	for(Ship &ship : state.ship_list)
	{
		if(ship.id != lump.id)
			continue;

		ship.x = lump.x;
		ship.y = lump.y;
		ship.xv = lump.xv;
		ship.yv = lump.yv;
		ship.health = lump.health;

		return;
	}

	state.ship_list.push_back({random, lump.id});
	integrate(lump);
	if(!win)
		announcements.push({"Protect the passenger cruiser!"});
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
					const Asteroid &aster = *it;

					Particle::create(particle_list, aster.x + (aster.w / 2), aster.y + (aster.h / 2), 40, random);
					state.asteroid_list.erase(it);
					break;
				}
			}

			break;
		}

		case Entity::Type::SHIP:
		{
			// find it in the ship list
			for(auto it = state.ship_list.begin(); it != state.ship_list.end(); ++it)
			{
				if((*it).id == lump.ref.id)
				{
					const Ship &ship = *it;

					if((*it).health < 1)
					{
						Particle::create(particle_list, ship.x + (SHIP_WIDTH / 2), ship.y + (SHIP_HEIGHT / 2), 120, random);
						if(!win)
							announcements.push({"The passenger cruiser was destroyed\nand all 1 billion billion passengers were killed!"});
					}
					else if(score != 0 && !win)
						announcements.push({"The passenger cruiser safely made it\nthrough the asteroid field!"});
					state.ship_list.erase(it);
					break;
				}
			}

			break;
		}

		default:
			hcf("invalide remove id %d", lump.ref.type);
			break;
	}
}
