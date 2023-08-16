#include "Utils.h"

#include <iostream>
#include <stdio.h>

namespace BalouxEngine {

	int Utils::m_Sq120ToSq64[120];
	int Utils::m_Sq64ToSq120[64];

	int Utils::FilesBrd[120];
	int Utils::RanksBrd[120];

	void Utils::Init64To120() {

		int i = 0;
		int file = FILE_A;
		int rank = RANK_1;
		int square = A1;
		int square64 = 0;

		for (i = 0; i < 120; ++i) {
			m_Sq120ToSq64[i] = 65;
		}

		for (i = 0; i < 64; ++i) {
			m_Sq64ToSq120[i] = 120;
		}

		for (rank = RANK_1; rank <= RANK_8; ++rank) {
			for (file = FILE_A; file <= FILE_H; ++file) {
				square = FR2SQ(file, rank);
				m_Sq64ToSq120[square64] = square;
				m_Sq120ToSq64[square] = square64;
				square64++;
			}
		}

	}

	void Utils::InitFilesRanksBrd() {
		int index = 0;
		int file = FILE_A;
		int rank = RANK_1;
		int sq = A1;

		for (index = 0; index < BRD_SQ_NUM; ++index) {
			FilesBrd[index] = OFFBOARD;
			RanksBrd[index] = OFFBOARD;
		}

		for (rank = RANK_1; rank <= RANK_8; ++rank) {
			for (file = FILE_A; file <= FILE_H; ++file) {
				sq = FR2SQ(file, rank);
				FilesBrd[sq] = file;
				RanksBrd[sq] = rank;
			}
		}
	}

	char* Utils::SquareToString(const int square) {
		static char SquareString[3];
		int file = FilesBrd[square];
		int rank = RanksBrd[square];

		sprintf_s(SquareString, "%c%c", 'a' + file, '1' + rank);
		return SquareString;
	}

	char* Utils::MoveToString(const int move) {
		static char MoveString[6];

		int ff = FilesBrd[FROM(move)];
		int rf = RanksBrd[FROM(move)];
		int ft = FilesBrd[TO(move)];
		int rt = RanksBrd[TO(move)];

		int promoted = PROMOTED(move);

		if (promoted) {
			char pchar = 'q';
			if (IsKN(promoted)) {
				pchar = 'n';
			}
			else if (IsRQ(promoted) && !IsBQ(promoted)) {
				pchar = 'r';
			}
			else if (!IsRQ(promoted) && IsBQ(promoted)) {
				pchar = 'b';
			}
			sprintf_s(MoveString, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pchar);
		}
		else {
			sprintf_s(MoveString, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));
		}

		return MoveString;
	}

	void Utils::PrintMoveList(const MoveList* list) {
		int index = 0;
		int score = 0;
		int move = 0;
		printf("MoveList:\n");

		for (index = 0; index < list->count; ++index) {

			move = list->moves[index].move;
			score = list->moves[index].score;

			printf("Move:%d > %s (score:%d)\n", index + 1, MoveToString(move), score);
		}
		printf("MoveList Total %d Moves:\n\n", list->count);
	}

}