#include "Evaluate.h"
#include "Board.h"

#include "data.h"

namespace BalouxEngine {

	constexpr int PawnTable[64] = {
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
	};

	constexpr int KnightTable[64] = {
	0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
	0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
	0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
	0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
	5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
	5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
	0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
	};

	constexpr int BishopTable[64] = {
	0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
	0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
	0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
	0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
	0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
	0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
	0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
	};

	constexpr int RookTable[64] = {
	0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
	0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
	0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
	0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
	0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
	0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
	25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
	0	,	0	,	5	,	10	,	10	,	5	,	0	,	0
	};
	
	constexpr int KingEndgame[64] = {
		-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	,
		-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
		0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
		0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
		0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
		0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
		-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
		-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50
	};

	constexpr int KingMiddlegame[64] = {
		5	,	10	,	5	,	-10	,	-10	,	0	,	20	,	5	,
		-15	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-15	,
		-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,
		-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
		-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
		-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
		-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
		-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70
	};

	U64 Evaluation::FileBBMask[8];
	U64 Evaluation::RankBBMask[8];
	U64 Evaluation::BlackPassedMask[64];
	U64 Evaluation::WhitePassedMask[64];
	U64 Evaluation::IsolatedMask[64];

	constexpr int PawnIsolatedPenalty = -10;
	constexpr int PawnPassedBonus[8] = { 0, 5, 10, 20, 35, 100, 200 };
	constexpr int RookOpenFileBonus = 10;
	constexpr int RookSemiOpenFileBonus = 5;
	constexpr int QueenOpenFileBonus = 5;
	constexpr int QueenSemiOpenFileBonus = 3;
	constexpr int BishopPairBonus = 20;

	bool Evaluation::MaterialDraw(const Board* pos) {

		assert(pos->CheckBoard());

		if (!pos->pieceNumber[WhiteRook] && !pos->pieceNumber[BlackRook] && !pos->pieceNumber[WhiteQueen] && !pos->pieceNumber[BlackQueen]) {
			if (!pos->pieceNumber[BlackBishop] && !pos->pieceNumber[WhiteBishop]) {
				if (pos->pieceNumber[WhiteKnight] < 3 && pos->pieceNumber[BlackKnight] < 3) { return true; }
			}
			else if (!pos->pieceNumber[WhiteKnight] && !pos->pieceNumber[BlackKnight]) {
				if (abs(pos->pieceNumber[WhiteBishop] - pos->pieceNumber[BlackBishop]) < 2) { return true; }
			}
			else if ((pos->pieceNumber[WhiteKnight] < 3 && !pos->pieceNumber[WhiteBishop]) || (pos->pieceNumber[WhiteBishop] == 1 && !pos->pieceNumber[WhiteKnight])) {
				if ((pos->pieceNumber[BlackKnight] < 3 && !pos->pieceNumber[BlackBishop]) || (pos->pieceNumber[BlackBishop] == 1 && !pos->pieceNumber[BlackKnight])) { return true; }
			}
		}
		else if (!pos->pieceNumber[WhiteQueen] && !pos->pieceNumber[BlackQueen]) {
			if (pos->pieceNumber[WhiteRook] == 1 && pos->pieceNumber[BlackRook] == 1) {
				if ((pos->pieceNumber[WhiteKnight] + pos->pieceNumber[WhiteBishop]) < 2 && (pos->pieceNumber[BlackKnight] + pos->pieceNumber[BlackBishop]) < 2) { return true; }
			}
			else if (pos->pieceNumber[WhiteRook] == 1 && !pos->pieceNumber[BlackRook]) {
				if ((pos->pieceNumber[WhiteKnight] + pos->pieceNumber[WhiteBishop] == 0) && (((pos->pieceNumber[BlackKnight] + pos->pieceNumber[BlackBishop]) == 1) || ((pos->pieceNumber[BlackKnight] + pos->pieceNumber[BlackBishop]) == 2))) { return true; }
			}
			else if (pos->pieceNumber[BlackRook] == 1 && !pos->pieceNumber[WhiteRook]) {
				if ((pos->pieceNumber[BlackKnight] + pos->pieceNumber[BlackBishop] == 0) && (((pos->pieceNumber[WhiteKnight] + pos->pieceNumber[WhiteBishop]) == 1) || ((pos->pieceNumber[WhiteKnight] + pos->pieceNumber[WhiteBishop]) == 2))) { return true; }
			}
		}
		return false;
	}

	void Evaluation::InitEvalMasks() {
		int sq, tsq, r, f;
		for (sq = 0; sq < 8; ++sq) {
			FileBBMask[sq] = 0ULL;
			RankBBMask[sq] = 0ULL;
		}

		for (r = RANK_8; r >= RANK_1; --r) {
			for (f = FILE_A; f <= FILE_H; ++f) {
				sq = r * 8 + f;
				FileBBMask[f] |= (1ULL << sq);
				RankBBMask[r] |= (1ULL << sq);
			}
		}

		for (sq = 0; sq < 64; ++sq) {
			WhitePassedMask[sq] = 0ULL;
			BlackPassedMask[sq] = 0ULL;
			IsolatedMask[sq] = 0ULL;
		}

		for (sq = 0; sq < 64; ++sq) {
			tsq = sq + 8;

			while(tsq < 64) {
				WhitePassedMask[sq] |= (1ULL << tsq);
				tsq += 8;
			}

			tsq = sq - 8;

			while (tsq >= 0) {
				BlackPassedMask[sq] |= (1ULL << tsq);
				tsq -= 8;
			}

			if (Utils::FilesBrd[SQ120(sq)] > FILE_A) {
				IsolatedMask[sq] |= FileBBMask[Utils::FilesBrd[SQ120(sq)] - 1];

				tsq = sq + 7;

				while (tsq < 64) {
					WhitePassedMask[sq] |= (1ULL << tsq);
					tsq += 8;
				}

				tsq = sq - 9;

				while (tsq >= 0) {
					BlackPassedMask[sq] |= (1ULL << tsq);
					tsq -= 8;
				}
			}

			if (Utils::FilesBrd[SQ120(sq)] < FILE_H) {
				IsolatedMask[sq] |= FileBBMask[Utils::FilesBrd[SQ120(sq)] + 1];

				tsq = sq + 9;

				while (tsq < 64) {
					WhitePassedMask[sq] |= (1ULL << tsq);
					tsq += 8;
				}

				tsq = sq - 7;

				while (tsq >= 0) {
					BlackPassedMask[sq] |= (1ULL << tsq);
					tsq -= 8;
				}
			}
		}



	}

#define ENDGAME_MAT (1 * PieceVal[WhiteRook] + 2 * PieceVal[WhiteKnight])

	int Evaluation::EvalPosition(Board* board) {

		int piece;
		int pieceNum;
		int square;
		int score = board->GetWhiteMaterial() - board->GetBlackMaterial();

		if (MaterialDraw(board) && !board->pieceNumber[WhitePawn] && !board->pieceNumber[BlackPawn]) {
			return 0;
		}

		// Pawns
		piece = WhitePawn;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));
			score += PawnTable[SQ64(square)];

			if ((IsolatedMask[SQ64(square)] & board->pawns[White].Get()) == 0ULL) {
				score += PawnIsolatedPenalty;
			}

			if ((WhitePassedMask[SQ64(square)] & board->pawns[Black].Get()) == 0ULL) {
				score += PawnPassedBonus[Utils::RanksBrd[square]];
			}
		}

		piece = BlackPawn;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));
			score -= PawnTable[MIRROR64(SQ64(square))];

			if ((IsolatedMask[SQ64(square)] & board->pawns[Black].Get()) == 0ULL) {
				score -= PawnIsolatedPenalty;
			}

			if ((BlackPassedMask[SQ64(square)] & board->pawns[White].Get()) == 0ULL) {
				score -= PawnPassedBonus[7 - Utils::RanksBrd[square]];
			}
		}

		// Knights
		piece = WhiteKnight;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));
			score += KnightTable[SQ64(square)];
		}

		piece = BlackKnight;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));
			score -= KnightTable[MIRROR64(SQ64(square))];
		}

		// Bishops
		piece = WhiteBishop;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));
			score += BishopTable[SQ64(square)];
		}

		piece = BlackBishop;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));
			score -= BishopTable[MIRROR64(SQ64(square))];
		}

		if (board->pieceNumber[WhiteBishop >= 2]) {
			score += BishopPairBonus;
		}
		if (board->pieceNumber[BlackBishop >= 2]) {
			score -= BishopPairBonus;
		}

		// Rooks
		piece = WhiteRook;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));
			score += RookTable[SQ64(square)];

			if ((board->pawns[Both].Get() & FileBBMask[Utils::FilesBrd[square]]) == 0) {
				score += RookOpenFileBonus;
			}
			else if ((board->pawns[White].Get() & FileBBMask[Utils::FilesBrd[square]]) == 0) {
				score += RookSemiOpenFileBonus;
			}
		}

		piece = BlackRook;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));
			score -= RookTable[MIRROR64(SQ64(square))];

			if ((board->pawns[Both].Get() & FileBBMask[Utils::FilesBrd[square]]) == 0) {
				score -= RookOpenFileBonus;
			}
			else if ((board->pawns[Black].Get() & FileBBMask[Utils::FilesBrd[square]]) == 0) {
				score -= RookSemiOpenFileBonus;
			}
		}
		
		// Queens
		piece = WhiteQueen;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));

			if ((board->pawns[Both].Get() & FileBBMask[Utils::FilesBrd[square]]) == 0) {
				score += QueenOpenFileBonus;
			}
			else if ((board->pawns[White].Get() & FileBBMask[Utils::FilesBrd[square]]) == 0) {
				score += QueenSemiOpenFileBonus;
			}
		}

		piece = BlackQueen;
		for (pieceNum = 0; pieceNum < board->pieceNumber[piece]; ++pieceNum) {
			square = board->pieceList[piece][pieceNum];
			assert(Utils::SquareOnBoard(square));

			if ((board->pawns[Both].Get() & FileBBMask[Utils::FilesBrd[square]]) == 0) {
				score -= QueenOpenFileBonus;
			}
			else if ((board->pawns[Black].Get() & FileBBMask[Utils::FilesBrd[square]]) == 0) {
				score -= QueenSemiOpenFileBonus;
			}
		}

		// Kings
		piece = WhiteKing;
		square = board->pieceList[piece][0];
		if (board->material[Black] < ENDGAME_MAT) {
			score += KingEndgame[SQ64(square)];
		}
		else {
			score += KingMiddlegame[SQ64(square)];
		}

		piece = BlackKing;
		square = board->pieceList[piece][0];
		if (board->material[White] < ENDGAME_MAT) {
			score += KingEndgame[MIRROR64(SQ64(square))];
		}
		else {
			score += KingMiddlegame[MIRROR64(SQ64(square))];
		}

		if (board->GetSide() == White) {
			return score;
		}
		else {
			return -score;
		}

		return 0;

	}

}