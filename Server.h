#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include <atomic>
#include <chrono>

#include "network.h"
#include "Lump.h"
#include "ServerState.h"

struct Client;

class Server
{
public:
	Server();
	~Server();

private:
	void wait();
	void accept();
	static void loop(Server*);

	ServerState state;
	std::vector<Client> client_list;

	mersenne random;
	std::atomic<bool> running;
	net::tcp_server tcp;
	net::udp_server udp;
	std::chrono::time_point<std::chrono::high_resolution_clock> last;
	std::thread background;
};

struct Client
{
	Player player(std::vector<Player> &list)
	{
		for(auto &p : list)
			if(p.id == id)
				return p;
	}

	static int last_id;;

	net::udp_id udpid;

	int32_t secret;
	int32_t id;
};

#endif // SERVER_H
