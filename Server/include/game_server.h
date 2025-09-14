#pragma once


#include "server.h"

namespace game
{

	struct Players
	{
	public:
		void add_player(uint32_t id);

		void remove_player(uint32_t id);

		void set_callback(std::function<void(std::vector<uint32_t>&)> cb);

	private:
		std::vector<uint32_t> players;
		std::function<void(std::vector<uint32_t>&)> callback;
	};

	class GameServer
	{
	public:
		GameServer(unsigned short port);
		
		~GameServer() = default;

	private:

		server::Server server;

		std::pair<uint32_t, uint32_t> clientIds = { 0, 0 };

		Players openPlayerIds;
	};
}
