#include <stdexcept>

#include "Server.h"

int Client::last_id = 0;

Server::Server()
	: gameover_timer(TIMER_GAMEOVER)
	, running(true)
	, tcp(SERVER_PORT)
	, udp(SERVER_PORT)
	, last(std::chrono::high_resolution_clock::now())
	, background(Server::loop, this)
{
	if(!tcp || !udp)
	{
		running = false;
		background.join();
		throw std::runtime_error("Could not bind to port " + std::to_string(SERVER_PORT));
	}
}

Server::~Server()
{
	running = false;
	background.join();
}

void Server::wait()
{
	std::chrono::duration<long long, std::nano> diff;
	std::chrono::time_point<std::chrono::high_resolution_clock> current;

	do
	{
		current = std::chrono::high_resolution_clock::now();
		diff = current - last;
#ifndef _WIN32
		// sorry windows, but your Sleep(...) sucks
		std::this_thread::sleep_for(std::chrono::microseconds(100));
#endif
	}while(diff.count() < 16666000);

	last = current;

	// frequency check
	{
		static int sps, second;
		const int current = time(NULL);
		if(current != second)
		{
			if(state.stepno > 200 && sps < 56)
				log("sps == %d -- having trouble keeping up", sps);
			sps = 0;
			second = current;
		}
		else
			++sps;
	}
}

void Server::accept()
{
	const int sock = tcp.accept();

	if(sock == -1)
		return;

	net::tcp stream(sock);
	const uint8_t accept_client = client_list.size() < MAX_PLAYERS;
	stream.send_block(&accept_client, sizeof(accept_client));

	if(!accept_client)
		return;

	Client client(++Client::last_id, random(0, 500'000'000));
	Player player(client.id);
	stream.send_block(&client.secret, sizeof(client.secret));

	client_list.push_back(client);
	state.player_list.push_back(player);
}

void Server::kick(const Client &client, const std::string &reason)
{
	for(auto it = state.player_list.begin(); it != state.player_list.end(); ++it)
	{
		if((*it).id == client.id)
		{
			state.player_list.erase(it);
			break;
		}
	}

	for(auto it = client_list.begin(); it != client_list.end(); ++it)
	{
		if((*it).id == client.id)
		{
			log("client %d kicked (%s)", client.id, reason.c_str());
			client_list.erase(it);
			break;
		}
	}
}

void Server::send()
{
	for(const Client &client : client_list)
	{
		if(!client.udpid.initialized)
			continue;

		lmp::netbuf buffer;
		compile_datagram(client, buffer);
		if(buffer.size == 0)
			continue;

		udp.send(buffer.raw.data(), buffer.size, client.udpid);
	}
}

void Server::recv()
{
	net::udp_id udpid;
	lmp::netbuf net_buffer;

	while(lmp::netbuf::get(net_buffer, udp, udpid))
	{
		// udpid related nonsense
		const lmp::ClientInfo *const info = net_buffer.pop<lmp::ClientInfo>();
		if(info == NULL)
		{
			log("no client info present in net buffer");
			continue;
		}

		Client *const client = Client::by_secret(info->secret, client_list);
		if(client == NULL)
		{
			log("received a datagram from an unrecognized client");
			continue;
		}
		else if(!client->udpid.initialized)
		{
			client->udpid = udpid;
		}

		integrate_client(*client, *info);
	}
}

void Server::compile_datagram(const Client &client, lmp::netbuf &buffer)
{
	const Player &current = client.player(state.player_list);
	int repair_percentage = current.percent_repair;
	// see if this guy is repairing anything
	if(repair_percentage == 0)
	{
		// see if this guy is being repaired
		for(const Player &p : state.player_list)
		{
			if(&current == &p)
				continue;

			if(p.repairing_id == current.id)
			{
				repair_percentage = p.percent_repair;
				break;
			}
		}
	}

	bool info_present = false;
	const GameState &oldstate = get_hist_state(client.stepno);

	// server info
	lmp::ServerInfo info;
	info.stepno = state.stepno;
	info.my_id = client.id;
	if(oldstate.score != state.score)
		info_present = true;
	info.repair = repair_percentage;
	if(repair_percentage != 0)
		info_present = true;
	info.score = state.score;
	buffer.push(info);

	std::vector<const Entity*> ent_list;

	// figure out what players have changed
	compile_diff(oldstate.player_list, state.player_list, ent_list);
	for(const auto subject : ent_list)
	{
		info_present = true;
		buffer.push(lmp::Player(*(Player*)subject));
	}

	// figure out what asteroids have changed
	ent_list.clear();
	compile_diff(oldstate.asteroid_list, state.asteroid_list, ent_list);
	for(const auto subject : ent_list)
	{
		info_present = true;
		buffer.push(lmp::Asteroid(*(Asteroid*)subject));
	}

	// figure out what ships have changed
	ent_list.clear();
	compile_diff(oldstate.ship_list, state.ship_list, ent_list);
	for(const auto subject : ent_list)
	{
		info_present = true;
		buffer.push(lmp::Ship(*(Ship*)subject));
	}


	// figure out what entities have been deleted
	std::vector<Entity::Reference> remove_list;
	compile_remove_diff(Entity::Type::PLAYER, oldstate.player_list, state.player_list, remove_list);
	compile_remove_diff(Entity::Type::ASTEROID, oldstate.asteroid_list, state.asteroid_list, remove_list);
	compile_remove_diff(Entity::Type::SHIP, oldstate.ship_list, state.ship_list, remove_list);
	for(const auto subject : remove_list)
	{
		info_present = true;
		buffer.push(lmp::Remove(subject));
	}

	if(!info_present)
		buffer.reset();
}

void Server::integrate_client(Client &client, const lmp::ClientInfo &lump)
{
	client.controls.up = lump.up == 1;
	client.controls.right = lump.right == 1;
	client.controls.left = lump.left == 1;
	client.controls.down = lump.down == 1;
	client.controls.fire = lump.fire == 1;
	client.controls.angle = lump.angle;
	client.stepno = lump.stepno;
	client.last_datagram_time = time(NULL);
	Player &player = client.player(state.player_list);
	player.shooting = client.controls.fire;
	player.rot = client.controls.angle;
}

const GameState &Server::get_hist_state(unsigned stepno) const
{
	for(const GameState &st : history)
	{
		if(st.stepno == stepno)
		{
			return st;
		}
	}

	return GameState::blank;
}

void Server::check_timeout()
{
	const int now = time(NULL);

	for(const Client &client : client_list)
	{
		if(!client.udpid.initialized)
			continue;

		if(now - client.last_datagram_time > CLIENT_TIMEOUT)
		{
			kick(client, "ping timeout");
			return;
		}
	}
}

void Server::step()
{
	++state.stepno;

	// see if players are all dead
	bool gameover = true;
	for(const Player &p : state.player_list)
	{
		if(p.health > 0)
		{
			gameover = false;
			break;
		}
	}
	if(gameover)
	{
		if(--gameover_timer == 0)
		{
			state.reset();
			gameover_timer = TIMER_GAMEOVER;
		}
	}

	// process players
	for(Client &client : client_list)
		client.player(state.player_list).step(true, client.controls, state, 1.0f, random);

	// process boooletts
	Bullet::step(true, state, NULL, random);

	// process asteroids
	Asteroid::step(true, state.asteroid_list, state.player_list, random, 244);

	// process ships
	Ship::step(true, state.ship_list, state.asteroid_list, NULL, 1.0f, random);

	// add this state to the history
	history.push_back(state);
	if(history.size() > STATE_HISTORY)
		history.pop_front();
}

void Server::loop(Server *s)
{
	Server &server = *s;

	while(server.running)
	{
		server.accept(); // accept or reject new clients

		server.recv(); // receive data from clients

		server.step(); // one world-simulation step

		server.send(); // send data to clients

		server.check_timeout(); // see who has timed out

		server.wait(); // sleep (or spinlock) until time for next loop
	}
}
