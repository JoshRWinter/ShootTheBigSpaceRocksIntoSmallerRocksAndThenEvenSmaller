#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include <atomic>
#include <chrono>

#include "network.h"
#include "Lump.h"
#include "ServerState.h"

#define SERVER_PORT 28881

class Server
{
public:
	Server();
	~Server();

private:
	void wait();
	static void loop(Server*);

	ServerState state;

	std::atomic<bool> running;
	net::tcp_server tcp;
	net::udp_server udp;
	std::chrono::time_point<std::chrono::high_resolution_clock> last;
	std::thread background;
};

#endif // SERVER_H
