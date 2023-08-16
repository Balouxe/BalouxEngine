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


int main() {
	Utils::Init64To120();
	Utils::InitFilesRanksBrd();
	Bitboard::InitBitMasks();
	Hash::InitHashKeys();

	Board board = Board();

	board.ParseFEN(TRICKY_FEN);

	Perft p = Perft(&board);
	p.DoPerftTest(3);

	return 0;
}

