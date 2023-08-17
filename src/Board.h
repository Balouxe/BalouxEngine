#pragma once
#include <vector>
#include <string>

#include "Types.h"
#include "Utils.h"
#include "Bitboard.h"
#include "MoveGenerator.h"
#include "Search.h"


namespace BalouxEngine {

	struct Undo {
		int move;
		int castlingPerm;
		int enPassant;
		int fiftyMove;
		U64 posKey;
	};

	class Board {
	public:
		Board();

		bool MakeMove(int move);
		void TakeMove();

		void ResetBoard();
		void PrintBoard();

		void ParseFEN(std::string FEN);

		void UpdateListsMaterials();
		bool CheckBoard();

		bool isSquareAttacked(const int square, const int side);

		inline int GetPieceAtIndex(int index) const { return pieces[index]; }
		inline int GetSide() const { return m_side; }
		inline int GetEnPassantSquare() const { return enPassant; }
		inline int GetCastlingRights() const { return castlingPermission; }
		inline Bitboard GetWhitePawns() const { return pawns[White]; }
		inline Bitboard GetBlackPawns() const { return pawns[Black]; }
		inline Bitboard GetBothPawns() const { return pawns[Both]; }
		inline PVTable& GetPVTable() { return pvTable; }
		inline int GetWhiteMaterial() const { return material[White]; }
		inline int GetBlackMaterial() const { return material[Black]; }
	private:
		void ClearPiece(const int square);
		void AddPiece(const int square, const int piece);
		void MovePiece(const int from, const int to);

		friend class MoveGenerator;
		friend class Search;
		friend class PVTable;
		friend class Evaluation;

		int pieces[BRD_SQ_NUM];
		Bitboard pawns[3];

		int KingSq[2];

		int m_side;
		int enPassant;
		int fiftyMove;

		int ply;
		int hisPly;

		U64 posKey;

		int bigPieces[2];
		int majorPieces[2];
		int minorPieces[2];
		int material[2];

		int pieceNumber[13]; // 13 being the number of pieces, had to make this public because you can't return an arrray in c++
		int pieceList[13][10]; // 13 again, and 10 being the max amount of same piece on the board

		int castlingPermission;

		Undo history[MAXGAMEMOVES];

		PVTable pvTable;

		int searchHistory[13][120];
		int searchKillers[2][MAXDEPTH];
	public:
		int PvArray[MAXDEPTH];
	};

}