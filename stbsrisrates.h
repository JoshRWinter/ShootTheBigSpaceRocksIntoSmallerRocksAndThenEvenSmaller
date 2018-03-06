#ifndef STBSRISRATES_H
#define STBSRISRATES_H

#define SERVER_PORT 28881

#include <random>

#include <time.h>

#include "Log.h"

#define MAX_PLAYERS 2
#define MAX_ASTEROIDS 36
#define MAX_DATAGRAM_SIZE 700

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define CLIENT_TIMEOUT 4
#define SERVER_TIMEOUT 10

#define WORLD_LEFT -400
#define WORLD_WIDTH 1000
#define WORLD_TOP -400
#define WORLD_HEIGHT 1000
#define WORLD_RIGHT (WORLD_LEFT + WORLD_WIDTH)
#define WORLD_BOTTOM (WORLD_TOP + WORLD_HEIGHT)

#define hcf(msg) {log(std::string("\033[35;1mFatal Error:\033[0m ") + msg);std::abort();}

class mersenne
{
public:
	mersenne(int seed = time(NULL))
		: generator(seed) {}

	int operator()(int low, int high)
	{
		return std::uniform_int_distribution<int>(low, high)(generator);
	}

	float operator()(double low, double high)
	{
		return std::uniform_real_distribution<double>(low, high)(generator);
	}

	bool operator()(int onein)
	{
		if(onein == 0)
			return false;

		return this->operator()(0, onein - 1) == 0;
	}

private:
	std::mt19937 generator;
};

inline void targetf(float *const subject, float step, float target)
{
	if(*subject > target)
	{
		*subject -= step;
		if(*subject < target)
			*subject = target;
	}
	else if(*subject < target)
	{
		*subject += step;
		if(*subject > target)
			*subject = target;
	}
}

inline void zerof(float *const subject, float step)
{
	targetf(subject, step, 0.0f);
}

#endif // STBSRISRATES_H
