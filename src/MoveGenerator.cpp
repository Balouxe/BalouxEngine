#include "MoveGenerator.h"

#include "Utils.h"
#include "Board.h"
#include "data.h"

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))
// #define SQOFFBOARD(sq) (FilesBrd[(sq)]==OFFBOARD) will test performance later, inline function might be the same exact thing


constexpr int LoopSlidePiece[8] = { WhiteBishop, WhiteRook, WhiteQueen, 0, BlackBishop, BlackRook, BlackQueen, 0 };
constexpr int LoopNonSlidePiece[8] = { WhiteKnight, WhiteKing, 0, BlackKnight, BlackKing, 0 };
constexpr int LoopSlideIndex[2] = { 0, 4 };
constexpr int LoopNonSlideIndex[2] = { 0, 3 };

constexpr int PieceDir[13][8] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 }
};

constexpr int NumDir[13] = {
 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

namespace BalouxEngine {

	MoveGenerator::MoveGenerator(Board* board) : m_board(board) {
		m_moveList = new MoveList();
	}

	void MoveGenerator::AddQuietMove(int move) {
		m_moveList->moves[m_moveList->count].move = move;
		m_moveList->moves[m_moveList->count].score = 0;
		m_moveList->count++;
	}

	void MoveGenerator::AddCaptureMove(int move) {
		m_moveList->moves[m_moveList->count].move = move;
		m_moveList->moves[m_moveList->count].score = 0;
		m_moveList->count++;
	}

	void MoveGenerator::AddEnPassantMove(int move) {
		m_moveList->moves[m_moveList->count].move = move;
		m_moveList->moves[m_moveList->count].score = 0;
		m_moveList->count++;
	}

	void MoveGenerator::AddWhitePawnCaptureMove(const int from, const int to, const int capture) {
		assert(Utils::SquareOnBoard(from));
		assert(Utils::SquareOnBoard(to));

		if (Utils::RanksBrd[from] == RANK_7) {
			AddCaptureMove(MOVE(from, to, capture, WhiteRook, 0));
			AddCaptureMove(MOVE(from, to, capture, WhiteBishop, 0));
			AddCaptureMove(MOVE(from, to, capture, WhiteQueen, 0));
			AddCaptureMove(MOVE(from, to, capture, WhiteKnight, 0));
		}
		else {
			AddCaptureMove(MOVE(from, to, capture, PieceNone, 0));
		}
	}

	void MoveGenerator::AddWhitePawnMove(const int from, const int to) {
		assert(Utils::SquareOnBoard(from));
		assert(Utils::SquareOnBoard(to));
		if (Utils::RanksBrd[from] == RANK_7) {
			AddQuietMove(MOVE(from, to, PieceNone, WhiteQueen, 0));
			AddQuietMove(MOVE(from, to, PieceNone, WhiteRook, 0));
			AddQuietMove(MOVE(from, to, PieceNone, WhiteBishop, 0));
			AddQuietMove(MOVE(from, to, PieceNone, WhiteKnight, 0));
		}
		else {
			AddQuietMove(MOVE(from, to, PieceNone, PieceNone, 0));
		}
	}

	void MoveGenerator::AddBlackPawnCaptureMove(const int from, const int to, const int capture) {
		assert(Utils::SquareOnBoard(from));
		assert(Utils::SquareOnBoard(to));
		if (Utils::RanksBrd[from] == RANK_2) {
			AddCaptureMove(MOVE(from, to, capture, BlackQueen, 0));
			AddCaptureMove(MOVE(from, to, capture, BlackRook, 0));
			AddCaptureMove(MOVE(from, to, capture, BlackBishop, 0));
			AddCaptureMove(MOVE(from, to, capture, BlackKnight, 0));
		}
		else {
			AddCaptureMove(MOVE(from, to, capture, PieceNone, 0));
		}
	}

	void MoveGenerator::AddBlackPawnMove(const int from, const int to) {
		assert(Utils::SquareOnBoard(from));
		assert(Utils::SquareOnBoard(to));
		if (Utils::RanksBrd[from] == RANK_2) {
			AddQuietMove(MOVE(from, to, PieceNone, BlackRook, 0));
			AddQuietMove(MOVE(from, to, PieceNone, BlackQueen, 0));
			AddQuietMove(MOVE(from, to, PieceNone, BlackBishop, 0));
			AddQuietMove(MOVE(from, to, PieceNone, BlackKnight, 0));
		}
		else {
			AddQuietMove(MOVE(from, to, PieceNone, PieceNone, 0));
		}
	}

	void MoveGenerator::GenerateAllMoves() {
		assert(m_board->CheckBoard());
		m_moveList->count = 0;

		int piece = PieceNone;
		int side = m_board->GetSide();
		int square = 0;
		int t_square = 0;
		int pieceNumber = 0;

		int dir = 0;
		int i = 0;
		int pieceIndex = 0;

		if (side == White) {
			// Pawns

			for (pieceNumber = 0; pieceNumber < m_board->pieceNumber[WhitePawn]; ++pieceNumber) {
				square = m_board->pieceList[WhitePawn][pieceNumber];
				assert(Utils::SquareOnBoard(square));

				if (m_board->GetPieceAtIndex(square + 10) == PieceNone) {
					AddWhitePawnMove(square, square + 10);
					if (Utils::RanksBrd[square] == RANK_2 && m_board->GetPieceAtIndex(square + 20) == PieceNone) {
						AddQuietMove(MOVE(square, square + 20, PieceNone, PieceNone, MOVEFLAGPS));
					}
				}

				if (Utils::SquareOnBoard(square + 9) && PieceCol[m_board->GetPieceAtIndex(square + 9)] == Black) {
					AddWhitePawnCaptureMove(square, square + 9, m_board->GetPieceAtIndex(square + 9));
				}

				if (Utils::SquareOnBoard(square + 11) && PieceCol[m_board->GetPieceAtIndex(square + 11)] == Black) {
					AddWhitePawnCaptureMove(square, square + 11, m_board->GetPieceAtIndex(square + 11));
				}

				if (m_board->GetEnPassantSquare() != SQ_NONE) {
					if (square + 9 == m_board->GetEnPassantSquare()) {
						AddCaptureMove(MOVE(square, square + 9, PieceNone, PieceNone, MOVEFLAGEP));
					}

					if (square + 11 == m_board->GetEnPassantSquare()) {
						AddCaptureMove(MOVE(square, square + 11, PieceNone, PieceNone, MOVEFLAGEP));
					}
				}

			}

			if (m_board->GetCastlingRights() & WHITE_OO) {
				if (m_board->GetPieceAtIndex(F1) == PieceNone 
					&& m_board->GetPieceAtIndex(G1) == PieceNone) {
					if (!m_board->isSquareAttacked(E1, Black)
						&& !m_board->isSquareAttacked(F1, Black)) {
						AddQuietMove(MOVE(E1, G1, PieceNone, PieceNone, MOVEFLAGCAS));
					}
				}
			}

			if (m_board->GetCastlingRights() & WHITE_OOO) {
				if (m_board->GetPieceAtIndex(D1) == PieceNone
					&& m_board->GetPieceAtIndex(C1) == PieceNone
					&& m_board->GetPieceAtIndex(B1) == PieceNone) {
					if (!m_board->isSquareAttacked(E1, Black)
						&& !m_board->isSquareAttacked(D1, Black)) {
						AddQuietMove(MOVE(E1, C1, PieceNone, PieceNone, MOVEFLAGCAS));
					}
				}
			}
		}
		else {

			// Pawns

			for (pieceNumber = 0; pieceNumber < m_board->pieceNumber[BlackPawn]; pieceNumber++) {
				square = m_board->pieceList[BlackPawn][pieceNumber];
				assert(Utils::SquareOnBoard(square));

				if (m_board->GetPieceAtIndex(square - 10) == PieceNone) {
					AddBlackPawnMove(square, square - 10);
					if (Utils::RanksBrd[square] == RANK_7 && m_board->GetPieceAtIndex(square - 20) == PieceNone) {
						AddQuietMove(MOVE(square, square - 20, PieceNone, PieceNone, MOVEFLAGPS));
					}
				}

				if (Utils::SquareOnBoard(square - 9) && PieceCol[m_board->GetPieceAtIndex(square - 9)] == White) {
					AddBlackPawnCaptureMove(square, square - 9, m_board->GetPieceAtIndex(square - 9));
				}

				if (Utils::SquareOnBoard(square - 11) && PieceCol[m_board->GetPieceAtIndex(square - 11)] == White) {
					AddBlackPawnCaptureMove(square, square - 11, m_board->GetPieceAtIndex(square - 11));
				}

				if (m_board->GetEnPassantSquare() != SQ_NONE) {
					if (square - 9 == m_board->GetEnPassantSquare()) {
						AddCaptureMove(MOVE(square, square - 9, PieceNone, PieceNone, MOVEFLAGEP));
					}

					if (square - 11 == m_board->GetEnPassantSquare()) {
						AddCaptureMove(MOVE(square, square - 11, PieceNone, PieceNone, MOVEFLAGEP));
					}
				}
			}

			if (m_board->GetCastlingRights() & BLACK_OO) {
				if (m_board->GetPieceAtIndex(F8) == PieceNone
					&& m_board->GetPieceAtIndex(G8) == PieceNone) {
					if (!m_board->isSquareAttacked(E8, White)
						&& !m_board->isSquareAttacked(F8, White)) {
						AddQuietMove(MOVE(E8, G8, PieceNone, PieceNone, MOVEFLAGCAS));
					}
				}
			}

			if (m_board->GetCastlingRights() & BLACK_OOO) {
				if (m_board->GetPieceAtIndex(D8) == PieceNone
					&& m_board->GetPieceAtIndex(C8) == PieceNone
					&& m_board->GetPieceAtIndex(B8) == PieceNone) {
					if (!m_board->isSquareAttacked(E8, White)
						&& !m_board->isSquareAttacked(D8, White)) {
						AddQuietMove(MOVE(E8, C8, PieceNone, PieceNone, MOVEFLAGCAS));
					}
				}
			}

		}

		// Sliding pieces

		pieceIndex = LoopSlideIndex[side];
		piece = LoopSlidePiece[pieceIndex++];

		while (piece != 0) {
			assert(Utils::PieceValid(piece));

			for (pieceNumber = 0; pieceNumber < m_board->pieceNumber[piece]; ++pieceNumber) {
				square = m_board->pieceList[piece][pieceNumber];
				assert(Utils::SquareOnBoard(square));

				for (i = 0; i < NumDir[piece]; ++i) {
					dir = PieceDir[piece][i];
					t_square = square + dir;

					while (Utils::SquareOnBoard(t_square)) {
						if (m_board->pieces[t_square] != PieceNone) {
							if (PieceCol[m_board->pieces[t_square]] == (side ^ 1)) {
								AddCaptureMove(MOVE(square, t_square, m_board->pieces[t_square], PieceNone, 0));
							}
							break;
						}
						AddQuietMove(MOVE(square, t_square, PieceNone, PieceNone, 0));
						t_square += dir;
					}

				}

			}

			piece = LoopSlidePiece[pieceIndex++];
		}

		pieceIndex = LoopNonSlideIndex[side];
		piece = LoopNonSlidePiece[pieceIndex++];

		while (piece != 0) {
			assert(Utils::PieceValid(piece));

			for (pieceNumber = 0; pieceNumber < m_board->pieceNumber[piece]; ++pieceNumber) {
				square = m_board->pieceList[piece][pieceNumber];
				assert(Utils::SquareOnBoard(square));

				for (i = 0; i < NumDir[piece]; ++i) {
					dir = PieceDir[piece][i];
					t_square = square + dir;

					if (!Utils::SquareOnBoard(t_square)) {
						continue;
					}

					if (m_board->pieces[t_square] != PieceNone) {
						if (PieceCol[m_board->pieces[t_square]] == (side ^ 1)) {
							AddCaptureMove(MOVE(square, t_square, m_board->pieces[t_square], PieceNone, 0));
						}
						continue;
					}
					AddQuietMove(MOVE(square, t_square, PieceNone, PieceNone, 0));
				}

			}

			piece = LoopNonSlidePiece[pieceIndex++];
		}

		assert(MoveListOk());
	}

	bool MoveGenerator::MoveListOk() {
			if (m_moveList->count < 0 || m_moveList->count >= MAXPOSITIONMOVES) {
				return false;
			}

			int MoveNum;
			int from = 0;
			int to = 0;
			for (MoveNum = 0; MoveNum < m_moveList->count; ++MoveNum) {
				to = TO(m_moveList->moves[MoveNum].move);
				from = FROM(m_moveList->moves[MoveNum].move);
				if (!Utils::SquareOnBoard(to) || !Utils::SquareOnBoard(from)) {
					return false;
				}
				if (!Utils::PieceValid(m_board->GetPieceAtIndex(from))) {
					m_board->PrintBoard();
					return false;
				}
			}

			return true;
	}
	
}