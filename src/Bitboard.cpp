#include "Bitboard.h"

#include "Utils.h"

U64 SetMask[64];
U64 ClearMask[64];

namespace BalouxEngine {

	Bitboard::Bitboard() {
		bitboard = 0;
	}

	void Bitboard::PrintBitboard() {
		U64 shiftMe = 1ULL;

		int rank = 0;
		int file = 0;
		int square = 0;
		int square64 = 0;

		std::cout << "\n";
		for (rank = RANK_8; rank >= RANK_1; --rank) {
			for (file = FILE_A; file <= FILE_H; ++file) {
				square = FR2SQ(file, rank);
				square64 = Utils::Sq120ToSq64(square);

				if ((shiftMe << square64) & bitboard) {
					std::cout << "X";
				}
				else {
					std::cout << "-";
				}
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}

	void Bitboard::PrintBitboard(U64 bb) {
		U64 shiftMe = 1ULL;

		int rank = 0;
		int file = 0;
		int square = 0;
		int square64 = 0;

		std::cout << "\n";
		for (rank = RANK_8; rank >= RANK_1; --rank) {
			for (file = FILE_A; file <= FILE_H; ++file) {
				square = FR2SQ(file, rank);
				square64 = Utils::Sq120ToSq64(square);

				if ((shiftMe << square64) & bb) {
					std::cout << "X";
				}
				else {
					std::cout << "-";
				}
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}


	int Bitboard::PopBit() {
		U64 b = bitboard ^ (bitboard - 1);
		unsigned int fold = (unsigned)((b & 0xffffffff) ^ (b >> 32));
		bitboard &= (bitboard - 1);
		return BitTable[(fold * 0x783a9b23) >> 26];
	}

	int Bitboard::CountBits() {
		int r;
		for (r = 0; bitboard; r++, bitboard &= bitboard - 1);
		return r;
	}

	void Bitboard::InitBitMasks(){
		int i = 0;

		for (i = 0; i < 64; i++) {
			SetMask[i] = 0ULL;
			ClearMask[i] = 0ULL;
		}

		for (i = 0; i < 64; i++) {
			SetMask[i] |= (1ULL << i);
			ClearMask[i] = ~SetMask[i];
		}
	}

	void Bitboard::SetBit(int pos) {
		bitboard |= SetMask[pos];
	}
	void Bitboard::ClearBit(int pos) {
		bitboard &= ClearMask[pos];
	}
}