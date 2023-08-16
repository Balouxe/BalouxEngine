#include "Perft.h"
#include "../Board.h"

namespace BalouxEngine {

	Perft::Perft(Board* board) : leafNodes(0l), m_board(board) {

	}

	void Perft::DoPerft(int depth) {

		assert(m_board->CheckBoard());

		if (depth == 0) {
			leafNodes++;
			return;
		}

		MoveGenerator moveGen = MoveGenerator(m_board);
		moveGen.GenerateAllMoves();

		int MoveNum = 0;
		for (MoveNum = 0; MoveNum < moveGen.GetMoveList()->count; ++MoveNum) {

			if (!m_board->MakeMove(moveGen.GetMoveList()->moves[MoveNum].move)) {
				continue;
			}
			DoPerft(depth - 1);
			m_board->TakeMove();
		}

		return;
	}

	void Perft::DoPerftTest(int depth) {

		assert(m_board->CheckBoard());

		m_board->PrintBoard();
		printf("\nStarting Test To Depth:%d\n", depth);
		leafNodes = 0;
		// int start = GetTimeMs();
		MoveGenerator moveGen = MoveGenerator(m_board);
		moveGen.GenerateAllMoves();

		int move;
		int MoveNum;
		for (MoveNum = 43; MoveNum < moveGen.GetMoveList()->count; ++MoveNum) {
			move = moveGen.GetMoveList()->moves[MoveNum].move;
			if (!m_board->MakeMove(move)) {
				continue;
			}
			long cumnodes = leafNodes;
			DoPerft(depth - 1);
			m_board->TakeMove();
			long oldnodes = leafNodes - cumnodes;
			printf("move %d : %s : %ld\n", MoveNum + 1, Utils::MoveToString(move), oldnodes);
		}

		printf("\nTest Complete : %ld nodes visited\n", leafNodes);

		return;
	}
}