#pragma once

#include <asio.hpp>



namespace server
{

	class Server
	{
	public:

		// Constructor: Initializes the server with a specified port
		Server(unsigned short port, std::function<void()> onConnect = nullptr, std::function<void()> onDisconnect = nullptr);

		void set_on_disconnect(std::function<void()> callback);

		void set_on_connect(std::function<void()> callback);

		void add_on_message_received(std::function<void(const std::vector<uint8_t>&)> callback);

		template<typename T>
		void send_data(size_t clientId, const T& data);

#ifdef SERVER_SAVE_PREV_DATA
		std::vector<uint8_t> get_accumulated_data(size_t id);
#endif
		template<typename T>
		void send_data_to_some(const std::vector<size_t>& clientId, const T& data);

		template<typename T>
		void send_data_to_all(const T& data);

		asio::ip::tcp::endpoint get_endpoint() const;

		void stop();

		~Server();

	private:

		std::thread _ioThread;

		asio::io_context _ioContext; 
	
		asio::executor_work_guard<asio::io_context::executor_type> _workGuard;

		asio::ip::tcp::endpoint _endpoint;
		asio::ip::tcp::acceptor _acceptor; 

		struct ClientSession
		{
			std::shared_ptr<asio::ip::tcp::socket> socket;
			std::vector<uint8_t> buffer;

			ClientSession(std::shared_ptr<asio::ip::tcp::socket> sock, size_t bufferSize)
				: socket(std::move(sock)), buffer(bufferSize) {
			}

			ClientSession() = default;
		};

		std::unordered_map<size_t, ClientSession> _clients;


		size_t _nextClientId = 0; 

		std::function<void()> _onDisconnect = nullptr;
		std::function<void()> _onConnect = nullptr;
		std::vector<std::function<void(const std::vector<uint8_t>&)>> _onMessageReceived;

#ifdef SERVER_SAVE_PREV_DATA
		std::unordered_map<size_t, std::vector<uint8_t>> _accumulatedData;
#endif

		void start_accept(); 
		
		void handle_accept(std::shared_ptr<asio::ip::tcp::socket> socket, const asio::error_code& error);

		//void handle
		void start_session(size_t id);

		void handle_incoming_data(const asio::error_code& error, size_t byteSizeTransferred, size_t id);
	};

}

#include "server.inl" 