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
	return
	x + w > subject.x + tolerance &&
	x < subject.x + subject.w - tolerance &&
	y + h > subject.y + tolerance &&
	y < subject.y + subject.h - tolerance;
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

void Player::step_server(Player &player, const Controls &controls, std::vector<Bullet> &bullet_list)
{
	if(controls.left || controls.right)
	{
		if(controls.right)
			player.xv += PLAYER_SPEEDUP;
		if(controls.left)
			player.xv -= PLAYER_SPEEDUP;
	}
	else
	{
		zerof(&player.xv, PLAYER_SPEEDUP);
	}

	if(controls.up || controls.down)
	{
		if(controls.up)
			player.yv -= PLAYER_SPEEDUP;
		if(controls.down)
			player.yv += PLAYER_SPEEDUP;
	}
	else
	{
		zerof(&player.yv, PLAYER_SPEEDUP);
	}

	player.rot = controls.angle;

	// clamp
	if(player.xv > PLAYER_MAX_SPEED)
		player.xv = PLAYER_MAX_SPEED;
	else if(player.xv < -PLAYER_MAX_SPEED)
		player.xv = -PLAYER_MAX_SPEED;
	if(player.yv > PLAYER_MAX_SPEED)
		player.yv = PLAYER_MAX_SPEED;
	else if(player.yv < -PLAYER_MAX_SPEED)
		player.yv = -PLAYER_MAX_SPEED;

	player.x += player.xv;
	player.y += player.yv;

	// prevent player from leaving world boundaries
	if(player.x < WORLD_LEFT)
	{
		player.x = WORLD_LEFT;
		player.xv = 0.0f;
	}
	else if(player.x + PLAYER_WIDTH > WORLD_LEFT + WORLD_WIDTH)
	{
		player.x = WORLD_LEFT + WORLD_WIDTH - PLAYER_WIDTH;
		player.xv = 0.0f;
	}
	if(player.y < WORLD_TOP)
	{
		player.y = WORLD_TOP;
		player.yv = 0.0f;
	}
	else if(player.y + PLAYER_HEIGHT > WORLD_TOP + WORLD_HEIGHT)
	{
		player.y = WORLD_TOP + WORLD_HEIGHT - PLAYER_HEIGHT;
		player.yv = 0.0f;
	}

	player.step_client(bullet_list);
}

void Player::step_client(std::vector<Bullet> &bullet_list)
{
	if(timer_fire > 0)
		--timer_fire;

	if(shooting && timer_fire == 0)
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
	, health(100)
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

void Asteroid::step(bool server, std::vector<Asteroid> &asteroid_list, std::vector<Player> &player_list, mersenne &random)
{
	if(server && asteroid_list.size() == 0 && random(0, 100) == 1)
	{
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

		aster.x += aster.xv;
		aster.y += aster.yv;

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
			return 90;
		case AsteroidType::MED:
			return 40;
		case AsteroidType::SMALL:
			return 10;
		default:
			break;
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

			if(bullet.collide(aster, 10))
			{
				targetf(&aster.xv, 1, bullet.xv);
				targetf(&aster.yv, 1, bullet.yv);

				if(server)
				{
					aster.health -= 4;
					// maybe delete the asteroid
					if(aster.health < 1)
					{
						const AsteroidType t = Asteroid::next(aster.type);
						Asteroid newguy1(t, random, &aster);
						Asteroid newguy2(t, random, &aster);
						Asteroid newguy3(t, random, &aster);
						it2 = asteroid_list.erase(it2);
						if(t != AsteroidType::NONE)
						{
							asteroid_list.push_back(newguy1);
							asteroid_list.push_back(newguy2);
							asteroid_list.push_back(newguy3);
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
