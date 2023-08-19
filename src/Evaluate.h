#pragma once
#include "Types.h"

namespace BalouxEngine {

	class Board;

	class Evaluation {
	public:
		static int EvalPosition(Board* board);

		static void InitEvalMasks();

	public:
		static bool MaterialDraw(const Board* pos);

		static U64 FileBBMask[8];
		static U64 RankBBMask[8];
		static U64 BlackPassedMask[64];
		static U64 WhitePassedMask[64];
		static U64 IsolatedMask[64];
	};

}