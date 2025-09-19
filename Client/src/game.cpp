#include "headers.h"
#include "chess_pieces_texture.h"
#include "game.h"

#define LIGHTBLUE Color(144, 213, 255, 255)

namespace chess
{

	Game* Game::instance = nullptr;

	Game::Game(int screenSize, int minSize, const char* title, const std::string& host, unsigned short port)
		: isGameOver(false), minSize(minSize), screenSize(screenSize), title(title)
	{		
		SetTraceLogLevel(LOG_ERROR);

		websocket::WebSocketState err = client.connect(("ws://" + host + ":" + std::to_string(port)).c_str());
		
		if (err != websocket::OPEN)
			return;

		client.set_on_message_received([this](const std::string& message) 
		{
			std::string data = message;
			data.erase(std::remove(data.begin(), data.end(), '\0'), data.end());

			auto pieces = chessEngine.get_board();

			if (!startGame)
			{
				if (data == "BLACK")
				{
					std::cout << "You are playing as black" << std::endl;

					chessEngine = ChessEngine(PL_BLACK, 1000);
					startGame = true;
					load_assets(); // DEBUG
					return;
				}
				else if (data == "WHITE")
				{
					std::cout << "You are playing as black" << std::endl;

					chessEngine = ChessEngine(PL_WHITE, 1000);
					startGame = true;
					load_assets(); // DEBUG
					return;
				}
				std::cout << data.size() << std::endl;
			}


			if (data.contains("PROM"))
			{
				auto res = process_promotion(data);
				chessEngine.opponent_promote(res.first, res.second);
			}
			else if (data.contains("TO"))
			{
				std::pair<int, int> toFrom = process_str_to_pair(data, 3);
				chessEngine.opponent_move(toFrom.first, toFrom.second);
			}
			else if (data.contains("WALL"))
			{
				std::pair<int, int> toFrom = process_str_to_pair_wall(data);
				chessEngine.build_wall_opponent(toFrom.first, toFrom.second);
			}
			else if (data.contains("ENPS"))
			{
				std::pair<int, int> toFrom = process_str_to_pair(data, 5);
				chessEngine.add_en_passent_oppertunity(toFrom.first, toFrom.second);
			}
			else if (data == "LOSE")
			{
				isGameOver = true;
				std::cout << "You Lost!" << std::endl;
			}
			else
			{
				isGameOver = true;
				std::cout << "Opponent disconnected" << std::endl;
			}
		});

		SetConfigFlags(FLAG_WINDOW_RESIZABLE);

		windowExists = true;
		InitWindow(screenSize, screenSize, title);
		SetTargetFPS(60);

		run();
	}


	void Game::run()
	{
		instance = this;
		emscripten_set_main_loop_arg(&Game::game_loop_stub, this, 0, 1);

	}



	void Game::render_board()
	{

		// Draw chess board
		for (int i = 0; i < 64; i++)
		{
			int cellSize = screenSize / 8;

			int x = (i % 8) * (cellSize);
			int y = int(i / 8) * (cellSize);

			if (chessEngine.is_square_waiting(i))
				DrawRectangle(x, y, cellSize, cellSize, LIGHTBLUE);
			else if (click.pos.first == i && click.state == FIRST_CLICK)
				DrawRectangle(x, y, cellSize, cellSize, RED);
			else if (click.hoverPos == i && click.state == FIRST_CLICK)
				DrawRectangle(x, y, cellSize, cellSize, GREEN);
			else if ((i + (i / 8)) % 2 == 0)
				DrawRectangle(x, y, cellSize, cellSize, LIGHTGRAY);
			else
				DrawRectangle(x, y, cellSize, cellSize, DARKGRAY);

			// Render walls
			if (chessEngine.is_wall_at(i) == WL_SUCCESS)
			{
				std::array<bool, 4> walls = chessEngine.get_wall_at(i);
				struct WallAdjust
				{
					int dx, dy;  // offset in pixels
					float wScale, hScale; // scale factors (fraction of cell)
				};

				int thinSize = cellSize / 16;
				float wallFrac = float(thinSize) / cellSize;

				std::array<WallAdjust, 4> adjust = {
					WallAdjust{0, 0, 1.0f, wallFrac},
					WallAdjust{0, cellSize - thinSize, 1.0f, wallFrac},
					WallAdjust{0, 0, wallFrac, 1.0f},
					WallAdjust{cellSize - thinSize, 0, wallFrac, 1.0f}
				};

				for (size_t j = 0; j < walls.size(); j++)
				{
					if (!walls[j])
						continue;

					int wallX = x + adjust[j].dx;
					int wallY = y + adjust[j].dy;
					int wallW = static_cast<int>(cellSize * adjust[j].wScale);
					int wallH = static_cast<int>(cellSize * adjust[j].hScale);

					DrawRectangle(wallX, wallY, wallW, wallH, Color(160, 82, 45, 255));
				}

			}
		}

		//Win or lose message
		if (isGameOver)
		{
			DrawRectangle(0, screenSize / 2 - 50, screenSize, 100, BLACK);
			const char* text = didWin.has_value() && didWin.value() ? "You Won!" : "You Lost!";
			int fontSize = 40;
			int textWidth = MeasureText(text, fontSize);
			int textX = (screenSize - textWidth) / 2;
			int textY = (screenSize / 2) - (fontSize / 2);
			DrawText(text, textX, textY, fontSize, WHITE);
		}

		int squareSize = screenSize / 8;
		// Draw pieces

		for (int i = 0; i < piecesRects.size(); i++)
		{
			for (int j = 0; j < 64; j++)
			{
				if (chessEngine.piece_at(j) == EMPTY || piecesRects[i].piece != chessEngine.piece_at(j))
					continue;
	
				int x = (j % 8) * squareSize;
				int y = (j / 8) * squareSize;
				Rectangle dest = {
					(float)x,                   
					(float)y,                   
					(float)squareSize,          
					(float)squareSize           
				};
				//Vector2 pos = { (float)x, (float)y };
				DrawTexturePro(texInfo.tex, piecesRects[i].rect, dest, { 0, 0 }, 0.0f, WHITE);
				
			}
		}

		if (promotion.stateActive)
			render_promotion_options();


		// ADD A RESIZER FOR SMALLER SCREENS AND ADD COLORING TO PIECES IN COOLDOWN

		// Decide move
		if (click.state == SECOND_CLICK)
		{
			std::cout << "Second click at " << click.pos.second << std::endl;

			if (click.buildWall && chessEngine.build_wall(click.pos.first, click.pos.second) == WL_SUCCESS)
			{
				client.send(std::string("WALL ") + std::to_string(chessEngine.reverse(click.pos.first)) + " " + std::to_string(chessEngine.reverse(click.pos.second)));
				click.reset();
			}
			else
			{
				switch (chessEngine.move_piece(click.pos.first, click.pos.second))
				{
				case MOVE_EN_PASSENT_OPPORTUNITY:
				{
					client.send(std::string("ENPS ") + std::to_string(chessEngine.reverse(chessEngine.get_under_position_of(click.pos.second))) + " " + std::to_string(chessEngine.get_game_moves_count()));
					client.send(std::string("TO ") + std::to_string(chessEngine.reverse(click.pos.first)) + " " + std::to_string(chessEngine.reverse(click.pos.second)));
					click.reset();
					break;
				}
				case MOVE_PROMOTION:
				case MOVE_PROMOTION_CAPTURE:
				{
					promotion.stateActive = PS_DECIDING;
					click.reset();
					break;
				}

				case MOVE_SUCCESS:
				case MOVE_CAPTURE:
				{
					client.send(std::string("TO ") + std::to_string(chessEngine.reverse(click.pos.first)) + " " + std::to_string(chessEngine.reverse(click.pos.second)));

					click.reset();
					break;
				}

				case MOVE_INVALID:
				{
					click.state = FIRST_CLICK;
					break;
				}

				default:
					click.state = FIRST_CLICK;
				}
			}

			if (chessEngine.did_other_lose())
			{
				client.send("LOSE");

				std::cout << "You WON!" << std::endl;
				didWin = true;
				isGameOver = true;
			}
		}


	}

	void Game::load_assets()
	{
		Image img = LoadImageFromMemory(".png", chess_pieces_spritesheet_png, chess_pieces_spritesheet_png_len);
		texInfo.tex = LoadTextureFromImage(img);
		UnloadImage(img);

		texInfo.width = texInfo.tex.width / 6;
		texInfo.height = texInfo.tex.height / 2;

		piecesRects.reserve(chessEngine.piece_count());

		for (int i = 0; i < chessEngine.piece_count(); i++)
		{
			Rectangle rec{ (float)(i % 6) * texInfo.width, (float)(i / 6) * texInfo.height, (float)texInfo.width, (float)texInfo.height };

			Pieces piece = static_cast<Pieces>(i + 1);

			piecesRects.push_back({ rec, piece });
		}

	}

	std::pair<int, int> Game::process_str_to_pair(const std::string& str, unsigned int offset) const
	{
		std::istringstream iss(str.substr(offset));
		std::string s;
		std::vector<std::string> strs;

		while (getline(iss, s, ' '))
			strs.push_back(s);

		if (strs.size() < 2)
			return { 0, 0 };

		strs.resize(2);

		return { get_number(strs[0]), get_number(strs[1]) };
	}

	std::pair<ToFrom, PromotionResult> Game::process_promotion(const std::string& str) const
	{
		std::istringstream iss(str.substr(5));
		std::string s;
		std::vector<std::string> strs;

		while (getline(iss, s, ' '))
			strs.push_back(s);

		if (strs.size() < 3)
			return { {0, 0}, PR_NONE };

		strs.resize(3);

		PromotionResult res = PR_NONE;

		if (strs[2] == "K")
			res = PR_KNIGHT;
		else if (strs[2] == "B")
			res = PR_BISHOP;
		else if (strs[2] == "R")
			res = PR_ROOK;
		else if (strs[2] == "Q")
			res = PR_QUEEN;

		return { { get_number(strs[0]), get_number(strs[1]) }, res };
	}

	std::pair<int, int> Game::process_str_to_pair_wall(const std::string& str) const
	{
		std::istringstream iss(str.substr(5));
		std::string s;
		std::vector<std::string> strs;
		while (getline(iss, s, ' '))
			strs.push_back(s);
		if (strs.size() < 2)
			return { 0, DIR_NONE };
		strs.resize(2);
		
		return { get_number(strs[0]), get_number(strs[1]) };

	}

	std::string to_str(PromotionResult res)
	{
		switch (res)
		{
		case PR_QUEEN:
			return "Q";
		case PR_ROOK:
			return "R";
		case PR_BISHOP:
			return "B";
		case PR_KNIGHT:
			return "K";
		default:
			return "N";
		}
	}

	void Game::game_loop_stub(void* arg)
	{
		Game* self = static_cast<Game*>(arg);
		self->game_loop();
	}

	void Game::game_loop()
	{

		if (WindowShouldClose())
		{
			emscripten_cancel_main_loop(); // clean exit
			return;
		}

		process_input();

		BeginDrawing();
		ClearBackground(BLACK);

		render_board();

		EndDrawing();

		chessEngine.check_timeouts();
	
	}

	void Game::process_input()
	{
		switch (promotion.stateActive)
		{
		case PS_DECIDING:
			handle_promotion_input();
			break;
		case PS_INACTIVE:
			handle_clicks();
			break;
		case PS_DECIDED:
			auto promPoses = chessEngine.get_waiting_for_promotion();
			client.send(std::string("PROM ") + std::to_string(chessEngine.reverse(promPoses.from)) + " " + std::to_string(chessEngine.reverse(promPoses.to)) + " " + to_str(promotion.result));
			chessEngine.promote(promotion.result);
			promotion.reset();
		}

		handle_resize();

	}

	void Game::handle_resize()
	{
		if (IsWindowResized())
		{
			int screenWidth = GetScreenWidth();
			int screenHeight = GetScreenHeight();

			if (screenWidth != screenHeight)
			{
				int newSize = std::min(screenWidth, screenHeight);
				if (newSize < minSize)
					screenSize = minSize;
				else
					screenSize = newSize;

				SetWindowSize(screenSize, screenSize);
			}
			click.reset();
		}
	}

	void Game::handle_clicks()
	{
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			int index = get_index_from_mouse_pos();

			switch (click.state)
			{
			case NO_CLICK:
				if (chessEngine.valid_piece(index))
				{
					click.pos.first = index;
					click.state = FIRST_CLICK;
				}
				break;

			case FIRST_CLICK:
				click.pos.second = index;

				std::cout << "First Click: " << click.pos.first << std::endl;

				if (click.pos.first != click.pos.second)
				{
					click.state = SECOND_CLICK;
					//boardSetup[click.pos.second] = boardSetup[click.pos.first];
					//boardSetup[click.pos.first] = EMPTY;
				}
				break;

			default:
				click.reset();
				break;
			}
		}
		else if (click.state == SECOND_CLICK && !chessEngine.valid_piece(click.pos.first))
		{
			click.reset();
		}
		else if (IsKeyPressed(KEY_B))
		{
			if (click.state == FIRST_CLICK)
			{
				click.buildWall = true;
			}
		}


		if (click.state == FIRST_CLICK)
		{
			click.hoverPos = get_index_from_mouse_pos();

			if (IsKeyPressed(KEY_BACKSPACE))
			{
				click.reset();
			}
		}
	}

	void Game::handle_promotion_input()
	{
		PromRects rects = promotion.get_rects();

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			Vector2 mouse = GetMousePosition();

			if (CheckCollisionPointRec(mouse, rects.queenBox))
			{
				promotion.result = PR_QUEEN;
				promotion.stateActive = PS_DECIDED;
			}
			else if (CheckCollisionPointRec(mouse, rects.rookBox))
			{
				promotion.result = PR_ROOK;
				promotion.stateActive = PS_DECIDED;
			}
			else if (CheckCollisionPointRec(mouse, rects.bishopBox))
			{
				promotion.result = PR_BISHOP;
				promotion.stateActive = PS_DECIDED;
			}
			else if (CheckCollisionPointRec(mouse, rects.knightBox))
			{
				promotion.result = PR_KNIGHT;
				promotion.stateActive = PS_DECIDED;
			}
		}


	}

	int Game::get_number(const std::string& number) const
	{
		int result = 0;
		for (char c : number)
		{
			if (c >= '0' && c <= '9')
				result = result * 10 + (c - '0');
			else
				break; // Stop processing at the first non-digit character
		}

		return result;
	}


	void Game::render_promotion_options()
	{
		PromRects rects = promotion.get_rects();

		// Draw the UI
		DrawRectangleRec(rects.promoBox, LIGHTGRAY);
		DrawRectangleRec(rects.queenBox, RAYWHITE);
		DrawRectangleRec(rects.rookBox, RAYWHITE);
		DrawRectangleRec(rects.bishopBox, RAYWHITE);
		DrawRectangleRec(rects.knightBox, RAYWHITE);

		// You can replace text with textures of chess pieces
		DrawText("Q", rects.queenBox.x + 30, rects.queenBox.y + 25, 30, BLACK);
		DrawText("R", rects.rookBox.x + 30, rects.rookBox.y + 25, 30, BLACK);
		DrawText("B", rects.bishopBox.x + 30, rects.bishopBox.y + 25, 30, BLACK);
		DrawText("N", rects.knightBox.x + 30, rects.knightBox.y + 25, 30, BLACK);

	}

	int Game::get_x_pos(int index) const
	{
		return (index % 8) * (screenSize / 8);
	}

	int Game::get_index_from_mouse_pos() const
	{
		Vector2 mouse = GetMousePosition();
		int col = mouse.x / (screenSize / 8);
		int row = mouse.y / (screenSize / 8);

		//std::cout << "Mouse Position: (" << mouse.x << ", " << mouse.y << ") -> Index: (" << row << ", " << col << ")\n";

		return row * 8 + col;
	}

	int Game::get_y_pos(int index) const
	{
		return (index / 8) * (screenSize / 8);
	}


	Game::~Game()
	{
		if (windowExists)
			CloseWindow();
	}

} // namespace chess