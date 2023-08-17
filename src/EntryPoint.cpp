#include "Thread.h"
#include "Utils.h"
#include "Bitboard.h"
#include "Hash.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "perft/Perft.h"

using namespace BalouxEngine;

// #define MOVE(from,to,capture,promote,flags) ( (from) | ((to) << 7) | ( (capture) << 14 ) | ( (promote) << 20 ) | (flags))
#define TRICKY_FEN "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
#define TRICKY_FEN2 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"
#define TRICKY_FEN3 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define TRICKY_FEN4 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define WAC_FEN "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - 0 1"

#define RICHARD_FEN "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"


int main() {
	Utils::Init64To120();
	Utils::InitFilesRanksBrd();
	Bitboard::InitBitMasks();
	Hash::InitHashKeys();
	MoveGenerator::InitMvvLva();

	Board board = Board();
	board.GetPVTable().InitPvTable();

	board.ParseFEN(RICHARD_FEN);

	char input[6];
	int move = 0;
	int PvNum = 0;
	int Max = 0;
	PVTable table = board.GetPVTable();

	while (true) {
		board.PrintBoard();
		std::cout << "Enter a move : ";
		std::cin >> input;
		std::cout << "\n";

		if (input[0] == 'q') {
			break;
		}
		else if (input[0] == 't') {
			board.TakeMove();
		}
		else if (input[0] == 's') {
			SearchInfo info;
			info.depth = 6;
			Search search(&board);
			search.SetSearchInfo(info);
			search.SearchPosition();

		}
		else {
			move = Utils::ParseMove(input, &board);
			if (move != NOMOVE) {
				table.StorePvMove(move);
				board.MakeMove(move);
			}
		}
		std::cin.clear();
	}

	free(board.GetPVTable().pTable);

	return 0;
}

