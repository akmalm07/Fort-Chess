#include "headers.h"
#include "server.h"

namespace server
{
	Server::Server(unsigned short port, std::function<void()> onConnect, std::function<void()> onDisconnect)
		: _ioContext(),
		_workGuard(asio::make_work_guard(_ioContext)),
		_endpoint(asio::ip::make_address("0.0.0.0"), port), //DEBUG
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
				std::cerr << "IO context error: " << e.what() << std::endl;
			}
			});

		std::cout << "Server started on " << _endpoint.address().to_string() << ":" << _endpoint.port() << std::endl;
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
			if (socket->is_open())
				socket->close();
		}

		if (_ioThread.joinable())
			_ioThread.join();

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
				return; // Expected during shutdown

			std::cerr << "Accept failed: " << error.message() << std::endl;
			return;
		}

		size_t clientId = ++_nextClientId;

		std::cout << "Client connected: " << socket->remote_endpoint() << std::endl;

		_clients[clientId] = std::move(socket);

		_clientBuffers.emplace(clientId, 1024);

		if (_onConnect)
			_onConnect();

		start_session(*_clients[clientId], clientId);
		start_accept(); // queue next accept
	}

	void Server::start_session(asio::ip::tcp::socket& socket, size_t id)
	{
		std::cout << "Starting session with client: " << socket.remote_endpoint() << std::endl;

		size_t netId = htonl(id);
		asio::write(socket, asio::buffer(&netId, sizeof(netId)));

		socket.async_read_some(
			asio::buffer(_clientBuffers[id]),
			[this, id](const asio::error_code& error, std::size_t bytesTransferred)
			{
				handle_incoming_data(error, bytesTransferred, id);
			});
	}

	void Server::handle_incoming_data(const asio::error_code& error, size_t byteSizeTransferred, size_t id)
	{
		if (error || byteSizeTransferred == 0)
		{
			if (error != asio::error::eof && error != asio::error::operation_aborted)
			{
				std::cerr << "Error receiving data from client " << id << ": " << error.message() << std::endl;
			}
			else
			{
				std::cout << "Client " << id << " disconnected." << std::endl;
			}

			if (_onDisconnect)
				_onDisconnect();

			_clients.erase(id);
			_clientBuffers.erase(id);

			std::cout << "Session with client " << id << " ended." << std::endl;
			return;
		}

		if (byteSizeTransferred > _clientBuffers[id].size())
		{
			std::cerr << "Received data size exceeds buffer capacity for client " << id << ". Resizing buffer." << std::endl;
			_clientBuffers[id].resize(byteSizeTransferred);
		}

		std::vector<uint8_t> data(_clientBuffers[id].begin(), _clientBuffers[id].begin() + byteSizeTransferred);

		for (const auto& callback : _onMessageReceived)
			callback(data);

		_clients[id]->async_read_some(
			asio::buffer(_clientBuffers[id]),
			[this, id](const asio::error_code& err, size_t transferred)
			{
				handle_incoming_data(err, transferred, id);
			});
	}
}
