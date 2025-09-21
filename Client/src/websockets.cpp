#include "headers.h"
#include "websockets.h"

namespace websocket
{
    WebSocketClient::WebSocketClient(const char* url)
    {
        connect(url);
    }

    WebSocketState WebSocketClient::connect(const char* url)
    {
        if (!emscripten_websocket_is_supported())
        {
            std::cout << "WebSockets are not supported!\n";
            websocket = 0;
            return NOT_SUPPORTED;
        }

        EmscriptenWebSocketCreateAttributes weAttrs;
        emscripten_websocket_init_create_attributes(&weAttrs);
        weAttrs.url = url;
        weAttrs.protocols = NULL;
        weAttrs.createOnMainThread = EM_TRUE;

        websocket = emscripten_websocket_new(&weAttrs);
        if (websocket <= 0)
        {
            std::cout << "Failed to create WebSocket.\n";
            websocket = 0;
            return FAILED;
        }
 
        //emscripten_websocket_set_binary_type(websocket, EMSCRIPTEN_WEBSOCKET_BINARY_TYPE_ARRAYBUFFER);

        emscripten_websocket_set_onopen_callback(websocket, this, on_open);
        emscripten_websocket_set_onmessage_callback(websocket, this, on_message);
        emscripten_websocket_set_onclose_callback(websocket, this, on_close);
        emscripten_websocket_set_onerror_callback(websocket, this, on_error);

		return OPEN;
    }

    void WebSocketClient::send(const std::string& message)
    {
        if (websocket)
        {
			std::cout << "Sending message: " << message << std::endl;
            emscripten_websocket_send_utf8_text(websocket, message.c_str());
		}
    }

    void WebSocketClient::send(const std::vector<uint8_t>& data)
    {
        if (websocket)
        {
            emscripten_websocket_send_binary(websocket, (void*)data.data(), data.size());
        }
    }

    void WebSocketClient::set_on_message_received_str(std::function<void(const std::string&)> func)
    {
        onMessageRecievedStr = std::move(func);
    }

    void WebSocketClient::set_on_on_message_recived_bytes(std::function<void(std::vector<uint8_t>)> func)
    {
		onMessageRecievedByte = std::move(func);
    }

    WebSocketClient::~WebSocketClient()
    {
        if (websocket)
        {
            emscripten_websocket_close(websocket, 1000, "Normal Closure");
            emscripten_websocket_delete(websocket);
        }
    }


    EM_BOOL WebSocketClient::on_open(int eventType, const EmscriptenWebSocketOpenEvent* e, void* userData)
    {
        std::cout << "WebSocket opened!\n";
        return EM_TRUE;
    }

    EM_BOOL WebSocketClient::on_message(int eventType, const EmscriptenWebSocketMessageEvent* e, void* userData)
    {
        auto* self = static_cast<WebSocketClient*>(userData);
        if (e->isText)
        {
			printf("Received text message of %d bytes: %s\n", e->numBytes, reinterpret_cast<const char*>(e->data));
			if (self->onMessageRecievedStr)
                self->onMessageRecievedStr(std::string(reinterpret_cast<const char*>(e->data), e->numBytes));
        }
        else
        {
            printf("Received binary message of %d bytes:\n", e->numBytes);
            if (self->onMessageRecievedByte)
            {
                std::vector<uint8_t> bytes(
                    reinterpret_cast<const uint8_t*>(e->data),
                    reinterpret_cast<const uint8_t*>(e->data) + e->numBytes
                );
                self->onMessageRecievedByte(bytes);
            }
        }
        return EM_TRUE;
    }

    EM_BOOL WebSocketClient::on_close(int eventType, const EmscriptenWebSocketCloseEvent* e, void* userData)
    {
        std::cout << "WebSocket closed.\n";
        return EM_TRUE;
    }

    EM_BOOL WebSocketClient::on_error(int eventType, const EmscriptenWebSocketErrorEvent* e, void* userData)

    {
        std::cout << "WebSocket error!\n";
        return EM_TRUE;
    }

}