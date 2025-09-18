#pragma once

#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

#include <functional>
#include <string>


namespace websocket
{

	enum WebSocketState
	{
		FAILED,
		NOT_SUPPORTED,
		OPEN,
		CLOSING,
		CLOSED
	};


	class WebSocketClient
	{

	public:
		WebSocketClient() = default;
		WebSocketClient(const char* url);

		WebSocketState connect(const char* url);

		void send(const std::string& message);

		void set_on_message_received(std::function<void(const std::string&)> func);

		~WebSocketClient();

	private:
		std::function<void(const std::string&)> onMessageRecieved;
		EMSCRIPTEN_WEBSOCKET_T websocket;

	private:
		static EM_BOOL on_open(int eventType, const EmscriptenWebSocketOpenEvent* e, void* userData);

		static EM_BOOL on_message(int eventType, const EmscriptenWebSocketMessageEvent* e, void* userData);

		static EM_BOOL on_close(int eventType, const EmscriptenWebSocketCloseEvent* e, void* userData);

		static EM_BOOL on_error(int eventType, const EmscriptenWebSocketErrorEvent* e, void* userData);
	};

}