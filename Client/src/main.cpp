#include "headers.h"
#include "client.h"

int main()
{
	client::Client client("127.0.0.1", 8080);

	std::cout << "Press any key following enter to stop client connection ..." << std::endl;
	std::cin.get();
	
}