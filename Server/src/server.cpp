#include "headers.h"
#include "server.h"

namespace server
{
	Server::Server(unsigned short port, std::function<void()> onConnect, std::function<void()> onDisconnect)
		: _ioContext(),
		_workGuard(asio::make_work_guard(_ioContext)),
		_endpoint(asio::ip::tcp::v4(), port), 
		_acceptor(_ioContext, _endpoint)
	{
		if (onConnect) _onConnect = std::move(onConnect);
		if (onDisconnect) _onDisconnect = std::move(onDisconnect);

		start_accept();

		_ioThread = std::thread([this]() 
			{
			try {
				_ioContext.run();
			}
			catch (const std::exception& e)
			{
				if constexpr (SERVER_DEBUG)
				{
					std::cerr << "IO context error: " << e.what() << std::endl;
				}
			}
			});

		if constexpr (SERVER_DEBUG)
		{
			std::cout << "Server started on " << _endpoint.address().to_string() << ":" << _endpoint.port() << std::endl; \
		}
	}

	void Server::set_on_disconnect(std::function<void()> callback)
	{
		_onDisconnect = std::move(callback);
	}

	void Server::set_on_connect(std::function<void()> callback)
	{
		_onConnect = std::move(callback);
	}

	void Server::add_on_message_received(std::function<void(const std::vector<uint8_t>&)> callback)
	{
		_onMessageReceived.push_back(std::move(callback));
	}

#ifdef SERVER_SAVE_PREV_DATA

	std::vector<uint8_t> Server::get_accumulated_data(size_t id) 
	{
		auto it = _clients.find(id);
		if (it == _clients.end())
		{
			if constexpr (SERVER_DEBUG)
			{
				std::cerr << "Client ID " << id << " not found." << std::endl;
			}
			return {};
		}
		const ClientSession& client = it->second;
		if constexpr (SERVER_DEBUG)
		{
			std::cout << "Accumulated data for client " << id << ": ";

			const auto& data = _accumulatedData.at(id); 
			for (const auto& byte : data)
			{
				std::cout << static_cast<uint8_t>(byte) << " ";
			}
			std::cout << std::endl;
		}

		return _accumulatedData.at(id);
	}
#endif

	asio::ip::tcp::endpoint Server::get_endpoint() const
	{
		return _endpoint;
	}

	void Server::stop()
	{
		_acceptor.close();
		_ioContext.stop();

		for (auto& [id, socket] : _clients)
		{
			if (socket.socket->is_open())
				socket.socket->close();
		}

		if (_ioThread.joinable())
			_ioThread.join();

		_clients.clear();

		if constexpr (SERVER_DEBUG)
		{
			std::cout << "Server stopped." << std::endl;
		}
	}

	Server::~Server()
	{
		stop();
	}

	void Server::start_accept()
	{
		auto socket = std::make_shared<asio::ip::tcp::socket>(_ioContext);

		_acceptor.async_accept(*socket, [this, socket](const asio::error_code& error) 
			{
				handle_accept(socket, error);
			});
	}

	void Server::handle_accept(std::shared_ptr<asio::ip::tcp::socket> socket, const asio::error_code& error)
	{
		if (error)
		{
			if (error == asio::error::operation_aborted)
				return;

			std::cerr << "Accept failed: " << error.message() << std::endl;
			return;
		}

		size_t clientId = ++_nextClientId;

		if constexpr (SERVER_DEBUG)
		{
			std::cout << "Client connected: " << socket->remote_endpoint() << std::endl;
		}

		_clients[clientId] = std::move(ClientSession{ socket, 1024 });

#ifdef SERVER_SAVE_PREV_DATA
		_accumulatedData[clientId] = std::vector<uint8_t>();
#endif

		if (_onConnect)
			_onConnect();



		start_session(clientId);

		start_accept(); 
	}

	void Server::start_session(size_t id)
	{
		if (_clients.find(id) == _clients.end())
		{
			std::cerr << "Client ID " << id << " not found." << std::endl;
			return;
		}
		ClientSession& client = _clients[id];

		if constexpr (SERVER_DEBUG)
		{
			std::cout << "Starting session with client: " << client.socket->remote_endpoint() << std::endl;
		}

		size_t netId = htonl(id);
		asio::write(*client.socket, asio::buffer(&netId, sizeof(netId)));

		client.socket->async_read_some(
			asio::buffer(client.buffer),
			[this, id](const asio::error_code& error, std::size_t bytesTransferred)
			{
				handle_incoming_data(error, bytesTransferred, id);
			});
	}

	void Server::handle_incoming_data(const asio::error_code& error, size_t byteSizeTransferred, size_t id)
	{
		if (_clients.find(id) == _clients.end())
		{
			if constexpr (SERVER_DEBUG)
			{
				std::cerr << "Client ID " << id << " not found." << std::endl;
			}
			return;
		}
		if (error || byteSizeTransferred == 0)
		{
			if (error != asio::error::eof && error != asio::error::operation_aborted)
			{
				if constexpr (SERVER_DEBUG)
				{
				std::cerr << "Error receiving data from client " << id << ": " << error.message() << std::endl;
				}
			}
			else
			{
				if constexpr (SERVER_DEBUG)
				{
					std::cout << "Client " << id << " disconnected." << std::endl;
				}
			}

			if (_onDisconnect)
				_onDisconnect();

			_clients.erase(id);

			if constexpr (SERVER_DEBUG)
			{
				std::cout << "Session with client " << id << " ended." << std::endl;
			}
			return;
		}

		ClientSession& client = _clients[id];

		if (byteSizeTransferred > client.buffer.size())
		{
			if constexpr (SERVER_DEBUG)
			{
				std::cerr << "Received data size exceeds buffer capacity for client " << id << ". Resizing buffer." << std::endl;
			}
			client.buffer.resize(byteSizeTransferred);
		}

		std::vector<uint8_t> data(client.buffer.begin(), client.buffer.begin() + byteSizeTransferred);

#ifdef SERVER_SAVE_PREV_DATA
		_accumulatedData[id].insert(_accumulatedData[id].end(), data.begin(), data.end());
#endif

		for (const auto& callback : _onMessageReceived)
			callback(data);

		client.socket->async_read_some(
			asio::buffer(client.buffer),
			[this, id](const asio::error_code& err, size_t transferred)
			{
				handle_incoming_data(err, transferred, id);
			});
	}
}
