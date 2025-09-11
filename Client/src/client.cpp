#include "headers.h"
#include "client.h"

namespace client
{
	Client::Client(const std::string& host, unsigned short port, std::function<void()> onConnect, std::function<void()> onDisonnect)
		: _ioContext(), _endpoint(asio::ip::make_address(host), port), _socket(_ioContext)
	{	
		std::cout << "Client initialized. Connecting to " << host << ":" << port << std::endl;

		if (onConnect != nullptr)
		{
			_onConnect = std::move(onConnect);
		}
		if (onDisonnect != nullptr)
		{
			_onDisconnect = std::move(onDisonnect);
		}

		ClientError err = connect();

		if (err != ClientError::None)
		{
			std::cout << "Connection Not Found" << std::endl;
			return;
		}

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

		std::cout << "Client started on " << _endpoint.address().to_string() << ":" << _endpoint.port() << std::endl;

	}

	ClientError Client::connect()
	{
		_receiveBuffer.resize(1024);

		asio::error_code socketEc;
		_socket.connect(_endpoint, socketEc);
		if (socketEc)
		{
			std::cerr << "Connect failed: " << socketEc.message() << std::endl;
			stop();
			return ClientError::ConnectFailed;
		}

		asio::error_code dataEc;
		size_t data = 0;
		_socket.receive(asio::buffer(&data, sizeof(data)), 0, dataEc);
		if (dataEc)
		{
			std::cerr << "Initial connection failed: " << dataEc.message() << std::endl;
			stop(); 
			return ClientError::ConnectFailed;
		}
		_clientId = data;

		if (_onConnect != nullptr)
		{
			_onConnect();
		}

		_socket.async_receive(asio::buffer(_receiveBuffer),
			[this](const asio::error_code& error, size_t bytesTransferred)
			{
				handle_receive(error, bytesTransferred);
			});

		return ClientError::None;

	}

	void Client::set_on_connect(std::function<void()> callback)
	{
		_onConnect = std::move(callback);
	}

	void Client::set_on_disconnect(std::function<void()> callback)
	{
		_onDisconnect = std::move(callback);
	}

	void Client::set_on_message_received(std::function<void(const std::vector<uint8_t>&)> callback)
	{
		_onMessageReceived = std::move(callback);
	}

	void Client::stop()
	{
		_ioContext.stop();
		if (_ioThread.joinable())
		{
			_ioThread.join();
		}
		if (_socket.is_open())
		{
			_socket.close();
		}

		if (_onDisconnect)
		{
			_onDisconnect();
		}

		_receiveBuffer.clear();

		std::cout << "Client stopped." << std::endl;
	}

	Client::~Client()
	{
		stop();
	}

	void Client::run()
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

	void Client::handle_receive(const asio::error_code& error, size_t bytesTransferred)
	{
		bool errorOccured = false;
		if (error)
		{
			std::cerr << "Receive failed: " << error.message() << std::endl;
			errorOccured = true;
		}
		if (error == asio::error::eof || bytesTransferred == 0)
		{
			std::cout << "Server disconnected." << std::endl;
			errorOccured = true;
		}
		if (errorOccured)
		{
			std::cout << "Stopping client due to error." << std::endl;
			stop();
			return;
		}
		
		std::vector<uint8_t> data(_receiveBuffer.begin(), _receiveBuffer.begin() + bytesTransferred); 
		
		// this might copy the buffer from
		//the beginning to the buffer th the bytes transferred,
		//and if it was inserted twice, it might read from wrong offset
		_onMessageReceived(data);

		_socket.async_receive(asio::buffer(_receiveBuffer),
			[this](const asio::error_code& error, size_t bytesTransferred)
			{
				handle_receive(error, bytesTransferred);
			});
	}

}
