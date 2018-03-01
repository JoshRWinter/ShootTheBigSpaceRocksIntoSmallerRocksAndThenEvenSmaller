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
}

void Server::recv()
{
	int dgram_size;
	unsigned offset = 0;
	net::udp_id udpid;
	while((dgram_size = udp.recv(net_buffer.data(), net_buffer.size(), udpid)) > 0)
	{
		while((int)offset < dgram_size)
		{
			// read type
			lmp::Type type;
			memcpy(&type, net_buffer.data() + offset, sizeof(type));
			offset += sizeof(type);
			switch(type)
			{
				case lmp::Type::CLIENT_INFO:
				{
					lmp::ClientInfo info(net_buffer, offset);
					info.deserialize();
					offset += info.size;
					process(info);
					break;
				}
				default:
					log("unrecognized lump type: " + std::to_string(int(type)));
					break;
			}
		}
	}
}

void Server::process(const lmp::ClientInfo &info)
{
	log(std::string("up: ") + (info.up ? "true" : "false"));
}

void Server::loop(Server *s)
{
	Server &server = *s;

	while(server.running)
	{
		server.accept();

		server.recv();

		server.wait();
	}
}
