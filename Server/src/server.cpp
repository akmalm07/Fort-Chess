#include "headers.h"
#include "server.h"


namespace server
{
	Server::Server(unsigned short port, std::function<void()> onConnect, std::function<void()> onDisconnect)
		: _ioContext(), _workGuard(asio::make_work_guard(_ioContext)), _endpoint(asio::ip::tcp::v4(), port), _acceptor(_ioContext, _endpoint)
	{
		if (onConnect != nullptr)
		{
			_onConnect = std::move(onConnect);
		}
		if (onDisconnect != nullptr)
		{
			_onDisconnet = std::move(onDisconnect);
		}

		start_accept();

	
		std::cout << "Server started on " << _endpoint.address().to_string() << ":" << _endpoint.port() << std::endl;
	}
	void Server::run()
	{
		_ioThread = std::thread([this]() 
			{
				try
				{
					_ioContext.run();
				}
				catch (const std::exception& e)
				{
					std::cerr << "Client IO context error: " << e.what() << std::endl;
				}
			});
	}

	void Server::set_on_disconnect(std::function<void()> callback)
	{
		_onDisconnet = std::move(callback);
	}

	void Server::set_on_connect(std::function<void()> callback)
	{
		_onConnect = std::move(callback);
	}

	void Server::add_on_message_received(std::function<void(const std::vector<size_t>&)> callback)
	{
		_onMessageReceived.push_back(std::move(callback));
	}

	asio::ip::tcp::endpoint Server::get_endpoint() const
	{
		return _endpoint;
	}

	void Server::stop()
	{
		_acceptor.close();

		for (auto& [id, socket] : _clients)
		{
			if (socket->is_open())
			{
				socket->close();
			}
		}

		if (_ioThread.joinable())
		{
			_ioThread.join();
			_ioContext.stop();
		}

		_clients.clear();
		_clientBuffers.clear();

		std::cout << "Server stopped." << std::endl;
	}

	Server::~Server()
	{
		stop();
	}

	void Server::start_accept()
	{
		auto socket = asio::ip::tcp::socket(_ioContext); 
		_acceptor.async_accept(socket,
			[this, &socket](const asio::error_code& error) { handle_accept(socket, error); });

	}

	void Server::handle_accept(asio::ip::tcp::socket& socket, const asio::error_code& error)
	{
		if (error)
		{
			std::cerr << "Accept failed: " << error.message() << std::endl;
			return;
		}
		
		std::cout << "Client connected: " << socket.remote_endpoint() << std::endl;

		_nextClientId++;

		_clients.emplace(_nextClientId, std::make_unique<asio::ip::tcp::socket>(std::move(socket)));
		_clientBuffers.emplace_back();
		_clientBuffers.back().resize(1024); 
		if (_onConnect != nullptr)
		{
			_onConnect();
		}

		start_session(*_clients[_nextClientId], _nextClientId);
		

		start_accept();
	}

	void Server::start_session(asio::ip::tcp::socket& socket, size_t id)
	{
		std::cout << "Starting session with client: " << socket.remote_endpoint() << std::endl;

		size_t netId = htonl(id);

		asio::write(socket, asio::buffer(&netId, sizeof(netId)));
		
		socket.async_read_some(asio::buffer(_clientBuffers[id]),
			[this, id](const asio::error_code& error, std::size_t bytesTransferred)
			{
				handle_incoming_data(error, bytesTransferred, id);
			});


	}

    void Server::handle_incoming_data(const asio::error_code& error, size_t byteSizeTransferred, size_t id)
    {
       bool errorOccurred = false;
       if (error)
       {
           std::cerr << "Error receiving data from client " << id << ": " << error.message() << std::endl;
           errorOccurred = true; 
       }
       if (error == asio::error::eof || byteSizeTransferred == 0)
       {
           std::cout << "Client " << id << " disconnected." << std::endl;
           errorOccurred = true;
       }
       if (errorOccurred)
       {
           if (_onDisconnet != nullptr)
           {
               _onDisconnet();
           }
           _clients.erase(id);
           _clientBuffers.erase(_clientBuffers.begin() + id - 1); 

		   std::cout << "Session with client " << id << " ended." << std::endl;
           return;
       }
       if (byteSizeTransferred > _clientBuffers[id].capacity())
       {
           std::cerr << "Received data size exceeds buffer capacity for client " << id << ". Resizing buffer." << std::endl;
           _clientBuffers[id].resize(byteSizeTransferred); 
       }

       std::vector<size_t> buffer(_clientBuffers[id].begin(), _clientBuffers[id].end());

       for (const auto& msg : _onMessageReceived)
       {
           msg(buffer);
       }
    }


}