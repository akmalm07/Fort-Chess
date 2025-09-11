#pragma once


namespace client
{
	enum class ClientError
	{
		None = 0,
		ConnectFailed,
		SendFailed,
		ReceiveFailed,
		Disconnected
	};

	class Client
	{
	public:

		Client(const std::string& host, unsigned short port, std::function<void()> onConnect = nullptr, std::function<void()> onDisonnect = nullptr);

		ClientError connect();

		void set_on_connect(std::function<void()> callback);
		void set_on_disconnect(std::function<void()> callback);

		template<typename T>
		void send(const T& data);

		void set_on_message_received(std::function<void(const std::vector<uint8_t>&)> callback);

		void stop();

		~Client();
	private:

		std::thread _ioThread;
		asio::io_context _ioContext;
		
		asio::ip::tcp::endpoint _endpoint;
		asio::ip::tcp::socket _socket;

		std::vector<uint8_t> _receiveBuffer; 


		std::function<void()> _onDisconnect = nullptr;
		std::function<void()> _onConnect = nullptr;

		size_t _clientId = 0;
		
		std::function<void(const std::vector<uint8_t>&)> _onMessageReceived;
		void run(); 
		void handle_receive(const asio::error_code& error, size_t bytesTransferred);
	};

}

#include "client.inl"