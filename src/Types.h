#pragma once
#include <iostream>
#include <assert.h>

typedef unsigned long long U64;

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define BRD_SQ_NUM 120
#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256
#define NOMOVE 0
#define MAXDEPTH 64
#define MATE (INF_BOUND - MAXDEPTH)
#define INF_BOUND 30000

extern U64 SetMask[64];
extern U64 ClearMask[64];

namespace BalouxEngine {

	class Board;
	
	struct Move {
		int move;
		int score;
	};

	struct MoveList {
		Move moves[MAXPOSITIONMOVES];
		int count;
	};

	struct SearchInfo {
		int startTime;
		int endTime;
		int depth;
		int depthSet;
		int timeSet;
		int movesToGo;
		int infinite;

		long nodes;

		bool quit = false;
		bool stopped = false;

		float fh;
		float fhf;
	};

	struct SearchThreadData {
		Board* board;
		SearchInfo* info;
	};

	struct HashEntry {
		U64 posKey;
		int move;
		int score;
		int depth;
		int flags;
		int age;
	};

	enum HashFlags { HF_NONE, HF_ALPHA, HF_BETA, HF_EXACT };

	struct HashTableContainer {
		HashEntry* pTable;
		int newWrite;
		int hit;
		int cut;
		int overWrite;
		int currentAge;
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
	constexpr int Mirror64[64] = {
56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
	};


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
#define MIRROR64(sq) (Mirror64[(sq)])
#define INCHECK(s) (isSquareAttacked(KingSq[(s)], (s) ^ 1))

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
	};*/