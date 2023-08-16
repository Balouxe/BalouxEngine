#pragma once
#include "Types.h"
#include "Board.h"

namespace BalouxEngine {

	class Hash {
	public:
		static void InitHashKeys();

		static U64 GeneratePosKey(const Board* position);

		static U64 m_PieceKeys[13][120];
		static U64 m_SideKey;
		static U64 m_CastleKeys[16];
	};

}