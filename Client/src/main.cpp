#include "headers.h"
#include "main.h"


void game_loop()
{
	InitWindow(1000, 1000, "Chess Client");

	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);
		for (int i = 0; i < 64; i++)
		{
			int cellSize = 800 / 8;

			int x = (i % 8) * (cellSize);
			int y = int(i / 8) * (cellSize);
			if ((i + (i / 8)) % 2 == 0)
				DrawRectangle(x, y, cellSize, cellSize, LIGHTGRAY);
			else
				DrawRectangle(x, y, cellSize, cellSize, DARKGRAY);
		}
		EndDrawing();
	}
}


int main()
{
	std::cout << "Starting chess client..." << std::endl;

	//websocket::WebSocketClient client("ws://localhost:8080");

	//emscripten_set_main_loop(game_loop, 0, 1);

	// Create game instance

	chess::Game game(1000, 400, "Chess Game", "localhost", 8080);

    // Browser main loop

	return 0;

}