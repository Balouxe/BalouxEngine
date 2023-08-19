#pragma once
#include "Types.h"

namespace BalouxEngine {
	class Board;

	class UCI {
	public:
		static void Loop(Board* board, SearchInfo* info);
		static void ParseGo(const char* line, SearchInfo* info, Board* board);
		static void ParsePosition(const char* line, Board* board);
	private:

	};

}