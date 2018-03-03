#include <stdexcept>

#include "Server.h"

int Client::last_id = 0;

Server::Server()
	: running(true)
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
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}while(diff.count() < 16666000);

	last = current;

	// frequency check
	{
		static int sps, second;
		const int current = time(NULL);
		if(current != second)
		{
			if(state.stepno > 200 && sps < 56)
				log("sps == " + std::to_string(sps) + " -- having trouble keeping up");
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

	Client client;
	Player player(++Client::last_id);
	client.id = player.id;
	client.secret = random(0, 500'000'000);
	stream.send_block(&client.secret, sizeof(client.secret));

	client_list.push_back(client);
	state.player_list.push_back(player);
}

void Server::send()
{
	for(const Client &client : client_list)
	{
		if(!client.udpid.initialized)
			continue;

		lmp::netbuf buffer;
		compile_datagram(client, buffer);

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

void Server::compile_datagram(const Client&, lmp::netbuf &buffer)
{
	// server info
	lmp::ServerInfo info;
	info.stepno = state.stepno;
	buffer.push(info);

	// players
	for(const Player &p : state.player_list)
	{
		lmp::Player player;

		player.id = p.id;
		player.x = p.x;
		player.y = p.y;
		player.xv = p.xv;
		player.yv = p.yv;
		player.shooting = p.shooting;
		player.health = p.health;

		buffer.push(player);
	}
}

void Server::integrate_client(Client &client, const lmp::ClientInfo &lump)
{
	client.controls.up = lump.up == 1;
	client.controls.right = lump.right == 1;
	client.controls.left = lump.left == 1;
	client.controls.down = lump.down == 1;
	client.controls.fire = lump.fire == 1;
	client.controls.angle = lump.angle;
}

void Server::step()
{
	++state.stepno;

	// process players
	for(Client &client : client_list)
		Player::step_server(client.player(state.player_list), client.controls);

	// add this state to the history
	history.push(state);
	if(history.size() > STATE_HISTORY)
		history.pop();
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

		server.wait(); // sleep (or spinlock) until time for next loop
	}
}
