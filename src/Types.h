#pragma once
#include <iostream>
#include <assert.h>

typedef unsigned long long U64;

#define BRD_SQ_NUM 120
#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

extern U64 SetMask[64];
extern U64 ClearMask[64];

namespace BalouxEngine {
	
	struct Move {
		int move;
		int score;
	};

	struct MoveList {
		Move moves[MAXPOSITIONMOVES];
		int count;
	};

	enum Piece {
		PieceNone,
		WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen, WhiteKing,
		BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen, BlackKing,
	};

	enum File : int{
		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE
	};

	enum Rank : int {
		RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE
	};

	enum Color {
		White, Black, Both
	};

	enum Square : int {
		A1 = 21, B1, C1, D1, E1, F1, G1, H1,
		A2 = 31, B2, C2, D2, E2, F2, G2, H2,
		A3 = 41, B3, C3, D3, E3, F3, G3, H3,
		A4 = 51, B4, C4, D4, E4, F4, G4, H4,
		A5 = 61, B5, C5, D5, E5, F5, G5, H5,
		A6 = 71, B6, C6, D6, E6, F6, G6, H6,
		A7 = 81, B7, C7, D7, E7, F7, G7, H7,
		A8 = 91, B8, C8, D8, E8, F8, G8, H8,
		SQ_NONE, OFFBOARD
	};

	enum CastlingRights {
		WHITE_OO = 1,
		WHITE_OOO = 2,
		BLACK_OO = 4,
		BLACK_OOO = 8,
	};

	// Const expressions

	constexpr bool PiecePawn[13] = { false, true, false, false, false, false, false, true, false, false, false, false, false };
	constexpr bool PieceKnight[13] = { false, false, true, false, false, false, false, false, true, false, false, false, false };
	constexpr bool PieceKing[13] = { false, false, false, false, false, false, true, false, false, false, false, false, true };
	constexpr bool PieceRookQueen[13] = { false, false, false, false, true, true, false, false, false, false, true, true, false };
	constexpr bool PieceBishopQueen[13] = { false, false, false, true, false, true, false, false, false, true, false, true, false };
	constexpr bool PieceSlides[13] = { false, false, false, true, true, true, false, false, false, true, true, true, false };

	// Macros


#define FR2SQ(f,r) ( (21 + (f)) + ( (r) * 10 ) )
#define CLRBIT(bb, sq) ((bb.Get()) &= ClearMask[(sq)])
#define SETBIT(bb, sq) ((bb.Get()) |= SetMask[(sq)])
#define SQ64(s) (Utils::Sq120ToSq64(s))
#define SQ120(s) (Utils::Sq64ToSq120(s))
#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsKN(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])

#define FROM(m) ((m) & 0x7F)
#define TO(m) (((m) >> 7) &0x7F)
#define CAPTURED(m) (((m) >> 14) & 0xF)
#define PROMOTED(m) (((m) >> 20) & 0xF)

#define MOVEFLAGEP 0x40000
#define MOVEFLAGPS 0x80000
#define MOVEFLAGCAS 0x1000000
#define MOVEFLAGCAP 0x7C000
#define MOVEFLAGPROM 0xF00000
}

/*
enum Move : int {
		MoveNone,
		MoveNull = 65
	};

	enum MoveType {
		Normal,
		Promotion = 1,
		EnPassant = 2,
		Castling = 3
	};




	enum Phase {
		Opening,
		Middlegame,
		Endgame,
		PhaseNumber = 2
	};

	enum Value : int {
		MateValue = 30000,
		PawnValueMg = 126, PawnValueEg = 208,
		KnightValueMg = 781, KnightValueEg = 854,
		BishopValueMg = 825, BishopValueEg = 915,
		RookValueMg = 1276, RookValueEg = 1380,
		QueenValueMg = 2538, QueenValueEg = 2682
	};

	enum PieceType {
		PieceTypeNone, Pawn, Knight, Bishop, Rook, Queen, King
	};

	using Depth = int;



	enum Direction : int {
		NORTH = 8,
		EAST = 1,
		SOUTH = -NORTH,
		WEST = -EAST,

		NORTH_EAST = NORTH + EAST,
		SOUTH_EAST = SOUTH + EAST,
		SOUTH_WEST = SOUTH + WEST,
		NORTH_WEST = NORTH + WEST
	};



	/// Score enum stores a middlegame and an endgame value in a single integer (enum).
	/// The least significant 16 bits are used to store the middlegame value and the
	/// upper 16 bits are used to store the endgame value. We have to take care to
	/// avoid left-shifting a signed int to avoid undefined behavior.
	enum Score : int { SCORE_ZERO };

	constexpr Score make_score(int mg, int eg) {
		return Score((int)((unsigned int)eg << 16) + mg);
	}

	/// Extracting the signed lower and upper 16 bits is not so trivial because
	/// according to the standard a simple cast to short is implementation defined
	/// and so is a right shift of a signed integer.
	inline Value eg_value(Score s) {
		union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) };
		return Value(eg.s);
	}

	inline Value mg_value(Score s) {
		union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) };
		return Value(mg.s);
	}

#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, int d2) { return T(int(d1) + d2); }    \
constexpr T operator-(T d1, int d2) { return T(int(d1) - d2); }    \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, int d2) { return d1 = d1 + d2; }       \
inline T& operator-=(T& d1, int d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

	ENABLE_FULL_OPERATORS_ON(Value)
		ENABLE_FULL_OPERATORS_ON(Direction)

		ENABLE_INCR_OPERATORS_ON(Piece)
		ENABLE_INCR_OPERATORS_ON(PieceType)
		ENABLE_INCR_OPERATORS_ON(Square)
		ENABLE_INCR_OPERATORS_ON(File)
		ENABLE_INCR_OPERATORS_ON(Rank)

		ENABLE_BASE_OPERATORS_ON(Score)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON

		/// Additional operators to add a Direction to a Square
		constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
	constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
	inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
	inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

	/// Only declared but not defined. We don't want to multiply two scores due to
	/// a very high risk of overflow. So user should explicitly convert to integer.
	Score operator*(Score, Score) = delete;

	/// Division of a Score must be handled separately for each term
	inline Score operator/(Score s, int i) {
		return make_score(mg_value(s) / i, eg_value(s) / i);
	}

	/// Multiplication of a Score by an integer. We check for overflow in debug mode.
	inline Score operator*(Score s, int i) {

		Score result = Score(int(s) * i);

		assert(eg_value(result) == (i * eg_value(s)));
		assert(mg_value(result) == (i * mg_value(s)));
		assert((i == 0) || (result / i) == s);

		return result;
	}

	/// Multiplication of a Score by a boolean
	inline Score operator*(Score s, bool b) {
		return b ? s : SCORE_ZERO;
	}

	constexpr Color operator~(Color c) {
		return Color(c ^ Black); // Toggle color
	}

	constexpr Square flip_rank(Square s) { // Swap A1 <-> A8
		return Square(s ^ SQ_A8);
	}

	constexpr Square flip_file(Square s) { // Swap A1 <-> H1
		return Square(s ^ SQ_H1);
	}

	constexpr Piece operator~(Piece pc) {
		return Piece(pc ^ 8); // Swap color of piece B_KNIGHT <-> W_KNIGHT
	}

	constexpr CastlingRights operator&(Color c, CastlingRights cr) {
		return CastlingRights((c == White ? WHITE_CASTLING : BLACK_CASTLING) & cr);
	}

	constexpr Value mate_in(int ply) {
		return MateValue - ply;
	}

	constexpr Value mated_in(int ply) {
		return -MateValue + ply;
	}

	constexpr Square make_square(File f, Rank r) {
		return Square((r << 3) + f);
	}

	constexpr Piece make_piece(Color c, PieceType pt) {
		return Piece((c << 3) + pt);
	}

	constexpr PieceType type_of(Piece pc) {
		return PieceType(pc & 7);
	}

	inline Color color_of(Piece pc) {
		assert(pc != PieceNone);
		return Color(pc >> 3);
	}

	constexpr bool is_ok(Move m) {
		return m != MoveNone && m != MoveNull;
	}

	constexpr bool is_ok(Square s) {
		return s >= SQ_A1 && s <= SQ_H8;
	}

	constexpr File file_of(Square s) {
		return File(s & 7);
	}

	constexpr Rank rank_of(Square s) {
		return Rank(s >> 3);
	}

	constexpr Square relative_square(Color c, Square s) {
		return Square(s ^ (c * 56));
	}

	constexpr Rank relative_rank(Color c, Rank r) {
		return Rank(r ^ (c * 7));
	}

	constexpr Rank relative_rank(Color c, Square s) {
		return relative_rank(c, rank_of(s));
	}

	constexpr Direction pawn_push(Color c) {
		return c == White ? NORTH : SOUTH;
	}

	constexpr Square from_sq(Move m) {
		assert(is_ok(m));
		return Square((m >> 6) & 0x3F);
	}

	constexpr Square to_sq(Move m) {
		assert(is_ok(m));
		return Square(m & 0x3F);
	}

	constexpr int from_to(Move m) {
		return m & 0xFFF;
	}

	constexpr MoveType type_of(Move m) {
		return MoveType(m & (3 << 14));
	}

	constexpr PieceType promotion_type(Move m) {
		return PieceType(((m >> 12) & 3) + Knight);
	}

	constexpr Move make_move(Square from, Square to) {
		return Move((from << 6) + to);
	}

	template<MoveType T>
	constexpr Move make(Square from, Square to, PieceType pt = Knight) {
		return Move(T + ((pt - Knight) << 12) + (from << 6) + to);
	}*/