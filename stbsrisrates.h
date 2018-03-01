#ifndef STBSRISRATES_H
#define STBSRISRATES_H

#define SERVER_PORT 28881

#include <random>

#include "Log.h"

#define MAX_PLAYERS 2
#define MAX_DATAGRAM_SIZE 500

#define hcf(msg) {log(std::string("\033[35;1mFatal Error:\033[0m ") + msg);std::abort();}

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

#endif // STBSRISRATES_H
