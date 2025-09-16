#pragma once

#define NOUSER
#include <raylib.h>

#include <array>
#include <vector>
#include <utility>
#include "engine.h"



struct Rectangle;

namespace chess
{



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
		Game(int screenSize, int minSize, const char* title, const std::string& host, unsigned short port);

		void run();

		~Game();

	private:


		int screenSize; // one value becuase the screen will always be square
		int minSize;

		const char* title;

		bool isGameOver = false;
		bool windowExists = false;



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

			void reset()
			{
				state = NO_CLICK;
				pos.first = -1;
				pos.second = -1;
				buildWall = false;
				hoverPos = -1;
			}
		} click;

		Player player = PL_WHITE;

		client::Client client;

		bool startGame = false;


	private:

		void process_input();

		int get_x_pos(int index) const;
		int get_y_pos(int index) const;

		int get_index(int x, int y) const;
		int get_index_from_mouse_pos() const;

		void render_board();
		void load_assets();

		std::pair<int, int> process_str_to_pair(const std::string& str, unsigned int offset) const;
		std::pair<ToFrom, PromotionResult> process_promotion(const std::string& str) const;
		std::pair<int, int> process_str_to_pair_wall(const std::string& str) const;
		// Handles
		void handle_resize();
		void handle_clicks();

		void render_promotion_options();

		void handle_promotion_input();

	};
}