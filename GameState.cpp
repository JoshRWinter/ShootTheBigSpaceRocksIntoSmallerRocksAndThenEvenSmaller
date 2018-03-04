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
				break;
			}
		}

		if(!found)
			delta.push_back(&player_current);
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
Asteroid::Asteroid(AsteroidType type)
	: Entity(0, 0, size(type), size(type))
	, id(++last_id)
	, health(100)
{}

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

void Bullet::step_server(std::vector<Bullet> &bullet_list, std::vector<Asteroid> &)
{
	step_client(bullet_list);
}

void Bullet::step_client(std::vector<Bullet> &bullet_list)
{
	for(auto it = bullet_list.begin(); it != bullet_list.end();)
	{
		Bullet &bullet = *it;

		bullet.x += bullet.xv;
		bullet.y += bullet.yv;

		if(--bullet.ttl == 0)
		{
			bullet_list.erase(it);
			continue;
		}

		++it;
	}
}
