#include "headers.h"
#include "websockets.h"

namespace websocket
{
    WebSocketClient::WebSocketClient(const char* url)
    {
        connect(url);
    }

    WebSocketState websocket::WebSocketClient::connect(const char* url)
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

    void websocket::WebSocketClient::set_on_message_received(std::function<void(const std::string&)> func)
    {
        onMessageRecieved = std::move(func);
    }

    websocket::WebSocketClient::~WebSocketClient()
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
        if (e->isText)
        {
            auto* self = static_cast<WebSocketClient*>(userData);
            self->onMessageRecieved(std::string(reinterpret_cast<const char*>(e->data), e->numBytes));
        }
        else
        {
            printf("Received binary message of %d bytes:\n", e->numBytes);
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(e->data);
            for (int i = 0; i < e->numBytes; i++)
            {
                printf("%02X ", bytes[i]);
            }
            printf("\n");
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