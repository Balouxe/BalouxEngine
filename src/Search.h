#pragma once
#include "Types.h"

namespace BalouxEngine {

	class Board;

	class PVTable {
	public:
		PVTable(Board* board);

		struct PVEntry {
			U64 posKey;
			int move;
		};

		void InitPvTable();
		void ClearPvTable();
		int ProbePvTable();

		void StorePvMove(const int move);

		int GetPvLine(const int depth);


	public:
		PVEntry* pTable;
		int numEntries;

	private:
		Board* m_board;
	};

	class Search {
	public:
		Search(Board* board);

		inline void SetSearchInfo(SearchInfo searchInfo) { info = searchInfo; }
		void SearchPosition();
		bool isRepetition();

	private:
		void CheckUp();
		void ClearForSearch();

		int AlphaBeta(int alpha, int beta, int depth, bool doNull);
		int Quiescence(int alpha, int beta);

		void PickNextMove(int moveNum, MoveList* moveList);

		SearchInfo info;
		Board* m_board;

	};

}