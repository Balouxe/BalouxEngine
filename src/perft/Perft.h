#pragma once
#include "../Types.h"
#include "../MoveGenerator.h"

namespace BalouxEngine {

	class Board;

	class Perft {
	public:
		Perft(Board* board);

		void DoPerft(int depth);
		void DoPerftTest(int depth);

	private:
		Board* m_board;

		long leafNodes;
	};

}