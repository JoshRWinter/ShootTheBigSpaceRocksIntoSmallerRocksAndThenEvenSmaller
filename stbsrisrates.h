#ifndef STBSRISRATES_H
#define STBSRISRATES_H

#define SERVER_PORT 28881

#include <random>

#include "Log.h"

class mersenne
{
public:
	mersenne(int seed = time(NULL))
		: generator(seed) {}

	int operator()(int low, int high)
	{
		return std::uniform_int_distribution(low, high)(generator);
	}

	float operator()(float low, float high)
	{
		return std::uniform_real_distribution<float>(low, high)(generator);
	}

private:
	std::mt19937 generator;
};

#define MAX_PLAYERS 2
#define MAX_DATAGRAM_SIZE 500

#endif // STBSRISRATES_H
