#include "GameState.h"
#include "Server.h"

GameState GameState::blank;

void GameState::reset()
{
	for(Player &p : player_list)
	{
		p.health = 100;
		p.x = 0.0f;
		p.y = 0.0f;
	}

	asteroid_list.clear();
	bullet_list.clear();
	ship_list.clear();
	score = 0;
}

// *********
// *********
// BASE ENTITY
// *********
// *********

Entity::Entity(float X, float Y, float W, float H, float ROT, float XV, float YV)
	: x(X)
	, y(Y)
	, w(W)
	, h(H)
	, rot(ROT)
	, xv(XV)
	, yv(YV)
{}

bool Entity::collide(const Entity &subject, float tolerance) const
{
	const float center_x = x + (w / 2.0f);
	const float center_y = y + (h / 2.0f);
	const float subject_center_x = subject.x + (subject.w / 2.0f);
	const float subject_center_y = subject.y + (subject.h / 2.0f);

	const float radius = ((w + h) / 2.0f) / 2.0f;
	const float subject_radius = ((subject.w + subject.h) / 2.0f) / 2.0f;

	const float dist = sqrtf(powf(center_x - subject_center_x, 2) + powf(center_y - subject_center_y, 2));

	return dist < radius + subject_radius - tolerance;
}

// *********
// *********
// PLAYER
// *********
// *********

Player::Player(int ident)
	: Entity(0, 0, PLAYER_WIDTH, PLAYER_HEIGHT)
	, id(ident)
	, shooting(false)
	, health(100)
	, timer_fire(0)
	, timer_idle(0)
{}

void Player::step(bool server, const Controls &controls, std::vector<Bullet> &bullet_list, float delta, mersenne &random)
{
	if(server)
	{
		if(health > 0 && (controls.left || controls.right))
		{
			timer_idle = 0;

			if(controls.right)
				xv += PLAYER_SPEEDUP;
			if(controls.left)
				xv -= PLAYER_SPEEDUP;
		}
		else
		{
			zerof(&xv, PLAYER_SPEEDUP);
		}

		if(health > 0 && (controls.up || controls.down))
		{
			timer_idle = 0;

			if(controls.up)
				yv -= PLAYER_SPEEDUP;
			if(controls.down)
				yv += PLAYER_SPEEDUP;
		}
		else
		{
			zerof(&yv, PLAYER_SPEEDUP);
		}

		rot = controls.angle;

		// clamp
		if(xv > PLAYER_MAX_SPEED)
			xv = PLAYER_MAX_SPEED;
		else if(xv < -PLAYER_MAX_SPEED)
			xv = -PLAYER_MAX_SPEED;
		if(yv > PLAYER_MAX_SPEED)
			yv = PLAYER_MAX_SPEED;
		else if(yv < -PLAYER_MAX_SPEED)
			yv = -PLAYER_MAX_SPEED;

		x += xv * delta;
		y += yv * delta;

		// prevent player from leaving world boundaries
		if(x < WORLD_LEFT)
		{
			x = WORLD_LEFT;
			xv = 0.0f;
		}
		else if(x + PLAYER_WIDTH > WORLD_LEFT + WORLD_WIDTH)
		{
			x = WORLD_LEFT + WORLD_WIDTH - PLAYER_WIDTH;
			xv = 0.0f;
		}
		if(y < WORLD_TOP)
		{
			y = WORLD_TOP;
			yv = 0.0f;
		}
		else if(y + PLAYER_HEIGHT > WORLD_TOP + WORLD_HEIGHT)
		{
			y = WORLD_TOP + WORLD_HEIGHT - PLAYER_HEIGHT;
			yv = 0.0f;
		}
	}

	if(timer_fire > 0.0f)
		timer_fire -= delta;

	if(shooting && timer_fire <= 0.0f && health > 0)
	{
		timer_idle = 0;
		const float plusminus = 0.02f;
		bullet_list.push_back({int(x + (PLAYER_WIDTH / 2)), int(y + (PLAYER_HEIGHT / 2)), rot + random(-plusminus, plusminus)});
		timer_fire = PLAYER_TIMER_FIRE;
	}

	++timer_idle;

	if(server && timer_idle > PLAYER_TIMER_IDLE && health < 100)
	{
		health += 0.2f;
		if(health > 100)
			health = 100;
	}
}

bool Player::diff(const Player &p) const
{
	return
		shooting != p.shooting ||
		health != p.health ||
		x != p.x ||
		y != p.y ||
		rot != p.rot;
}

// *********
// *********
// Asteroids
// *********
// *********

int Asteroid::last_id = 0;
Asteroid::Asteroid(AsteroidType t, mersenne &random, const Asteroid *parent, int ident)
	: Entity(0, 0, size(t), size(t))
	, type(t)
	, id(ident)
	, health(durability(t))
{
	rot = random(0.0, 3.1415926 * 2);
	rotv = random(-0.03, 0.03);
	const float speedmod = random(1.0f, 3.0f);
	if(parent != NULL)
	{
		x = parent->x + (parent->w / 2) - (w / 2);
		y = parent->y + (parent->h / 2) - (h / 2);
		xv = parent->xv + random(-2.5, 2.5);
		yv = parent->yv + random(-2.5, 2.5);
	}
	else
	{
		x = random(WORLD_LEFT, WORLD_RIGHT);
		y = random(WORLD_TOP, WORLD_BOTTOM);
		xv = cosf(rot) * speedmod;
		yv = sinf(rot) * speedmod;
	}
}

void Asteroid::step(bool server, std::vector<Asteroid> &asteroid_list, std::vector<Player> &player_list, mersenne &random, float delta)
{
	if(server)
	{
		// figure out potential # of asteroids
		int potential = 0;
		for(const Asteroid &aster : asteroid_list)
		{
			if(aster.type == AsteroidType::BIG)
				potential += 9;
			else if(aster.type == AsteroidType::MED)
				potential += 3;
			else if(aster.type == AsteroidType::SMALL)
				potential += 1;
			else
				hcf("invalid asteroid type");
		}

		if(potential <= MAX_ASTEROIDS - 9)
			asteroid_list.push_back({AsteroidType::BIG, random, NULL});
	}

	for(Asteroid &aster : asteroid_list)
	{
		if(server)
		{
			// check for collisions with player
			for(Player &player : player_list)
			{
				if(player.health > 0 && aster.collide(player, 20))
					player.health -= 2;
			}
		}

		const float mult = server ? 1.0f : delta;
		aster.x += aster.xv * mult;
		aster.y += aster.yv * mult;
		aster.rot += aster.rotv * mult;

		if(aster.x < WORLD_LEFT)
		{
			aster.x = WORLD_LEFT;
			aster.xv = -aster.xv;
		}
		else if(aster.x + aster.w > WORLD_RIGHT)
		{
			aster.x = WORLD_RIGHT - aster.w;
			aster.xv = -aster.xv;
		}

		if(aster.y < WORLD_TOP)
		{
			aster.y = WORLD_TOP;
			aster.yv = -aster.yv;
		}
		else if(aster.y + aster.h > WORLD_BOTTOM)
		{
			aster.y = WORLD_BOTTOM - aster.h;
			aster.yv = -aster.yv;
		}
	}
}

bool Asteroid::diff(const Asteroid &a) const
{
	return
		xv != a.xv ||
		yv != a.yv;
}

AsteroidType Asteroid::next(AsteroidType t)
{
	switch(t)
	{
		case AsteroidType::BIG:
			return AsteroidType::MED;
		case AsteroidType::MED:
			return AsteroidType::SMALL;
		case AsteroidType::SMALL:
			return AsteroidType::NONE;
		default:
			break;
	}

	hcf("no further asteroid types");
}

int Asteroid::size(AsteroidType type)
{
	switch(type)
	{
		case AsteroidType::BIG:
			return 110;
		case AsteroidType::MED:
			return 77;
		case AsteroidType::SMALL:
			return 48;
		default:
			break;
	}

	hcf("invalid asteroid type");
}

int Asteroid::durability(AsteroidType type)
{
	switch(type)
	{
		case AsteroidType::BIG:
			return 100;
		case AsteroidType::MED:
			return 40;
		case AsteroidType::SMALL:
			return 20;
		default: break;
	}

	hcf("invalid asteroid type");
}

int Asteroid::score(AsteroidType type)
{
	switch(type)
	{
		case AsteroidType::BIG:
			return 5;
		case AsteroidType::MED:
			return 3;
		case AsteroidType::SMALL:
			return 1;
		default: break;
	}

	hcf("invalid asteroid type");
}

// *********
// *********
// BOOLETS
// *********
// *********

Bullet::Bullet(int X, int Y, float ROT)
	: Entity(X - (BULLET_SIZE / 2), Y - (BULLET_SIZE / 2), BULLET_SIZE, BULLET_SIZE, ROT)
	, ttl(BULLET_TTL)
{
	xv = cosf(rot) * BULLET_SPEED;
	yv = sinf(rot) * BULLET_SPEED;
}

void Bullet::step(bool server, GameState &state, std::vector<Particle> *particle_list, mersenne &random)
{
	for(auto it = state.bullet_list.begin(); it != state.bullet_list.end();)
	{
		Bullet &bullet = *it;

		bullet.x += bullet.xv;
		bullet.y += bullet.yv;

		// check for collision with world boundary
		bool remove = false;
		if(bullet.x > WORLD_RIGHT)
		{
			remove = true;
			if(!server)
				Particle::create(*particle_list, WORLD_RIGHT, bullet.y, 3, random);
		}
		else if(bullet.x < WORLD_LEFT)
		{
			remove = true;
			if(!server)
				Particle::create(*particle_list, WORLD_LEFT, bullet.y, 3, random);
		}
		if(bullet.y < WORLD_TOP)
		{
			remove = true;
			if(!server)
				Particle::create(*particle_list, bullet.x, WORLD_TOP, 3, random);
		}
		else if(bullet.y > WORLD_BOTTOM)
		{
			remove = true;
			if(!server)
				Particle::create(*particle_list, bullet.x, WORLD_BOTTOM, 3, random);
		}
		if(remove)
		{
			state.bullet_list.erase(it);
			continue;
		}

		// check for collision with asteroids
		bool stop = false;
		for(auto it2 = state.asteroid_list.begin(); it2 != state.asteroid_list.end();)
		{
			Asteroid &aster = *it2;

			if(bullet.collide(aster))
			{
				if(server)
				{
					// align asteroid direction with bullet direction
					targetf(&aster.xv, 1.0 / (aster.w / 30.0f), bullet.xv / 2.0);
					targetf(&aster.yv, 1.0 / (aster.h / 30.0f), bullet.yv / 2.0);

					aster.health -= random(4, 7);
					// maybe delete the asteroid
					if(aster.health < 1)
					{
						state.score += Asteroid::score(aster.type);
						const AsteroidType t = Asteroid::next(aster.type);
						Asteroid parent = aster;

						it2 = state.asteroid_list.erase(it2);

						if(t != AsteroidType::NONE)
						{
							for(int i = 0; i < 3; ++i)
								state.asteroid_list.push_back({t, random, &parent});
						}
					}
				}
				else{
					// generate particles
					Particle::create(*particle_list, bullet.x, bullet.y, 6, random);
				}

				// delete the bullet
				it = state.bullet_list.erase(it);
				stop = true;
				break;
			}

			++it2;
		}

		if(stop)
			continue;

		if(--bullet.ttl == 0)
		{
			state.bullet_list.erase(it);
			continue;
		}

		++it;
	}
}

// *********
// *********
// SHIPS
// *********
// *********
int Ship::last_id = 0;
Ship::Ship(mersenne &random, int ID)
	: Entity(0, 0, SHIP_WIDTH, SHIP_HEIGHT)
	, id(ID)
	, health(100)
	, ttl(100)
{
	const bool going_left = random(2);
	y = random(WORLD_TOP, WORLD_BOTTOM - SHIP_HEIGHT);

	if(going_left)
	{
		x = WORLD_RIGHT + 1000;
		xv = -SHIP_SPEED;
	}
	else
	{
		x = WORLD_LEFT - 1000;
		xv = SHIP_SPEED;
	}
}

void Ship::step(bool server, std::vector<Ship> &ship_list, const std::vector<Asteroid> &asteroid_list, std::vector<Particle> *particle_list, float delta, mersenne &random)
{
	if(server && random(800) && ship_list.size() == 0)
		ship_list.push_back({random});

	for(auto it = ship_list.begin(); it != ship_list.end();)
	{
		Ship &ship = *it;

		float xv = ship.xv;
		if(ship.x < WORLD_LEFT - (SHIP_WIDTH * 2) || ship.x > WORLD_RIGHT + SHIP_WIDTH)
			xv *= 5;

		if(ship.health > 0)
			ship.x += xv * delta;

		if(ship.health < 1 && ship.ttl <= 0.0f && server)
		{
			it = ship_list.erase(it);
			continue;
		}
		else if(ship.health < 1)
			ship.ttl -= delta;

		// check for collisions with asteroid
		if(ship.health > 0)
		{
			for(const Asteroid &aster : asteroid_list)
			{
				if(aster.collide(ship))
				{
					if(server)
						ship.health -= 2;
					else
						Particle::create(*particle_list, ship.x + (SHIP_WIDTH / 2), ship.y + (SHIP_HEIGHT / 2), 30, random);
				}
			}
		}

		if(server)
		{
			if(ship.xv > 0.0f && ship.x > WORLD_RIGHT + 1000)
			{
				it = ship_list.erase(it);
				continue;
			}
			else if(ship.xv < 0.0f && ship.x < WORLD_LEFT - 1000)
			{
				it = ship_list.erase(it);
				continue;
			}
		}

		++it;
	}
}

bool Ship::diff(const Ship &other) const
{
	return
		health != other.health ||
		xv != other.xv;
}

// *********
// *********
// PARTICLES
// *********
// *********

Particle::Particle(float x, float y, mersenne &random)
	: Entity(x, y, 0, 0)
	, ttl(random(PARTICLE_TTL))
{
	rot = random(0.0, 3.1415926 * 2.0);
	xv = cosf(rot) * random(PARTICLE_SPEED);
	yv = sinf(rot) * random(PARTICLE_SPEED);
}

void Particle::create(std::vector<Particle> &particle_list, float x, float y, int count, mersenne &random)
{
	for(int i = 0; i < count; ++i)
		particle_list.push_back({x, y, random});
}

void Particle::step(std::vector<Particle> &particle_list, float delta)
{
	for(auto it = particle_list.begin(); it != particle_list.end();)
	{
		Particle &particle = *it;

		particle.ttl -= delta;
		if(particle.ttl <= 0.0f)
		{
			it = particle_list.erase(it);
			continue;
		}

		particle.x += particle.xv * delta;
		particle.y += particle.yv * delta;

		++it;
	}
}
