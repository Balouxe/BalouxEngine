#include "Search.h"
#include "Board.h"
#include "Evaluate.h"

namespace BalouxEngine {

	constexpr int PvSize = 0x100000 * 2;

	PVTable::PVTable(Board* board) : m_board(board) {}

	void PVTable::InitPvTable() {
		numEntries = PvSize / sizeof(PVEntry);
		numEntries -= 2;
		free(pTable);
		pTable = (PVEntry*) malloc(numEntries * sizeof(PVEntry));
		ClearPvTable();
	}

	void PVTable::ClearPvTable() {
		PVEntry* entry;

		for (entry = pTable; entry < pTable + numEntries; entry++) {
			entry->posKey = 0ULL;
			entry->move = 0;
		}
	}

	void PVTable::StorePvMove(const int move) {
		int i = m_board->posKey % numEntries;
		assert(i >= 0 && i <= numEntries - 1);

		pTable[i].move = move;
		pTable[i].posKey = m_board->posKey;
	}

	int PVTable::ProbePvTable() {
		int i = m_board->posKey % numEntries;
		assert(i >= 0 && i <= numEntries - 1);

		if (pTable[i].posKey == m_board->posKey) {
			return pTable[i].move;
		}
		else {
			return NOMOVE;
		}
	}

	int PVTable::GetPvLine(const int depth) {
		assert(depth < MAXDEPTH);

		int move = ProbePvTable();
		int count = 0;

		MoveGenerator moveGen(m_board);

		while (move != NOMOVE && count < depth) {
			assert(count < MAXDEPTH);

			if (moveGen.MoveExists(move)) {
				m_board->MakeMove(move);
				m_board->PvArray[count++] = move;
			}
			else {
				break;
			}
			move = ProbePvTable();
		}

		while (m_board->ply > 0) {
			m_board->TakeMove();
		}

		return count;
	}

	Search::Search(Board* board) : m_board(board){}

	void Search::SearchPosition() {
		int bestMove = NOMOVE;
		int bestScore = -INFINITE;
		int currentDepth = 0;
		int pvMoves = 0;
		int pvNum = 0;
		ClearForSearch();

		for (currentDepth = 1; currentDepth <= info.depth; ++currentDepth) {
			bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, true);
			pvMoves = m_board->GetPVTable().GetPvLine(currentDepth);
			bestMove = m_board->PvArray[0];

			std::cout << "Depth: " << currentDepth << ", score : " << bestScore << ", move: " << Utils::MoveToString(bestMove) << ", nodes : " << info.nodes << std::endl;

			for (pvNum = 0; pvNum < pvMoves; ++pvNum) {
				std::cout << Utils::MoveToString(m_board->PvArray[pvNum]) << " ";
			}
			std::cout << "\nOrdering : " << info.fhf / info.fh << "\n";
		}

	}

	int Search::AlphaBeta(int alpha, int beta, int depth, bool doNull) {
		assert(m_board->CheckBoard());

		if (depth == 0) {
			// return Quiescence(alpha, beta);
			info.nodes++;
			return Evaluation::EvalPosition(m_board);
		}

		info.nodes++;

		if (isRepetition() || m_board->fiftyMove >= 100) {
			return 0;
		}

		if (m_board->ply > 64) /*[[unlikely]]*/ {
			return Evaluation::EvalPosition(m_board);
		}

		MoveGenerator moveGen(m_board);
		moveGen.GenerateAllMoves();

		int moveNum = 0;
		int legal = 0;
		int OldAlpha = alpha;
		int BestMove = NOMOVE;
		int Score = -INFINITE;
		int PvMove = m_board->GetPVTable().ProbePvTable();

		if (PvMove != NOMOVE) {
			for (moveNum = 0; moveNum < moveGen.GetMoveList()->count; ++moveNum) {
				if (moveGen.GetMoveList()->moves[moveNum].move == PvMove) {
					moveGen.GetMoveList()->moves[moveNum].score = 2000000;
					break;
				}
			}
		}

		for (moveNum = 0; moveNum < moveGen.GetMoveList()->count; ++moveNum) {

			PickNextMove(moveNum, moveGen.GetMoveList());

			if (!m_board->MakeMove(moveGen.GetMoveList()->moves[moveNum].move)) {
				continue;
			}

			legal++;
			Score = -AlphaBeta(-beta, -alpha, depth - 1, true);
			m_board->TakeMove();

			if (Score > alpha) {
				if (Score >= beta) {
					if (legal == 1) {
						info.fhf++;
					}
					info.fh++;

					if (!(moveGen.GetMoveList()->moves[moveNum].move & MOVEFLAGCAP)) {
						m_board->searchKillers[1][m_board->ply] = m_board->searchKillers[0][m_board->ply];
						m_board->searchKillers[0][m_board->ply] = moveGen.GetMoveList()->moves[moveNum].move;
					}

					return beta;
				}
				alpha = Score;
				BestMove = moveGen.GetMoveList()->moves[moveNum].move;
				if (!(moveGen.GetMoveList()->moves[moveNum].move & MOVEFLAGCAP)) {
					m_board->searchHistory[m_board->pieces[FROM(BestMove)]][TO(BestMove)] += depth;
				}
			}

		}

		if (legal == 0) {
			if (m_board->isSquareAttacked(m_board->KingSq[m_board->m_side], m_board->m_side ^ 1)) {
				return -MATE + m_board->ply;
			}
			else {
				return 0;
			}
		}

		if (alpha != OldAlpha) {
			m_board->GetPVTable().StorePvMove(BestMove);
		}

		return alpha;
	}

	int Search::Quiescence(int alpha, int beta) {
		assert(m_board->CheckBoard());
		info.nodes++;

		if (isRepetition() || m_board->fiftyMove >= 100) {
			return 0;
		}

		if (m_board->ply > MAXDEPTH - 1) /*[[unlikely]]*/ {
			return Evaluation::EvalPosition(m_board);
		}

		int Score = Evaluation::EvalPosition(m_board);

		if (Score >= beta) {
			return beta;
		}

		if (Score > alpha) {
			alpha = Score;
		}

		MoveGenerator moveGen(m_board);
		moveGen.GenerateAllCaptures();

		int moveNum = 0;
		int legal = 0;
		int OldAlpha = alpha;
		int BestMove = NOMOVE;
		Score = -INFINITE;
		int PvMove = m_board->GetPVTable().ProbePvTable();


		for (moveNum = 0; moveNum < moveGen.GetMoveList()->count; ++moveNum) {

			PickNextMove(moveNum, moveGen.GetMoveList());

			if (!m_board->MakeMove(moveGen.GetMoveList()->moves[moveNum].move)) {
				continue;
			}

			legal++;
			Score = -Quiescence(-beta, -alpha);
			m_board->TakeMove();

			if (Score > alpha) {
				if (Score >= beta) {
					if (legal == 1) {
						info.fhf++;
					}
					info.fh++;

					return beta;
				}
				alpha = Score;
				BestMove = moveGen.GetMoveList()->moves[moveNum].move;
			}

		}

		if (alpha != OldAlpha) {
			m_board->GetPVTable().StorePvMove(BestMove);
		}

		return alpha;
	}

	void Search::ClearForSearch() {
		int i;
		int j;

		for (i = 0; i < 13; ++i) {
			for (j = 0; j < 120; ++j) {
				m_board->searchHistory[i][j] = 0;
			}
		}

		for (i = 0; i < 2; ++i) {
			for (j = 0; j < MAXDEPTH; ++j) {
				m_board->searchKillers[i][j] = 0;
			}
		}

		m_board->GetPVTable().ClearPvTable();
		m_board->ply = 0;

		info.startTime = Utils::GetTimeInMs();
		info.stopped = false;
		info.nodes = 0;
		info.fh = 0.0f;
		info.fhf = 0.0f;
	}

	void Search::CheckUp() {
		// checkup
	}

	bool Search::isRepetition() {
		int i;

		for (i = m_board->hisPly - m_board->fiftyMove; i < m_board->hisPly - 1; ++i) {
			if (m_board->posKey == m_board->history[i].posKey) {
				return true;
			}
		}

		return false;
	}

	void Search::PickNextMove(int moveNum, MoveList* moveList) {
		Move temp;
		int i = 0;
		int bestScore = 0;
		int bestNum = moveNum;

		for (i = 0; i < moveList->count; ++i) {
			if (moveList->moves[i].score > bestScore) {
				bestScore = moveList->moves[i].score;
				bestNum = i;
			}
		}

		temp = moveList->moves[moveNum];
		moveList->moves[moveNum] = moveList->moves[bestNum];
		moveList->moves[bestNum] = temp;
	}


}