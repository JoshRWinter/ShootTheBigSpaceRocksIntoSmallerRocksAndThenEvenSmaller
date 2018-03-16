#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include <atomic>
#include <chrono>
#include <list>

#include "network.h"
#include "Lump.h"
#include "GameState.h"

struct Client;

#define TIMER_GAMEOVER 400
#define TIMER_WIN 700

class Server
{
public:
	Server();
	~Server();

private:
	void wait();
	void accept();
	void kick(const Client&, const std::string&);
	void send();
	void recv();
	void compile_datagram(const Client&, lmp::netbuf&);
	void integrate_client(Client&, const lmp::ClientInfo&);
	const GameState &get_hist_state(unsigned) const;
	void check_timeout();
	bool check_pause() const;
	bool check_win() const;
	void step();
	static void loop(Server*);

	const int max_score;

	GameState state;
	std::list<GameState> history;
	std::vector<Client> client_list;
	int gameover_timer, win_timer;

	mersenne random; // prng
	std::atomic<bool> running; // flag to tell server to exit

	net::tcp_server tcp;
	net::udp_server udp;

	std::chrono::time_point<std::chrono::high_resolution_clock> last; // time point of last simulation step
	std::thread background; // handle for service thread
};

struct Client
{
	Client(std::int32_t ident, std::int32_t sec)
	: stepno(0)
	, id(ident)
	, secret(sec)
	, paused(false)
	, last_datagram_time(0)
	{}

	Player &player(std::vector<Player> &list) const
	{
		for(auto &p : list)
			if(p.id == id)
				return p;

		hcf("could not id player %d", id);
	}

	static Client *by_secret(std::int32_t s, std::vector<Client> &list)
	{
		for(auto &c : list)
			if(c.secret == s)
				return &c;

		return NULL;
	}

	static int last_id;

	Controls controls;

	net::udp_id udpid;
	std::uint32_t stepno;
	std::int32_t id;
	std::int32_t secret;
	bool paused;
	int last_datagram_time;
};

#endif // SERVER_H
