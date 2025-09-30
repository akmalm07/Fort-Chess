#pragma once

#define NOUSER
#include <raylib.h>

#include <array>
#include <vector>
#include <utility>
#include "engine.h"
#include "websockets.h"


struct Rectangle;

namespace chess
{

	enum SentDataType
	{
		SDT_SET_COLOR = 0,
		SDT_MOVE,
		SDT_MOVE_ENPS,
		SDT_PROMOTE,
		SDT_PROMOTE_DECIDING,
		SDT_WALL,
		SDT_ENPS,
		SDT_LOSE,
		SDT_WIN, // Unused

		SDT_KILL_DECISTION
	};;

	enum ClickState
	{
		NO_CLICK = 0,
		FIRST_CLICK,
		SECOND_CLICK
	};

	enum PromotionState
	{
		PS_INACTIVE = 0,
		PS_DECIDING,
		PS_DECIDED
	};

	class Game
	{
	public:
		Game(int screenSize, int minSize, const char* title, const std::string& host, unsigned short port = 0);

		void run();

		~Game();

	private:

		static Game* instance;


		int screenSize; // one value becuase the screen will always be square
		int minSize;

		const char* title;

		bool windowExists = false;

		struct {
			bool isGameOver = false;
			bool didWin = false;
		} overState;

		struct PiecesTexturePos
		{
			Rectangle rect;
			Pieces piece;
		};

		std::vector<PiecesTexturePos> piecesRects;

		struct
		{
			int width;
			int height;
			Texture2D tex;
		} texInfo;

		ChessEngine chessEngine;


		struct PromRects
		{
			Rectangle promoBox;
			Rectangle queenBox;
			Rectangle rookBox;
			Rectangle bishopBox;
			Rectangle knightBox;
		};

		struct
		{
			PromotionState stateActive = PS_INACTIVE;
			PromotionResult result = PR_NONE;

			void reset()
			{
				stateActive = PS_INACTIVE;
				result = PR_NONE;
			}

			PromRects get_rects() const
			{
				return { { 300, 200, 400, 100 }, { 310, 210, 80, 80 }, { 400, 210, 80, 80 }, { 490, 210, 80, 80 }, { 580, 210, 80, 80 } };
			}

		} promotion;

		struct
		{
			ClickState state = NO_CLICK;
			std::pair<int, int> pos = { -1, -1 };
			int hoverPos = -1;
			bool buildWall = false;

			bool third = false;

			void reset()
			{
				state = NO_CLICK;
				pos.first = -1;
				pos.second = -1;
				buildWall = false;
				third = false;
				hoverPos = -1;
			}
		} click;

		websocket::WebSocketClient client;

		bool startGame = false;


	private:

		static void game_loop_stub(void* arg);

		void game_loop();

		void process_input();

		int get_x_pos(int index) const;
		int get_y_pos(int index) const;

		int get_index_from_mouse_pos() const;

		void render_board();
		void load_assets();

		int get_number(const std::string& number) const;

		std::pair<int, int> process_str_to_pair(const std::vector<uint8_t>& data, unsigned int offset) const;
		std::pair<ToFrom, PromotionResult> process_promotion(const std::vector<uint8_t>& data) const;
		std::pair<int, int> process_str_to_pair_wall(const std::string& str) const;
		// Handles
		void handle_resize();
		void handle_clicks();

		void handle_second_click();

		void render_promotion_options();

		void handle_promotion_input();

		void game_over_screen();

	};
}