#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>


namespace chess
{
	enum Pieces
	{
		EMPTY = 0,
		W_KING,
		W_QUEEN,
		W_BISHOP,
		W_KNIGHT,
		W_ROOK,
		W_PAWN,
		B_KING,
		B_QUEEN,
		B_BISHOP,
		B_KNIGHT,
		B_ROOK,
		B_PAWN,
	};


	struct BoardData
	{
	public:
		Pieces piece = EMPTY;
		std::array<std::optional<std::reference_wrapper<bool>>, 4> walls;

		BoardData() = default;
		BoardData(Pieces piece, std::array<std::optional<std::reference_wrapper<bool>>, 4> walls) : piece(piece), walls(walls) {};
	};

	struct TotalPiecesCallback
	{
	private:
		unsigned int piecesLeft = 12;
		std::function<void(const std::array<BoardData, 64>&)> onCapture;

	public:
		TotalPiecesCallback() = default;
		TotalPiecesCallback(unsigned int piecesLeft, std::function<void(const std::array<BoardData, 64>&)> func = nullptr)
			: piecesLeft(piecesLeft), onCapture(std::move(func)) {}
		
		void set_on_capture(std::function<void(const std::array<BoardData, 64>&)>&& func)
		{
			onCapture = std::move(func);
		}

		operator unsigned int() const
		{
			return piecesLeft;
		}

		void operator++() = delete;

		void operator--()
		{
			piecesLeft--;
		}

		void call_on_capture(const std::array<BoardData, 64>& board)
		{
			onCapture(board);
		}

	};

	enum Player
	{
		PL_WHITE = 0,
		PL_BLACK
	};

	enum MoveState
	{
		MOVE_SUCCESS = 0,
		MOVE_INVALID,
		MOVE_CAPTURE,
		MOVE_PROMOTION,
		MOVE_PROMOTION_CAPTURE,
	};

	enum WallState
	{
		WL_SUCCESS = 0,
		WL_INVALID,
		WL_WALL_EXISTS,
		WL_NO_WALLS_LEFT,
	};

	enum Direction
	{
		DIR_NONE = 0,
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,

	};

	enum DiagnolDirection
	{
		DIR_UP_RIGHT = 0,
		DIR_UP_LEFT,
		DIR_DOWN_RIGHT,
		DIR_DOWN_LEFT
	};

	enum PromotionResult
	{
		PR_NONE = 0,
		PR_QUEEN,
		PR_ROOK,
		PR_BISHOP,
		PR_KNIGHT
	};

	struct ToFrom 	
	{
		int from;
		int to;
	};

	class ChessEngine
	{
	public:

		ChessEngine() = default;

		ChessEngine(Player player);

		void init();

		void opponent_move(int from, int to);

		void reset_board();

		void opponent_promote(ToFrom toFrom, PromotionResult res);

		Pieces piece_at(int index) const;

		int piece_count() const;

		MoveState move_piece(int from, int to);

		WallState is_wall_at(int index, Direction dir = DIR_NONE) const;

		std::array<bool, 4> get_wall_at(int i) const;

		size_t get_board_size() const;

		WallState build_wall(int place, int direction);

		bool piece_exists(int index) const;

		bool valid_piece(int index) const;

		void promote(PromotionResult promotion);

		ToFrom get_waiting_for_promotion() const;

		int reverse(int pos);

		bool did_other_lose() const;

		std::array<BoardData, 64> get_board() const;


	private:

		struct EnPassentOppertunity
		{
			int underPosition;
			bool whenImplemented = false;
		};

		Player player;

		std::array<bool, 112> chessBorders = { false };

		ToFrom waitingForPromotion = { -1, -1 };

		std::array<BoardData, 64> boardSetup;

		std::vector<EnPassentOppertunity> enPassantOppertunities;

		TotalPiecesCallback piecesLeft;

		size_t gameMovesCount = 0;

		struct RowCol
		{
			int fromRow;
			int fromCol;
			int toRow;
			int toCol;
		};

		bool kingMoved = false;
		bool didOtherLose = false;

		RowCol get_row_col(int from, int to) const;

		bool is_other_player_piece(int index) const;

		MoveState handle_pawn_move(int from, int to);
		MoveState handle_rook_move(int from, int to);
		MoveState handle_knight_move(int from, int to);
		MoveState handle_bishop_move(int from, int to);
		MoveState handle_queen_move(int from, int to);
		MoveState handle_king_move(int from, int to);

		MoveState handle_castling(int from, int to);

		int row_col_to_index(int row, int col) const;

		bool en_passent_avalible(int to) const;

		MoveState move_up_down_left_right(int from, int to, bool canBreakWalls);

		bool can_move_diagonal_wall_check(int referencePos, DiagnolDirection dir) const;
		bool can_move_straight_wall_check(int referencePos, Direction dir) const;

		void break_wall_if_encoutered(int referencePos, Direction dir);

		void move_piece_no_check(int from, int to);
	};

}