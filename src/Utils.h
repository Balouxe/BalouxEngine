#pragma once
#include "Types.h"

namespace BalouxEngine {

	class Board;

	class Utils {
	public:
		static void Init64To120();
		static void InitFilesRanksBrd();

		static char* SquareToString(const int square);
		static char* MoveToString(const int move);
		static void PrintMoveList(const MoveList* moveList);

		inline static bool SquareOnBoard(const int square) { return FilesBrd[square] == OFFBOARD ? false : true; }
		inline static bool SideValid(const int side) { return (side == White || side == Black) ? true : false; }
		inline static bool FileRankValid(const int fr) { return (fr >= 0 && fr <= 7) ? true : false; }
		inline static bool PieceValidEmpty(const int piece) { return (piece >= PieceNone && piece <= BlackKing) ? true : false; }
		inline static bool PieceValid(const int piece) { return (piece >= WhitePawn && piece <= BlackKing) ? true : false; }

		inline static int Sq120ToSq64(int sq120) { return m_Sq120ToSq64[sq120]; }
		inline static  int Sq64ToSq120(int sq64) { return m_Sq64ToSq120[sq64]; }

		static int ParseMove(char* move, Board* board);

		static int GetTimeInMs();
	private:
		static int m_Sq120ToSq64[120];
		static int m_Sq64ToSq120[64];
	public:
		static int FilesBrd[120];
		static int RanksBrd[120];
	};

}