#pragma once
#include "Types.h"

// TODO, later: Implement SEE instead of MvvLVA

namespace BalouxEngine {

	class Board;

	class MoveGenerator {
	public:
		MoveGenerator(Board* board);
		~MoveGenerator();

		void GenerateAllMoves();
		void GenerateAllCaptures();

		static void InitMvvLva();

		bool MoveExists(const int move);

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

		static int MvvLVAScores[13][13];
	};

}