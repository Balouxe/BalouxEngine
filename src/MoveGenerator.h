#pragma once
#include "Types.h"

namespace BalouxEngine {

	class Board;

	class MoveGenerator {
	public:
		MoveGenerator(Board* board);
		void GenerateAllMoves();

		bool MoveListOk();

		inline MoveList* GetMoveList() { return m_moveList; }

	private:
		void AddQuietMove(int move);
		void AddCaptureMove(int move);
		void AddEnPassantMove(int move);

		void AddWhitePawnCaptureMove(const int from, const int to, const int capture);
		void AddWhitePawnMove(const int from, const int to);

		void AddBlackPawnCaptureMove(const int from, const int to, const int capture);
		void AddBlackPawnMove(const int from, const int to);

		Board* m_board = nullptr;
		MoveList* m_moveList = nullptr;
	};

}