#include "headers.h"
#include "server.h"

int main()
{
	server::Server server(8080);
	//server.add_on_message_received([](const std::vector<uint8_t>&)
	//	{});
	server.add_on_message_received([](const std::vector<uint8_t>& data)
		{
			std::string message(data.begin(), data.end());

			std::cout << "Received message: " << message << std::endl;
		});

	std::cin.get();
	server.send_data_to_all(std::string("Hello, Client!"));
	std::cin.get();
	server.send_data_to_all(std::string("Hello, Client! 2"));

	std::cout << "Press Enter to stop the server..." << std::endl;
	std::cin.get();
}