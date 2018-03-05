#include "GameState.h"
#include "Server.h"

// *********
// *********
// GAME STATE
// *********
// *********

GameState GameState::blank;
void GameState::diff_players(const GameState &old, std::vector<const Player*> &delta) const
{
	// determine what needs to be added or modified
	for(const Player &player_current : player_list)
	{
		// try to find it in old state
		bool found = false;
		for(const Player &player_old : old.player_list)
		{
			if(player_current.id != player_old.id)
				continue;

			found = true;
			// determine if the object has changed
			if(
				player_current.x != player_old.x ||
				player_current.y != player_old.y ||
				player_current.shooting != player_old.shooting ||
				player_current.health != player_old.health ||
				player_current.rot != player_old.rot
			)
			{
				delta.push_back(&player_current);
			}
			break;
		}

		if(!found)
			delta.push_back(&player_current);
	}
}

void GameState::diff_asteroids(const GameState &old, std::vector<const Asteroid*> &delta) const
{
	// find out what entities are different
	for(const Asteroid &aster_new : asteroid_list)
	{
		// try to find it in the old state
		bool found = false;
		for(const Asteroid &aster_old : old.asteroid_list)
		{
			if(aster_new.id != aster_old.id)
				continue;

			found = true;
			// determine if it has changed
			if(
				aster_new.xv != aster_old.xv ||
				aster_new.yv != aster_old.yv
			)
			{
				delta.push_back(&aster_new);
			}
			break;
		}

		if(!found)
			delta.push_back(&aster_new);
	}
}

void GameState::diff_removed(const GameState &old, std::vector<Entity::Reference> &removed) const
{
	// see what players were removed
	for(const Player &player_old : old.player_list)
	{
		bool found = false;

		for(const Player &player_new : player_list)
		{
			if(player_new.id == player_old.id)
			{
				found = true;
				break;
			}
		}

		if(!found)
			removed.push_back({Entity::Type::PLAYER, player_old.id});
	}

	// see what asteroids were removed
	for(const Asteroid &aster_old : old.asteroid_list)
	{
		bool found = false;

		for(const Asteroid &aster_new : asteroid_list)
		{
			if(aster_old.id == aster_new.id)
			{
				found = true;
				break;
			}
		}

		if(!found)
			removed.push_back({Entity::Type::ASTEROID, aster_old.id});
	}
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
	: Entity((WINDOW_WIDTH / 2) - (PLAYER_WIDTH / 2), (WINDOW_HEIGHT / 2) - (PLAYER_HEIGHT / 2), PLAYER_WIDTH, PLAYER_HEIGHT)
	, id(ident)
	, shooting(false)
	, health(100)
	, timer_fire(0)
{}

void Player::step(bool server, const Controls &controls, std::vector<Bullet> &bullet_list, float delta)
{
	if(server)
	{
		if(controls.left || controls.right)
		{
			if(controls.right)
				xv += PLAYER_SPEEDUP;
			if(controls.left)
				xv -= PLAYER_SPEEDUP;
		}
		else
		{
			zerof(&xv, PLAYER_SPEEDUP);
		}

		if(controls.up || controls.down)
		{
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

	if(shooting && timer_fire <= 0.0f)
	{
		bullet_list.push_back({int(x + (PLAYER_WIDTH / 2)), int(y + (PLAYER_HEIGHT / 2)), rot});
		timer_fire = PLAYER_TIMER_FIRE;
	}
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
	const float speedmod = random(1.0f, 3.0f);
	if(parent != NULL)
	{
		x = parent->x + (parent->w / 2) - (w / 2);
		y = parent->y + (parent->h / 2) - (h / 2);
	}
	else
	{
		x = random(WORLD_LEFT, WORLD_RIGHT);
		y = random(WORLD_TOP, WORLD_BOTTOM);
	}
	xv = cosf(rot) * speedmod;
	yv = sinf(rot) * speedmod;
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
				if(aster.collide(player, 20))
					player.health -= 1;
			}
		}

		const float mult = server ? 1.0f : delta;
		aster.x += aster.xv * mult;
		aster.y += aster.yv * mult;

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
			return 70;
		case AsteroidType::SMALL:
			return 40;
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
	, ttl(200)
{
	xv = cosf(rot) * BULLET_SPEED;
	yv = sinf(rot) * BULLET_SPEED;
}

void Bullet::step(bool server, std::vector<Bullet> &bullet_list, std::vector<Asteroid> &asteroid_list, mersenne &random)
{
	for(auto it = bullet_list.begin(); it != bullet_list.end();)
	{
		Bullet &bullet = *it;

		bullet.x += bullet.xv;
		bullet.y += bullet.yv;

		// check for collision with asteroids
		bool stop = false;
		for(auto it2 = asteroid_list.begin(); it2 != asteroid_list.end();)
		{
			Asteroid &aster = *it2;

			if(bullet.collide(aster))
			{
				if(server)
				{
					// align asteroid direction with bullet direction
					targetf(&aster.xv, 1.0 / (aster.w / 30.0f), bullet.xv);
					targetf(&aster.yv, 1.0 / (aster.h / 30.0f), bullet.yv);

					aster.health -= random(2, 6);
					// maybe delete the asteroid
					if(aster.health < 1)
					{
						const AsteroidType t = Asteroid::next(aster.type);
						Asteroid parent = aster;

						it2 = asteroid_list.erase(it2);

						if(t != AsteroidType::NONE)
						{
							for(int i = 0; i < 3; ++i)
								asteroid_list.push_back({t, random, &parent});
						}
					}
				}

				// delete the bullet
				it = bullet_list.erase(it);
				stop = true;
				break;
			}

			++it2;
		}

		if(stop)
			continue;

		if(--bullet.ttl == 0)
		{
			bullet_list.erase(it);
			continue;
		}

		++it;
	}
}
