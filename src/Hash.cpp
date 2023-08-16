#include "Hash.h"
#include <stdlib.h>

#define RAND_64 ( \
	(U64)rand() | \
	(U64)rand() << 15 | \
	(U64)rand() << 30 | \
	(U64)rand() << 45 | \
	((U64)rand() & 0xf) << 60)


namespace BalouxEngine {

	U64 Hash::m_PieceKeys[13][120];
	U64 Hash::m_SideKey;
	U64 Hash::m_CastleKeys[16];

	void Hash::InitHashKeys() {
		int i = 0;
		int j = 0;
		for (i = 0; i < 13; ++i) {
			for (j = 0; j < 120; ++j) {
				m_PieceKeys[i][j] = RAND_64;
			}
		}
		m_SideKey = RAND_64;
		for (i = 0; i < 16; ++i) {
			m_CastleKeys[i] = RAND_64;
		}
	}

	U64 Hash::GeneratePosKey(const Board* position) {
		int square = 0;
		U64 finalKey = 0;
		int piece = PieceNone;

		for (square = 0; square < 120; ++square) {
			piece = position->GetPieceAtIndex(square);
			if (piece != Piece::PieceNone && piece != Square::SQ_NONE && piece != OFFBOARD) {
				assert(piece >= WhitePawn && piece <= BlackKing);
				finalKey ^= m_PieceKeys[piece][square];
			}
		}

		if (position->GetSide() == Color::White) {
			finalKey ^= m_SideKey;
		}

		if (position->GetEnPassantSquare() != Square::SQ_NONE) {
			assert(position->GetEnPassantSquare() >= 0 && position->GetEnPassantSquare() < 120);
			finalKey ^= m_PieceKeys[Piece::PieceNone][position->GetEnPassantSquare()];
		}

		assert(position->GetCastlingRights() >= 0 && position->GetCastlingRights() <= 15);
		finalKey ^= m_CastleKeys[position->GetCastlingRights()];
		return finalKey;
	}
}