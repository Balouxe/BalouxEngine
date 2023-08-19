#pragma once
#include "Types.h"

#include <atomic>
#include <mutex>

namespace BalouxEngine {

	class Board;

	class HashTable {
	public:
		HashTable();

		void InitHashTable();
		void ClearHashTable();
		int ProbeHashEntry(Board* board, int* move, int* score, int alpha, int beta, int depth);

		void StoreHashEntry(Board* board, const int move, int score, int flags, int depth);

		int GetPvLine(Board* board, const int depth);
		int ProbePvMove(Board* board);

	public:
		HashTableContainer table;
		int numEntries;
	};

	class Search {
	public:
		Search(Board* board);

		inline void SetSearchInfo(SearchInfo* searchInfo) { info = searchInfo; }
		void SearchPosition();
		static int SearchPosition_Thread(void* data);
		bool isRepetition();

		inline Board* getBoard() const { return m_board; }
		inline void setBoard(Board* board) { m_board = board; }
	private:
		void CheckUp();
		void ClearForSearch();

		int AlphaBeta(int alpha, int beta, int depth, bool doNull);
		int Quiescence(int alpha, int beta);

		void PickNextMove(int moveNum, MoveList* moveList);

		SearchInfo* info;
		Board* m_board;

	};

}