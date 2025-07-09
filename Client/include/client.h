#pragma once


namespace client
{

	class Client
	{
	public:
		// Constructor: Initializes the client with a specified host and port
		Client(const std::string& host, unsigned short port);

		void connect();
		// Sends data to the server
		template<typename T>
		void send(const T& data);
		// Receives data from the server
		void receive();
		// Sets a callback for when a message is received
		void set_on_message_received(std::function<void(const std::vector<uint8_t>&)> callback);
		// Stops the client connection
		void stop();

		~Client();
	private:
		asio::io_context _ioContext;
		asio::ip::tcp::socket _socket;
		asio::ip::tcp::endpoint _endpoint;
		std::thread _ioThread;
		std::function<void(const std::vector<uint8_t>&)> _onMessageReceived;
		void run(); // Runs the IO context in a separate thread
		void handle_receive(const asio::error_code& error, size_t bytes_transferred);
	};

}