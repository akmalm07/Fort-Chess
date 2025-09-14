#include "headers.h"
#include "engine.h"



// Impleent a stateless, function based chess design!
// Instead of having to see if the pawn moved, we cna simply check if the pieces is in its starting position
// This will make it easier to implement an AI later on

namespace chess
{

	namespace {
		int get_index(Player player, int row, int col)
		{
			if (player == PL_BLACK)
				return (7 - row) * 8 + (7 - col); // flipped perspective
			return row * 8 + col;
		}

	}

	ChessEngine::ChessEngine(Player p, unsigned int timeoutPerMoveInMilliseconds)
		: player(p), timeoutPerMoveMs(timeoutPerMoveInMilliseconds)
	{
		reset_board();
	}

	void ChessEngine::opponent_move(int from, int to)
	{
		move_piece_no_check(from, to);
	}


	void ChessEngine::reset_board()
	{
		piecesLeft = 12;
		

		std::array<Pieces, 64> initial = {
			B_ROOK,  B_KNIGHT,  B_BISHOP,  B_QUEEN,  B_KING,  B_BISHOP,  B_KNIGHT,  B_ROOK,
			B_PAWN,  B_PAWN,    B_PAWN,    B_PAWN,   B_PAWN,  B_PAWN,    B_PAWN,    B_PAWN,

			EMPTY,   EMPTY,     EMPTY,     EMPTY,    EMPTY,   EMPTY,     EMPTY,     EMPTY,
			EMPTY,   EMPTY,     EMPTY,     EMPTY,    EMPTY,   EMPTY,     EMPTY,     EMPTY,
			EMPTY,   EMPTY,     EMPTY,     EMPTY,    EMPTY,   EMPTY,     EMPTY,     EMPTY,
			EMPTY,   EMPTY,     EMPTY,     EMPTY,    EMPTY,   EMPTY,     EMPTY,     EMPTY,

			W_PAWN,  W_PAWN,    W_PAWN,    W_PAWN,   W_PAWN,  W_PAWN,    W_PAWN,    W_PAWN,
			W_ROOK,  W_KNIGHT,  W_BISHOP,  W_QUEEN,  W_KING,  W_BISHOP,  W_KNIGHT,  W_ROOK
		};

		if (player == PL_BLACK)
			std::reverse(initial.begin(), initial.end());

		chessBorders.fill(false);

		size_t next = 0;

		// assign pieces
		for (int i = 0; i < 64; i++)
		{
			boardSetup[i].piece = initial[i];
			int row = i / 8;
			int col = i % 8;

			// Left walls
			if (col > 0) // reuse right walls of left neighbor
				boardSetup[i].walls[DIR_LEFT - 1] = boardSetup[i - 1].walls[DIR_RIGHT - 1].value();
			else
				boardSetup[i].walls[DIR_LEFT - 1] = std::nullopt; // no walls at the left edge

			// Right walls
			if (col < 7) // only create right walls for non-edge squares
				boardSetup[i].walls[DIR_RIGHT - 1] = std::ref(chessBorders[next++]);
			else
				boardSetup[i].walls[DIR_RIGHT - 1] = std::nullopt; // right edge

			// Up walls
			if (row > 0) // reuse down walls of top neighbor
				boardSetup[i].walls[DIR_UP - 1] = boardSetup[i - 8].walls[DIR_DOWN - 1].value();
			else
				boardSetup[i].walls[DIR_UP - 1] = std::nullopt; // top edge

			// Down walls
			if (row < 7) // only create down walls for non-edge squares
				boardSetup[i].walls[DIR_DOWN - 1] = std::ref(chessBorders[next++]);
			else
				boardSetup[i].walls[DIR_DOWN - 1] = std::nullopt; // bottom edge
		}
	}

	void ChessEngine::add_en_passent_oppertunity(int underPosition, int whenImplemented)
	{
		enPassantOppertunities.push_back({ underPosition, whenImplemented });
	}

	void ChessEngine::opponent_promote(ToFrom toFrom, PromotionResult res)
	{
		if (res == PR_NONE)
			return;
		
		boardSetup[toFrom.from].piece = EMPTY;
		switch (res)
		{
		case PR_QUEEN:
			boardSetup[toFrom.to].piece = (player == PL_WHITE ? B_QUEEN : W_QUEEN);
			break;
		case PR_ROOK:
			boardSetup[toFrom.to].piece = (player == PL_WHITE ? B_ROOK : W_ROOK);
			break;
		case PR_BISHOP:
			boardSetup[toFrom.to].piece = (player == PL_WHITE ? B_BISHOP : W_BISHOP);
			break;
		case PR_KNIGHT:
			boardSetup[toFrom.to].piece = (player == PL_WHITE ? B_KNIGHT : W_KNIGHT);
			break;

		}
	}

	void ChessEngine::check_timeouts()
	{
		auto now = std::chrono::steady_clock::now();
		for (auto it = timeOutPositions.begin(); it != timeOutPositions.end();)
		{
			if (now >= it->expiry)
				it = timeOutPositions.erase(it);
			else
				++it;
		}
	}


	Pieces ChessEngine::piece_at(int index) const
	{
		return boardSetup[index].piece;
	}

	int ChessEngine::piece_count() const
	{
		return piecesLeft;
	}

	MoveState ChessEngine::move_piece(int from, int to)
	{
		if (!valid_piece(from))
			return MOVE_INVALID;

		switch (boardSetup[from].piece)
		{
		case W_PAWN:
		case B_PAWN:
			return handle_pawn_move(from, to);
		case W_ROOK:
		case B_ROOK:
			return handle_rook_move(from, to);
		case W_KNIGHT:
		case B_KNIGHT:
			return handle_knight_move(from, to);
		case W_BISHOP:
		case B_BISHOP:
			return handle_bishop_move(from, to);

		case W_QUEEN:
		case B_QUEEN:
			return handle_queen_move(from, to);
		case W_KING:
		case B_KING:
			return handle_king_move(from, to);
		default:
			return MOVE_INVALID;
		}
	}

	WallState ChessEngine::is_wall_at(int index, Direction dir) const
	{
		if (dir == DIR_NONE)
		{
			for (const auto& d : boardSetup[index].walls)
				if (d == true)
					return WL_SUCCESS;
			return WL_INVALID;
		}
		else
		{
			if (boardSetup[index].walls[(int)dir - 1] == true)
				return WL_SUCCESS;
			return WL_INVALID;
		}
	}

	std::array<bool, 4> ChessEngine::get_wall_at(int i) const
	{
		std::array<bool, 4> result;

		for (size_t j = 0; j < boardSetup[i].walls.size(); j++)
		{
			result[j] = boardSetup[i].walls[j].has_value() ? boardSetup[i].walls[j]->get() : false;
		}

		return result;
	}

	size_t ChessEngine::get_board_size() const
	{
		return 64;
	}

	int ChessEngine::get_game_moves_count() const
	{
		return gameMovesCount; 
	}

	WallState ChessEngine::build_wall(int place, int direction)
	{
		if (boardSetup[place].piece != (player == PL_WHITE ? W_PAWN : B_PAWN) || place == direction)
			return WL_INVALID;

		RowCol rc = get_row_col(place, direction);

		Direction dir = DIR_NONE;

		if (rc.fromCol == rc.toCol)
		{
			if (rc.fromRow > rc.toRow)
				dir = DIR_UP;
			else
				dir = DIR_DOWN;
		}
		else if (rc.fromRow == rc.toRow)
		{
			if (rc.fromCol > rc.toCol)
				dir = DIR_LEFT;
			else
				dir = DIR_RIGHT;
		}
		else
			return WL_INVALID;

		if (dir == DIR_NONE)
			return WL_INVALID;
		else if (boardSetup[place].walls[(int)dir - 1] != false)
			return WL_WALL_EXISTS;

		if (auto& wall = boardSetup[place].walls[(int)dir - 1])
		{
			//SHould I allow walls to be counted as moves?
			//++gameMovesCount;
			wall->get() = true;
			add_timeout(place);
		}
		else
			return WL_INVALID;

		return WL_SUCCESS;
	}

	void ChessEngine::build_wall_opponent(int place, int direction)
	{
		RowCol rc = get_row_col(place, direction);

		Direction dir = DIR_NONE;

		if (rc.fromCol == rc.toCol)
		{
			if (rc.fromRow > rc.toRow)
				dir = DIR_UP;
			else
				dir = DIR_DOWN;
		}
		else if (rc.fromRow == rc.toRow)
		{
			if (rc.fromCol > rc.toCol)
				dir = DIR_LEFT;
			else
				dir = DIR_RIGHT;
		}

		if (dir == DIR_NONE)
		{
			return;
		}

		if (auto& wall = boardSetup[place].walls[(int)dir - 1])
		{
			wall->get() = true;
		}
	}


	bool ChessEngine::piece_exists(int index) const
	{
		return boardSetup[index].piece != EMPTY;
	}

	int ChessEngine::get_under_position_of(int square)
	{
		RowCol rc = get_row_col(square, square);
	
		if (rc.fromRow == 7)
			return -1;

		return square + 8;
	
	}

	bool ChessEngine::valid_piece(int index) const
	{
		return piece_exists(index) &&
			(player == PL_WHITE
				? (W_KING <= boardSetup[index].piece && boardSetup[index].piece <= W_PAWN)
				: (B_KING <= boardSetup[index].piece && boardSetup[index].piece <= B_PAWN));
	}

	void ChessEngine::promote(PromotionResult promotion)
	{
		if (waitingForPromotion.to == -1)
			return;

		switch (promotion)
		{
		case PR_QUEEN:
			boardSetup[waitingForPromotion.to].piece = (player == PL_WHITE ? W_QUEEN : B_QUEEN);
			break;
		case PR_ROOK:
			boardSetup[waitingForPromotion.to].piece = (player == PL_WHITE ? W_ROOK : B_ROOK);
			break;
		case PR_BISHOP:
			boardSetup[waitingForPromotion.to].piece = (player == PL_WHITE ? W_BISHOP : B_BISHOP);
			break;
		case PR_KNIGHT:
			boardSetup[waitingForPromotion.to].piece = (player == PL_WHITE ? W_KNIGHT : B_KNIGHT);
			break;

		}
		waitingForPromotion = { -1, -1 };
	}

	ToFrom ChessEngine::get_waiting_for_promotion() const
	{
		return waitingForPromotion; 
	}

	int ChessEngine::reverse(int pos)
	{
		int row = pos / 8;
		int col = pos % 8;
		return (7 - row) * 8 + (7 - col); 
	}

	bool ChessEngine::did_other_lose() const
	{
		for (int i = 0; i < boardSetup.size(); i++)
		{
			if (boardSetup[i].piece == (player == PL_WHITE ? B_KING : W_KING))
				return false;
		}
		return true;
	}	

	std::array<BoardData, 64> ChessEngine::get_board() const
	{
		return boardSetup;
	}

	ChessEngine::RowCol ChessEngine::get_row_col(int from, int to) const
	{
		RowCol rc;
		rc.fromRow = from / 8;
		rc.fromCol = from % 8;
		rc.toRow = to / 8;
		rc.toCol = to % 8;

		return rc;
	}

	bool ChessEngine::is_other_player_piece(int index) const
	{
		return player == PL_WHITE
			? (B_KING <= boardSetup[index].piece && boardSetup[index].piece <= B_PAWN)
			: (W_KING <= boardSetup[index].piece && boardSetup[index].piece <= W_PAWN);
	}

	//  0   1   2   3   4   5   6   7 
	//  8   9   10  11  12  13  14  15 
	//  16  17  18  19  20  21  22  23 
	//  24  25  26  27  28  29  30  31 
	//  32  33  34  35  36  37  38  39 
	//  40  41  42  43  44  45  46  47 
	//  48  49  50  51  52  53  54  55 
	//  56  57  58  59  60  61  62  63

	MoveState ChessEngine::handle_pawn_move(int from, int to)
	{

		if (is_in_timeout(from) || to == from)
			return MOVE_INVALID;
		std::function<bool(const RowCol&)> move_to_right_left = [](const RowCol& rc)
		{
			return rc.toCol == rc.fromCol - 1 || rc.toCol == rc.fromCol + 1;
		};

		RowCol rc = get_row_col(from, to);


		if (boardSetup[from].walls[DIR_UP - 1] == false)
		{

			if (rc.toCol == rc.fromCol && rc.toRow == rc.fromRow - 1 && boardSetup[to].piece == EMPTY)
			{
				// Promotion without capture
				move_piece_no_check(from, to);
				if (rc.toRow == 0)
				{
					waitingForPromotion = { from, to };
					return MOVE_PROMOTION;
				}

				return MOVE_SUCCESS;
			}

			// Double move from starting position
			if (rc.toCol == rc.fromCol && rc.fromRow == 6 /* 6 is the start row */ && rc.toRow == rc.fromRow - 2 &&
				boardSetup[row_col_to_index(rc.toRow + 1, rc.toCol)].piece == EMPTY && boardSetup[to].piece == EMPTY)
			{
				move_piece_no_check(from, to);
				return MOVE_EN_PASSENT_OPPORTUNITY;
			}
		}

		// Capture move
		if (((rc.toCol == rc.fromCol - 1 && can_move_diagonal_wall_check(from, DIR_UP_LEFT)) ||
			(rc.toCol == rc.fromCol + 1 && can_move_diagonal_wall_check(from, DIR_UP_RIGHT)))
			&& rc.toRow == rc.fromRow - 1)
		{
			if (en_passent_avalible(to))
			{
				move_piece_no_check(from, to);
				boardSetup[row_col_to_index(rc.toRow + 1, rc.toCol)].piece = EMPTY;
				--piecesLeft;
			}
			else if (is_other_player_piece(to))
			{
				move_piece_no_check(from, to);
				--piecesLeft;

				// Promotion with capture
				if (rc.toRow == 0)
				{
					waitingForPromotion = { from, to };
					return MOVE_PROMOTION_CAPTURE;
				}
			}

			return MOVE_CAPTURE;
		}

		return MOVE_INVALID;
	}


	MoveState ChessEngine::handle_rook_move(int from, int to)
	{
		return move_up_down_left_right(from, to, true);
	}

	MoveState ChessEngine::handle_bishop_move(int from, int to)
	{
		if (is_in_timeout(from) || to == from)
			return MOVE_INVALID;

		RowCol rc = get_row_col(from, to);

		auto handle_diagonal_move = [this, to, from](DiagnolDirection dir) -> MoveState
			{
				int direction = (dir == DIR_DOWN_RIGHT ? 9 : (dir == DIR_DOWN_LEFT ? 7 : (dir == DIR_UP_RIGHT ? -7 : -9)));

				int current = from;
				while (current >= 0 && current < 64)
				{
					if (current == to)
					{
						if (is_other_player_piece(current))
						{
							move_piece_no_check(from, to);
							--piecesLeft;
							return MOVE_CAPTURE;
						}
						else if (boardSetup[current].piece == EMPTY)
						{
							move_piece_no_check(from, to);
							return MOVE_SUCCESS;
						}
						else
							return MOVE_INVALID;
					}
					else if (!can_move_diagonal_wall_check(current, dir))
						break;
					else if (piece_exists(current + direction) && current + direction != to)
						break;

					current += direction;
				}

				return MOVE_INVALID;
			};

		if (std::abs(rc.toRow - rc.fromRow) == std::abs(rc.toCol - rc.fromCol))
		{
			if (rc.toRow > rc.fromRow && rc.toCol > rc.fromCol)
				return handle_diagonal_move(DIR_DOWN_RIGHT); // Move down-right
			else if (rc.toRow > rc.fromRow && rc.toCol < rc.fromCol)
				return handle_diagonal_move(DIR_DOWN_LEFT); // Move down-left
			else if (rc.toRow < rc.fromRow && rc.toCol > rc.fromCol)
				return handle_diagonal_move(DIR_UP_RIGHT); // Move up-right
			else if (rc.toRow < rc.fromRow && rc.toCol < rc.fromCol)
				return handle_diagonal_move(DIR_UP_LEFT); // Move up-left
		}

		return MOVE_INVALID;
	}


	MoveState ChessEngine::handle_knight_move(int from, int to)
	{

		if (is_in_timeout(from) || to == from)
			return MOVE_INVALID;

		RowCol rc = get_row_col(from, to);
		if ((std::abs(rc.toRow - rc.fromRow) == 2 && std::abs(rc.toCol - rc.fromCol) == 1) ||
			(std::abs(rc.toRow - rc.fromRow) == 1 && std::abs(rc.toCol - rc.fromCol) == 2))
		{
			if (is_other_player_piece(to))
			{
				move_piece_no_check(from, to);
				--piecesLeft;
				return MOVE_CAPTURE;
			}
			else if (boardSetup[to].piece == EMPTY)
			{
				move_piece_no_check(from, to);
				return MOVE_SUCCESS;
			}
		}
		return MOVE_INVALID;

	}
	MoveState ChessEngine::handle_queen_move(int from, int to)
	{
		RowCol rc = get_row_col(from, to);
		if (rc.fromRow == rc.toRow || rc.fromCol == rc.toCol)
		{
			return move_up_down_left_right(from, to, false);
		}
		else if (std::abs(rc.toRow - rc.fromRow) == std::abs(rc.toCol - rc.fromCol))
		{
			return handle_bishop_move(from, to);
		}
		return MOVE_INVALID;
	}

	MoveState ChessEngine::handle_king_move(int from, int to)
	{
		if (is_in_timeout(from) || to == from)
			return MOVE_INVALID;

		RowCol rc = get_row_col(from, to);
		if (std::abs(rc.toRow - rc.fromRow) <= 1 && std::abs(rc.toCol - rc.fromCol) <= 1)
		{
			kingMoved = true;
			if (is_other_player_piece(to))
			{
				move_piece_no_check(from, to);
				--piecesLeft;
				return MOVE_CAPTURE;
			}
			else if (boardSetup[to].piece == EMPTY)
			{
				move_piece_no_check(from, to);
				return MOVE_SUCCESS;
			}
		}

		// Moving piece from 59 to 57
		// Moving piece from 59 to 61

		// Moving piece from 60 to 58
		// Moving piece from 60 to 62

		// Castling

		return handle_castling(from, to);
	}

	MoveState ChessEngine::handle_castling(int from, int to)
	{
		if (kingMoved)
			return MOVE_INVALID;

		if (player == PL_BLACK)
		{
			if (to == 57 && boardSetup[56].piece == B_ROOK && boardSetup[57].piece == EMPTY && boardSetup[58].piece == EMPTY)
			{
				boardSetup[56].piece = EMPTY;
				boardSetup[57].piece = B_KING;
				boardSetup[58].piece = B_ROOK;
				boardSetup[59].piece = EMPTY;
				kingMoved = true;
				++gameMovesCount;
				return MOVE_SUCCESS;
			}
			else if (to == 61 && boardSetup[63].piece == B_ROOK && boardSetup[62].piece == EMPTY && boardSetup[61].piece == EMPTY && boardSetup[60].piece == EMPTY)
			{
				boardSetup[59].piece = EMPTY;
				boardSetup[60].piece = B_ROOK;
				boardSetup[61].piece = B_KING;
				boardSetup[62].piece = EMPTY;
				boardSetup[63].piece = EMPTY;
				kingMoved = true;
				++gameMovesCount;
				return MOVE_SUCCESS;
			}
		}
		else
		{
			if (to == 58 && boardSetup[56].piece == W_ROOK && boardSetup[57].piece == EMPTY && boardSetup[58].piece == EMPTY && boardSetup[59].piece == EMPTY)
			{
				boardSetup[56].piece = EMPTY;
				boardSetup[57].piece = EMPTY;
				boardSetup[58].piece = W_KING;
				boardSetup[57].piece = W_ROOK;
				boardSetup[60].piece = EMPTY;
				kingMoved = true;
				++gameMovesCount;
				return MOVE_SUCCESS;
			}
			else if (to == 62 && boardSetup[63].piece == W_ROOK && boardSetup[62].piece == EMPTY && boardSetup[61].piece == EMPTY)
			{
				boardSetup[60].piece = EMPTY;
				boardSetup[61].piece = W_KING;
				boardSetup[62].piece = W_ROOK;
				boardSetup[63].piece = EMPTY;
				kingMoved = true;
				++gameMovesCount;
				return MOVE_SUCCESS;
			}
		}

		return MOVE_INVALID;
	}

	int ChessEngine::row_col_to_index(int row, int col) const
	{
		return (row * 8) + col;
	}

	bool ChessEngine::is_in_timeout(int from)
	{
		for (const auto& to : timeOutPositions)
		{
			if (to.position == from)
				return true;
		}
		return false;
	}

	bool ChessEngine::en_passent_avalible(int to) const
	{
		for (const auto& ep : enPassantOppertunities)
		{
			if (to == ep.underPosition)
			{
				return true;
			}
		}
		return false;
	}

	MoveState ChessEngine::move_up_down_left_right(int from, int to, bool canBreakWalls)
	{
		RowCol rc = get_row_col(from, to);

		if (is_in_timeout(from) || to == from)
			return MOVE_INVALID;

		std::function<MoveState(Direction)> handle_straight_line_move = [this, to, from, canBreakWalls](Direction dir) -> MoveState
			{
				if (dir == DIR_NONE)
					return MOVE_INVALID;

				int direction = (dir == DIR_DOWN ? 8 : (dir == DIR_LEFT ? -1 : (dir == DIR_RIGHT ? 1 : -8)));

				int current = from;
				while (current >= 0 && current < 64)
				{
					if (current == to)
					{
						if (is_other_player_piece(current))
						{
							move_piece_no_check(from, to);
							--piecesLeft;

							return MOVE_CAPTURE;
						}
						else if (boardSetup[current].piece == EMPTY)
						{
							move_piece_no_check(from, to);
							return MOVE_SUCCESS;
						}
						else
							return MOVE_INVALID;
					}
					else if (piece_exists(current + direction) && current + direction != to)
						break;

					if (canBreakWalls)
						break_wall_if_encoutered(current, dir);
					else if (!can_move_straight_wall_check(current, dir))
						break;


					current += direction;
				}

				return MOVE_INVALID;
			};


		if (rc.fromRow == rc.toRow)
		{
			if (rc.toCol > rc.fromCol)
				return handle_straight_line_move(DIR_RIGHT); // Move right
			else
				return handle_straight_line_move(DIR_LEFT); // Move left
		}
		else if (rc.fromCol == rc.toCol)
		{
			if (rc.toRow > rc.fromRow)
				return handle_straight_line_move(DIR_DOWN); // Move down
			else
				return handle_straight_line_move(DIR_UP); // Move up
		}
		return MOVE_INVALID;
	}

	bool ChessEngine::can_move_diagonal_wall_check(int refPos, DiagnolDirection dir) const
	{
		if (refPos < 0 || refPos >= 64)
			return false;

		auto wallExists = [this, refPos](int offset, Direction dir) -> bool {
			const auto& opt = boardSetup[refPos + offset].walls[(int)dir - 1];
			return opt ? opt->get() : true;
			};

		switch (dir)
		{
		case DIR_UP_LEFT:

			return !((wallExists(0, DIR_UP) && wallExists(0, DIR_LEFT))
				|| (wallExists(-9, DIR_DOWN) && wallExists(-9, DIR_RIGHT))
				|| (wallExists(-9, DIR_RIGHT) && wallExists(0, DIR_LEFT))
				|| (wallExists(-9, DIR_DOWN) && wallExists(0, DIR_UP)));

		case DIR_UP_RIGHT:

			return !((wallExists(0, DIR_UP) && wallExists(0, DIR_RIGHT))
				|| (wallExists(-7, DIR_DOWN) && wallExists(-7, DIR_LEFT))
				|| (wallExists(-7, DIR_LEFT) && wallExists(0, DIR_RIGHT))
				|| (wallExists(-7, DIR_DOWN) && wallExists(0, DIR_UP)));

		case DIR_DOWN_LEFT:

			return !((wallExists(0, DIR_DOWN) && wallExists(0, DIR_LEFT))
				|| (wallExists(7, DIR_UP) && wallExists(7, DIR_RIGHT))
				|| (wallExists(7, DIR_RIGHT) && wallExists(0, DIR_LEFT))
				|| (wallExists(7, DIR_UP) && wallExists(0, DIR_DOWN)));


		case DIR_DOWN_RIGHT:

			return !((wallExists(0, DIR_DOWN) && wallExists(0, DIR_RIGHT))
				|| (wallExists(9, DIR_UP) && wallExists(9, DIR_LEFT))
				|| (wallExists(9, DIR_LEFT) && wallExists(0, DIR_RIGHT))
				|| (wallExists(9, DIR_UP) && wallExists(0, DIR_DOWN)));

		default:
			return false;
		}
	}


	void ChessEngine::move_piece_no_check(int from, int to)
	{
		boardSetup[to].piece = boardSetup[from].piece;
		boardSetup[from].piece = EMPTY;

		for (auto it = enPassantOppertunities.begin(); it != enPassantOppertunities.end();)
		{
			if (it->whenImplemented + 1 == gameMovesCount)
				it = enPassantOppertunities.erase(it);
			else
				++it;
		}

		++gameMovesCount;
		
		add_timeout(to);
	}

	void ChessEngine::add_timeout(int position)
	{
		timeOutPositions.emplace_back(position, std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutPerMoveMs));
	}

	bool ChessEngine::can_move_straight_wall_check(int refPos, Direction dir) const
	{
		if (refPos < 0 || refPos >= 64)
			return false;

		auto wallExists = [this, refPos](int offset, Direction dir) -> bool {
			const auto& opt = boardSetup[refPos + offset].walls[(int)dir - 1];
			return opt ? opt->get() : true;
			};

		switch (dir)
		{
		case DIR_UP:
			return !wallExists(0, DIR_UP);

		case DIR_DOWN:
			return !wallExists(0, DIR_DOWN);

		case DIR_LEFT:
			return !wallExists(0, DIR_LEFT);

		case DIR_RIGHT:
			return !wallExists(0, DIR_RIGHT);

		default:
			return false;
		}
	}

	void ChessEngine::break_wall_if_encoutered(int refPos, Direction dir)
	{
		if (refPos < 0 || refPos >= 64)
			return;
		auto breakWall = [this, refPos](int offset, Direction dir) {
			auto& opt = boardSetup[refPos + offset].walls[(int)dir - 1];
			if (opt && opt->get() == true)
				opt->get() = false;
			};
		switch (dir)
		{
		case DIR_UP:
			breakWall(0, DIR_UP);
			breakWall(-8, DIR_DOWN);
			break;
		case DIR_DOWN:
			breakWall(0, DIR_DOWN);
			breakWall(8, DIR_UP);
			break;
		case DIR_LEFT:
			breakWall(0, DIR_LEFT);
			breakWall(-1, DIR_RIGHT);
			break;
		case DIR_RIGHT:
			breakWall(0, DIR_RIGHT);
			breakWall(1, DIR_LEFT);
			break;
		default:
			break;
		}
	}
} // namespace chess