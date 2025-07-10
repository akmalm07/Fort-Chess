#include "headers.h"
#include "server.h"

int main()
{
	server::Server server(8080);
	//server.add_on_message_received([](const std::vector<uint8_t>&)
	//	{});

	std::cout << "Press Enter to stop the server..." << std::endl;
	std::cin.get();
}