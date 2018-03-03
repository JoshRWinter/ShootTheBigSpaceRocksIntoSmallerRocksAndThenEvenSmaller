#include "GameState.h"
#include "Server.h"

// *********
// *********
// GAME STATE
// *********
// *********

std::vector<int> GameState::diffs;
GameState GameState::blank;
const std::vector<int> &GameState::diff_players(const GameState &old) const
{
	diffs.clear();

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
				player_current.health != player_old.health
			)
			{
				diffs.push_back(player_current.id);
				break;
			}
		}

		if(!found)
			diffs.push_back(player_current.id);
	}

	return diffs;
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
{}

void Player::step_server(Player &player, const Controls &controls)
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
	: Entity(X, Y, BULLET_SIZE, BULLET_SIZE, ROT)
	, ttl(200)
{
	xv = cosf(rot) * BULLET_SPEED;
	yv = sinf(rot) * BULLET_SPEED;
}
