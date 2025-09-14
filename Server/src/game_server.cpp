#include "headers.h"
#include "game_server.h"



namespace game
{

	GameServer::GameServer(unsigned short port)
		: server(port)
	{
		//openPlayerIds.set_callback([this](std::vector<uint32_t>& players)
		//{
		//	if (players.size() >= 2)
		//	{
		//		size_t size = players.size();
		//		for (size_t i = 0; i < size; i++)
		//		{
		//			size -= 2;
		//			uint32_t playerId = players.back();
		//			players.pop_back();
		//			uint32_t opponentId = players.back();
		//			players.pop_back();
		//			server.send_data_to_some<std::string>({ playerId, opponentId }, "startMessage");

		//			server.disconnect_client(playerId);
		//			server.disconnect_client(opponentId);
		//		}

		//	}
		//});

		server.set_on_connect([this]()
		{	
			if (server.client_count() % 2 == 0)
			{
				auto ids = server.get_all_client_ids();

				clientIds = { ids[0], ids[1] };

				server.send_data(clientIds.first, "BLACK");
				server.send_data(clientIds.second, "WHITE");
			}
		});

		server.set_on_disconnect([this]()
		{
			openPlayerIds.remove_player(server.get_current_client_id());
		});


		server.add_on_message_received([this](const std::vector<uint8_t>& data)
		{
			std::string message(data.begin(), data.end());

			auto clientId = server.get_last_client_sent_data();

			if (clientId == clientIds.first)
			{
				server.send_data<std::string>(clientIds.second, message);
			}
			else if (clientId == clientIds.second)
			{
				server.send_data<std::string>(clientIds.first, message);
			}

			std::cout << "Received message: " << message << std::endl;
		});
	}

	void Players::add_player(uint32_t id)
	{
		players.push_back(id);

		if (players.size() >= 2)
		{
			callback(players);
		}
	}
	void Players::remove_player(uint32_t id)
	{
		auto it = std::find(players.begin(), players.end(), id);
		if (it != players.end())
		{
			players.erase(it);
		}
	}

	void Players::set_callback(std::function<void(std::vector<uint32_t>&)> cb)
	{
		callback = std::move(cb);
	}
}