#include "headers.h"
#include "server.h"


namespace server
{
	Server::Server(unsigned short port, std::function<void()> onConnet, std::function<void()> onDisconnect)
		: _ioContext(), _workGuard(asio::make_work_guard(_ioContext)), _endpoint(asio::ip::tcp::v4(), port), _acceptor(_ioContext, _endpoint)
	{
		if (onConnet != nullptr)
		{
			_onConnect = std::move(onConnet);
		}
		if (onDisconnect != nullptr)
		{
			_onDisconnet = std::move(onDisconnect);
		}

		start_accept();
	}
	void Server::run()
	{
		_ioThread = std::thread([this]() 
			{
				_ioContext.run();
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

	void Server::set_on_message_received(std::function<void()> callback)
	{
		_onMessageReceived = std::move(callback);
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

		_workGuard.reset();
		_ioThread.join();
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
		_clientBuffers.back().reserve(1024); 
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
			return;
		}
		if (byteSizeTransferred > _clientBuffers[id].capacity())
		{
			std::cerr << "Received data size exceeds buffer capacity for client " << id << ". Resizing buffer." << std::endl;
			_clientBuffers[id].resize(byteSizeTransferred); 
		}

		std::vector<uint8_t>& buffer = _clientBuffers[id];

		if (_onMessageReceived != nullptr)
		{
			_onMessageReceived();
		}

		for (const auto& trigger : _onMessageWithTrigger)
		{
			if (byteSizeTransferred >= trigger.size &&
				std::equal(buffer.begin(), buffer.begin() + trigger.size, trigger.comparison.begin()))
			{
				trigger.func();
				break; 
			}
		}

		for (const auto& trigger : _onMessageStartingWith)
		{
			if (byteSizeTransferred >= trigger.size &&
				std::equal(buffer.begin(), buffer.begin() + trigger.size, trigger.comparison.begin()))
			{
				trigger.func(); 
				break;
			}
		}
	}


}