#include "headers.h"
#include "client.h"

namespace client
{
	Client::Client(const std::string& host, unsigned short port)
		: _ioContext(), _socket(_ioContext), _endpoint(asio::ip::make_address(host), port)
		
		{
	}
}
