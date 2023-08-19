#include "Search.h"
#include "Board.h"
#include "Evaluate.h"
#include "Hash.h"

namespace BalouxEngine {

	constexpr int HashSize = 0x100000 * 16;

	HashTable::HashTable() {
		table = HashTableContainer();
	}

	void HashTable::InitHashTable() {
		numEntries = HashSize / sizeof(HashEntry);
		numEntries -= 2;
		free(table.pTable);
		table.pTable = (HashEntry*) malloc(numEntries * sizeof(HashEntry));
		ClearHashTable();
	}

	void HashTable::ClearHashTable() {
		HashEntry* entry;

		for (entry = table.pTable; entry < table.pTable + numEntries; entry++) {
			entry->posKey = 0ULL;
			entry->move = NOMOVE;
			entry->depth = 0;
			entry->score = 0;
			entry->flags = 0;
			entry->age = 0;
		}

		table.currentAge = 0;
		table.newWrite = 0;
		table.overWrite = 0;
	}

	void HashTable::StoreHashEntry(Board* board, const int move, int score, int flags, int depth) {
		int i = board->posKey % numEntries;
		assert(i >= 0 && i <= numEntries - 1);

		bool replace = false;

		if (table.pTable[i].posKey == 0) {
			table.newWrite++;
			replace = true;
		}
		else {
			if (table.pTable[i].age < table.currentAge || table.pTable[i].depth <= depth) {
				replace = true;
			}
		}

		if (replace == false) return;

		if (score > MATE) score += board->ply;
		else if (score < -MATE) score -= board->ply;

		table.pTable[i].score = score;
		table.pTable[i].flags = flags;
		table.pTable[i].posKey = board->posKey;
		table.pTable[i].depth = depth;
		table.pTable[i].move = move;

	}

	int HashTable::ProbeHashEntry(Board* board, int* move, int* score, int alpha, int beta, int depth) {
		int i = board->posKey % numEntries;
		assert(i >= 0 && i <= numEntries - 1);

		if (table.pTable[i].posKey == board->posKey) {
			*move = table.pTable[i].move;
			if (table.pTable[i].depth >= depth) {
				table.hit++;

				*score = table.pTable[i].score;
				if (*score > MATE) *score -= board->ply;
				else if (*score < -MATE) *score += board->ply;

				switch (table.pTable[i].flags) {
					case HF_ALPHA:
						if (*score <= alpha) {
							*score = alpha;
							return true;
						}
						break;
					case HF_BETA:
						if (*score >= beta) {
							*score = beta;
							return true;
						}
						break;
					case HF_EXACT:
						return true;
						break;
					default:
						assert(false);
						break;
				}
			}
		}
		
		return false;
	}

	int HashTable::GetPvLine(Board* board, const int depth) {
		assert(depth < MAXDEPTH);

		int move = ProbePvMove(board);
		int count = 0;

		MoveGenerator moveGen(board);

		while (move != NOMOVE && count < depth) {
			assert(count < MAXDEPTH);

			if (moveGen.MoveExists(move)) {
				board->MakeMove(move);
				board->PvArray[count++] = move;
			}
			else {
				break;
			}
			move = ProbePvMove(board);
		}

		while (board->ply > 0) {
			board->TakeMove();
		}

		return count;
	}

	int HashTable::ProbePvMove(Board* board) {

		int index = board->posKey % Utils::globalHashTable->numEntries;
		assert(index >= 0 && index <= Utils::globalHashTable->numEntries - 1);

		if (Utils::globalHashTable->table.pTable[index].posKey == board->posKey) {
			return Utils::globalHashTable->table.pTable[index].move;
		}

		return NOMOVE;
	}

	Search::Search(Board* board) : m_board(board){}

	int Search::SearchPosition_Thread(void* data) {
		SearchThreadData* searchData = (SearchThreadData*)data;
		Board* board = (Board*)malloc(sizeof(Board));

		memcpy(board, searchData->board, sizeof(Board));
		Search search = Search(board);
		search.SetSearchInfo(searchData->info);

		search.SearchPosition();

		free(board);
		return 0;
	}

	void Search::SearchPosition() {
		int bestMove = NOMOVE;
		int bestScore = -INF_BOUND;
		int currentDepth = 0;
		int pvMoves = 0;
		int pvNum = 0;
		ClearForSearch();

		for (currentDepth = 1; currentDepth <= info->depth; ++currentDepth) {
			bestScore = AlphaBeta(-INF_BOUND, INF_BOUND, currentDepth, true);

			if (info->stopped == true) {
				break;
			}

			pvMoves = Utils::globalHashTable->GetPvLine(m_board, currentDepth);
			bestMove = m_board->PvArray[0];
			// std::cout << "Depth: " << currentDepth << ", score : " << bestScore << ", move: " << Utils::MoveToString(bestMove) << ", nodes : " << info->nodes << std::endl;
			printf("info score cp %d depth %d nodes %ld time %d ",
				bestScore, currentDepth, info->nodes, Utils::GetTimeInMs() - info->startTime);

			printf("pv");
			for (pvNum = 0; pvNum < pvMoves; ++pvNum) {
				printf(" %s", Utils::MoveToString(m_board->PvArray[pvNum]));
			}
			// std::cout << "\nOrdering : " << info->fhf / info->fh << "\n";
			printf("\n");

		}

		printf("bestmove %s\n", Utils::MoveToString(bestMove));
	}

	int Search::AlphaBeta(int alpha, int beta, int depth, bool doNull) {
		assert(m_board->CheckBoard());

		if (depth == 0) {
			return Quiescence(alpha, beta);
			// info->nodes++;
			// return Evaluation::EvalPosition(m_board);
		}

		if ((info->nodes & 2047) == 0) {
			CheckUp();
		}

		info->nodes++;

		if ((isRepetition() || m_board->fiftyMove >= 100) && m_board->ply) {
			return 0;
		}

		if (m_board->ply > 64) /*[[unlikely]]*/ {
			return Evaluation::EvalPosition(m_board);
		}

		bool InCheck = m_board->isSquareAttacked(m_board->KingSq[m_board->m_side], m_board->m_side ^ 1);

		if (InCheck) {
			depth++;
		}

		int Score = -INF_BOUND;

		if (doNull && !InCheck && m_board->ply && (m_board->bigPieces[m_board->m_side] > 1) && depth >= 4) {
			m_board->MakeNullMove();
			Score = -AlphaBeta(-beta, -beta + 1, depth - 4, false);
			m_board->TakeNullMove();
			if (info->stopped) {
				return 0;
			}
			if (Score >= beta && abs(Score) < MATE) {
				return beta;
			}
		}

		MoveGenerator moveGen(m_board);
		moveGen.GenerateAllMoves();

		int moveNum = 0;
		int legal = 0;
		int OldAlpha = alpha;
		int BestMove = NOMOVE;
		int BestScore = -INF_BOUND;
		Score = -INF_BOUND;
		int PvMove = NOMOVE;

		if (Utils::globalHashTable->ProbeHashEntry(m_board, &PvMove, &Score, alpha, beta, depth)) {
			Utils::globalHashTable->table.cut++;
			return Score;
		}

		for (moveNum = 0; moveNum < moveGen.GetMoveList()->count; ++moveNum) {

			PickNextMove(moveNum, moveGen.GetMoveList());

			if (!m_board->MakeMove(moveGen.GetMoveList()->moves[moveNum].move)) {
				continue;
			}

			legal++;
			Score = -AlphaBeta(-beta, -alpha, depth - 1, true);
			m_board->TakeMove();

			if (info->stopped == true) {
				return 0;
			}

			if (Score > BestScore) {
				BestScore = Score;
				BestMove = moveGen.GetMoveList()->moves[moveNum].move;
				if (Score > alpha) {
					if (Score >= beta) {
						if (legal == 1) {
							info->fhf++;
						}
						info->fh++;

						if (!(moveGen.GetMoveList()->moves[moveNum].move & MOVEFLAGCAP)) {
							m_board->searchKillers[1][m_board->ply] = m_board->searchKillers[0][m_board->ply];
							m_board->searchKillers[0][m_board->ply] = moveGen.GetMoveList()->moves[moveNum].move;
						}

						Utils::globalHashTable->StoreHashEntry(m_board, BestMove, beta, HF_BETA, depth);

						return beta;
					}
					alpha = Score;
					if (!(moveGen.GetMoveList()->moves[moveNum].move & MOVEFLAGCAP)) {
						m_board->searchHistory[m_board->pieces[FROM(BestMove)]][TO(BestMove)] += depth;
					}
				}
			}

		}

		if (legal == 0) {
			if (InCheck) {
				return -MATE + m_board->ply;
			}
			else {
				return 0;
			}
		}

		if (alpha != OldAlpha) {
			Utils::globalHashTable->StoreHashEntry(m_board, BestMove, BestScore, HF_EXACT, depth);
		}
		else {
			Utils::globalHashTable->StoreHashEntry(m_board, BestMove, alpha, HF_ALPHA, depth);
		}

		return alpha;
	}

	int Search::Quiescence(int alpha, int beta) {
		assert(m_board->CheckBoard());

		if ((info->nodes & 2047) == 0) {
			CheckUp();
		}

		info->nodes++;

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
		Score = -INF_BOUND;
		// int PvMove = m_board->GetPVTable().ProbePvTable();

		for (moveNum = 0; moveNum < moveGen.GetMoveList()->count; ++moveNum) {

			PickNextMove(moveNum, moveGen.GetMoveList());

			if (!m_board->MakeMove(moveGen.GetMoveList()->moves[moveNum].move)) {
				continue;
			}

			legal++;
			Score = -Quiescence(-beta, -alpha);
			m_board->TakeMove();

			if (info->stopped == true) {
				return 0;
			}

			if (Score > alpha) {
				if (Score >= beta) {
					if (legal == 1) {
						info->fhf++;
					}
					info->fh++;

					return beta;
				}
				alpha = Score;
				BestMove = moveGen.GetMoveList()->moves[moveNum].move;
			}

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

		Utils::globalHashTable->table.overWrite = 0;
		Utils::globalHashTable->table.hit = 0;
		Utils::globalHashTable->table.cut = 0;

		m_board->ply = 0;

		info->startTime = Utils::GetTimeInMs();
		info->stopped = false;
		info->nodes = 0;
		info->fh = 0.0f;
		info->fhf = 0.0f;
		Utils::globalHashTable->table.currentAge++;
	}

	void Search::CheckUp() {
		// checkup
		if (info->timeSet == true && Utils::GetTimeInMs() > info->endTime) {
			info->stopped = true;
		}
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

		for (i = moveNum; i < moveList->count; ++i) {
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