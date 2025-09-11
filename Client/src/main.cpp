#include "headers.h"
#include "client.h"

int main()
{
	client::Client client("127.0.0.1", 8080);

	
	client.set_on_message_received([](const std::vector<uint8_t>& data) {
		std::string message(data.begin(), data.end());
		std::cout << "Received message: " << message << std::endl;
		});


	std::cin.get();
	client.send<std::string>("Hello from Client!");
	std::cin.get();
	client.send<std::string>("Hello from Client 2!");
	std::cin.get(); 

	std::cout << "Press any key following enter to stop client connection ..." << std::endl;


	client.stop();
}