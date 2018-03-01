#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include <atomic>
#include <chrono>

#include "network.h"
#include "Lump.h"
#include "GameState.h"

struct Client;

class Server
{
public:
	Server();
	~Server();

private:
	void wait();
	void accept();
	void send();
	void recv();
	void compile_datagram(const Client&, lmp::netbuf&);
	void integrate_client(const Client&, const lmp::ClientInfo&);
	void step();
	static void loop(Server*);

	GameState state;
	std::vector<Client> client_list;

	mersenne random; // prng
	std::atomic<bool> running; // flag to tell server to exit

	net::tcp_server tcp;
	net::udp_server udp;

	std::chrono::time_point<std::chrono::high_resolution_clock> last; // time point of last simulation step
	std::thread background; // handle for service thread
};

struct Client
{
	Player player(std::vector<Player> &list)
	{
		for(auto &p : list)
			if(p.id == id)
				return p;
	}

	static Client *by_secret(std::int32_t s, std::vector<Client> &list)
	{
		for(auto &c : list)
			if(c.secret == s)
				return &c;

		return NULL;
	}

	static int last_id;;

	net::udp_id udpid;

	std::int32_t secret;
	std::int32_t id;
};

#endif // SERVER_H
