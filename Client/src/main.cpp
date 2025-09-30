#include "headers.h"
#include "main.h"



int main()
{
	std::cout << "Starting chess client..." << std::endl;

	//websocket::WebSocketClient client("ws://localhost:8080");

	//emscripten_set_main_loop(game_loop, 0, 1);

	// Create game instance

	chess::Game game(1000, 400, "Chess Game", "wss://fort-chess-host-402203537997.us-east4.run.app");

    // Browser main loop

	return 0;

}