#include "Board.h"

#include "Hash.h"
#include "data.h"

constexpr char PceChar[] = ".PNBRQKpnbrqk";
constexpr char SideChar[] = "wb-";
constexpr char RankChar[] = "12345678";
constexpr char FileChar[] = "abcdefgh";

const int KnightDir[8] = { -8, -19, -21, 12, 8, 19, 21, 12 };
const int RookDir[4] = { -1, -10, 1, 10 };
const int BishopDir[4] = { -9, -11, 9, 11 };
const int KingDir[8] = { -1, -10, 1, 10, -9, -11, 9, 11 };

namespace BalouxEngine {

	void Board::ResetBoard() {
		int i = 0;

		for (i = 0; i < 120; ++i) {
			pieces[i] = OFFBOARD;
		}

		for (i = 0; i < 64; ++i) {
			pieces[SQ120(i)] = Piece::PieceNone;
		}

		for (i = 0; i < 3; ++i) {
			pawns[i].Get() = 0ULL;
		}

		for (i = 0; i < 2; ++i) {
			bigPieces[i] = 0;
			majorPieces[i] = 0;
			minorPieces[i] = 0;
			material[i] = 0;
		}

		for (i = 0; i < 13; ++i) {
			pieceNumber[i] = 0;
		}

		KingSq[White] = SQ_NONE;
		KingSq[Black] = SQ_NONE;
		m_side = Both;
		enPassant = SQ_NONE;
		fiftyMove = 0;
		ply = 0;
		hisPly = 0;
		castlingPermission = 0;
		posKey = 0ULL;
	}

	void Board::ParseFEN(std::string FEN) {
		assert(FEN != "");

		char* fen = (char*)FEN.c_str();

		int rank = RANK_8;
		int file = FILE_A;
		int piece = 0;
		int count = 0;
		int i = 0;
		int square64 = 0;
		int square120 = 0;

		ResetBoard();

		while ((rank >= RANK_1) && *fen) {
			count = 1;
			switch (*fen) {
			case 'p': piece = BlackPawn; break;
			case 'r': piece = BlackRook; break;
			case 'n': piece = BlackKnight; break;
			case 'b': piece = BlackBishop; break;
			case 'k': piece = BlackKing; break;
			case 'q': piece = BlackQueen; break;
			case 'P': piece = WhitePawn; break;
			case 'R': piece = WhiteRook; break;
			case 'N': piece = WhiteKnight; break;
			case 'B': piece = WhiteBishop; break;
			case 'K': piece = WhiteKing; break;
			case 'Q': piece = WhiteQueen; break;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				piece = PieceNone;
				count = *fen - '0';
				break;

			case '/':
			case ' ':
				rank--;
				file = FILE_A;
				fen++;
				continue;

			default:
				printf("FEN error \n");
				return;
			}

			for (i = 0; i < count; i++) {
				square64 = rank * 8 + file;
				square120 = SQ120(square64);
				if (piece != PieceNone) {
					pieces[square120] = piece;
				}
				file++;
			}
			fen++;
		}

		assert(*fen == 'w' || *fen == 'b');

		m_side = (*fen == 'w') ? White : Black;
		fen += 2;

		for (i = 0; i < 4; i++) {
			if (*fen == ' ') {
				break;
			}
			switch (*fen) {
			case 'K': castlingPermission |= WHITE_OO; break;
			case 'Q': castlingPermission |= WHITE_OOO; break;
			case 'k': castlingPermission |= BLACK_OO; break;
			case 'q': castlingPermission |= BLACK_OOO; break;
			default:	     break;
			}
			fen++;
		}
		fen++;

		assert(castlingPermission >= 0 && castlingPermission <= 15);

		if (*fen != '-') {
			file = fen[0] - 'a';
			rank = fen[1] - '1';

			assert(file >= FILE_A && file <= FILE_H);
			assert(rank >= RANK_1 && rank <= RANK_8);

			enPassant = FR2SQ(file, rank);
		}

		posKey = Hash::GeneratePosKey(this);

		UpdateListsMaterials();
	}

	void Board::PrintBoard() {
		int square, file, rank, piece;

		std::cout << "Game Board\n\n";

		for (rank = RANK_8; rank >= RANK_1; --rank) {
			std::cout << rank + 1 << " ";
			for (file = FILE_A; file <= FILE_H; ++file) {
				square = FR2SQ(file, rank);
				piece = pieces[square];
				std::cout << PceChar[piece];
			}
			std::cout << "\n";
		}

		std::cout << "\n  ";
		for (file = FILE_A; file <= FILE_H; ++file) {
			std::cout << (char)('a' + file);
		}
		std::cout << "\n";

		std::cout << "Side: " << SideChar[m_side] << "\n";
		std::cout << "En Passant: " << enPassant << "\n";
		std::cout << "Castle: " << ((castlingPermission & WHITE_OO) ? 'K' : '-')
			<< ((castlingPermission & WHITE_OOO) ? 'Q' : '-')
			<< ((castlingPermission & BLACK_OO) ? 'k' : '-')
			<< ((castlingPermission & BLACK_OOO) ? 'q' : '-');
		std::cout << "\nPosKey: " << posKey << "\n";

	}

	void Board::UpdateListsMaterials() {
		int piece, sq, index, colour;

		for (index = 0; index < BRD_SQ_NUM; ++index) {
			sq = index;
			piece = pieces[index];
			// assert(PceValidEmptyOffbrd(piece));
			if (piece != OFFBOARD && piece != PieceNone) {
				colour = PieceCol[piece];
				assert(Utils::SideValid(colour));

				if (PieceBig[piece] == true) bigPieces[colour]++;
				if (PieceMin[piece] == true) minorPieces[colour]++;
				if (PieceMaj[piece] == true) majorPieces[colour]++;

				material[colour] += PieceVal[piece];

				assert(pieceNumber[piece] < 10 && pieceNumber[piece] >= 0);

				pieceList[piece][pieceNumber[piece]] = sq;
				pieceNumber[piece]++;


				if (piece == WhiteKing) KingSq[White] = sq;
				if (piece == BlackKing) KingSq[Black] = sq;

				if (piece == WhitePawn) {
					SETBIT(pawns[White], SQ64(sq));
					SETBIT(pawns[Both], SQ64(sq));
				}
				else if (piece == BlackPawn) {
					SETBIT(pawns[Black], SQ64(sq));
					SETBIT(pawns[Both], SQ64(sq));
				}
			}
		}
	
	}

	bool Board::CheckBoard() {
		int t_pieceNumber[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int t_bigPieces[2] = { 0, 0 };
		int t_majorPieces[2] = { 0, 0 };
		int t_minorPieces[2] = { 0, 0 };
		int t_material[2] = { 0, 0 };

		int square64, t_piece, t_pieceNum, square120, color, pieceCount;
		Bitboard t_pawns[3];
		t_pawns[White] = pawns[White];
		t_pawns[Black] = pawns[Black];
		t_pawns[Both] = pawns[Both];

		for (t_piece = WhitePawn; t_piece <= BlackKing; ++t_piece) {
			for (t_pieceNum = 0; t_pieceNum < pieceNumber[t_piece]; ++t_pieceNum) {
				square120 = pieceList[t_piece][t_pieceNum];
				assert(pieces[square120] == t_piece);
			}
		}

		// check piece count and other counters
		for (square64 = 0; square64 < 64; ++square64) {
			square120 = SQ120(square64);
			t_piece = pieces[square120];
			t_pieceNumber[t_piece]++;
			color = PieceCol[t_piece];
			if (PieceBig[t_piece] == true) t_bigPieces[color]++;
			if (PieceMin[t_piece] == true) t_minorPieces[color]++;
			if (PieceMaj[t_piece] == true) t_majorPieces[color]++;

			t_material[color] += PieceVal[t_piece];
		}

		for (t_piece = WhitePawn; t_piece <= BlackKing; ++t_piece) {
			assert(t_pieceNumber[t_piece] == pieceNumber[t_piece]);
		}

		// check bitboards count
		pieceCount = t_pawns[White].CountBits();
		assert(pieceCount == pieceNumber[WhitePawn]);
		pieceCount = t_pawns[Black].CountBits();
		assert(pieceCount == pieceNumber[BlackPawn]);
		pieceCount = t_pawns[Both].CountBits();
		assert(pieceCount == (pieceNumber[BlackPawn] + pieceNumber[WhitePawn]));

		// check bitboards squares
		while (t_pawns[White].Get()) {
			square64 = t_pawns[White].PopBit();
			assert(pieces[SQ120(square64)] == WhitePawn);
		}

		while (t_pawns[Black].Get()) {
			square64 = t_pawns[Black].PopBit();
			assert(pieces[SQ120(square64)] == BlackPawn);
		}

		while (t_pawns[Both].Get()) {
			square64 = t_pawns[Both].PopBit();
			assert((pieces[SQ120(square64)] == BlackPawn) || (pieces[SQ120(square64)] == WhitePawn));
		}

		assert(t_material[White] == material[White] && t_material[Black] == material[Black]);
		assert(t_minorPieces[White] == minorPieces[White] && t_minorPieces[Black] == minorPieces[Black]);
		assert(t_majorPieces[White] == majorPieces[White] && t_majorPieces[Black] == majorPieces[Black]);
		assert(t_bigPieces[White] == bigPieces[White] && t_bigPieces[Black] == bigPieces[Black]);

		assert(m_side == White || m_side == Black);
		assert(Hash::GeneratePosKey(this) == posKey);

		assert(enPassant == SQ_NONE || (Utils::RanksBrd[enPassant] == RANK_6 && m_side == White)
			|| (Utils::RanksBrd[enPassant] == RANK_3 && m_side == Black));

		assert(pieces[KingSq[White]] == WhiteKing);
		assert(pieces[KingSq[Black]] == BlackKing);

		assert(castlingPermission >= 0 && castlingPermission <= 15);

		return true;
	}

	bool Board::isSquareAttacked(const int square, const int side) {
		int pce, i, t_sq, dir;

		assert(Utils::SquareOnBoard(square));
		assert(Utils::SideValid(side));
		assert(CheckBoard());

		// pawns
		if (side == White) {
			if (pieces[square - 11] == WhitePawn || pieces[square - 9] == WhitePawn) {
				return true;
			}
		}
		else {
			if (pieces[square + 11] == BlackPawn || pieces[square + 9] == BlackPawn) {
				return true;
			}
		}

		// knights
		for (i = 0; i < 8; ++i) {
			pce = pieces[square + KnightDir[i]];
			//assert(PceValidEmptyOffbrd(pce));
			if (pce != OFFBOARD && IsKN(pce) && PieceCol[pce] == side) {
				return true;
			}
		}

		// rooks, queens
		for (i = 0; i < 4; ++i) {
			dir = RookDir[i];
			t_sq = square + dir;
			//assert(SqIs120(t_sq));
			pce = pieces[t_sq];
			//assert(PceValidEmptyOffbrd(pce));
			while (pce != OFFBOARD) {
				if (pce != PieceNone) {
					if (IsRQ(pce) && PieceCol[pce] == side) {
						return true;
					}
					break;
				}
				t_sq += dir;
				//assert(SqIs120(t_sq));
				pce = pieces[t_sq];
			}
		}

		// bishops, queens
		for (i = 0; i < 4; ++i) {
			dir = BishopDir[i];
			t_sq = square + dir;
			//assert(SqIs120(t_sq));
			pce = pieces[t_sq];
			//assert(PceValidEmptyOffbrd(pce));
			while (pce != OFFBOARD) {
				if (pce != PieceNone) {
					if (IsBQ(pce) && PieceCol[pce] == side) {
						return true;
					}
					break;
				}
				t_sq += dir;
				//assert(SqIs120(t_sq));
				pce = pieces[t_sq];
			}
		}

		// kings
		for (i = 0; i < 8; ++i) {
			pce = pieces[square + KingDir[i]];
			//assert(PceValidEmptyOffbrd(pce));
			if (pce != OFFBOARD && IsKi(pce) && PieceCol[pce] == side) {
				return true;
			}
		}

		return false;

	}

#define HASH_PCE(pce,sq) (posKey ^= (Hash::m_PieceKeys[(pce)][(sq)]))
#define HASH_CA (posKey ^= (Hash::m_CastleKeys[(castlingPermission)]))
#define HASH_SIDE (posKey ^= (Hash::m_SideKey))
#define HASH_EP (posKey ^= (Hash::m_PieceKeys[PieceNone][(enPassant)]))

	constexpr int CastlePerm[120] = {
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15
	};

	void Board::ClearPiece(const int square) {
		assert(Utils::SquareOnBoard(square));
		int piece = pieces[square];
		assert(Utils::PieceValid(piece));

		int color = PieceCol[piece];
		int i = 0;
		int t_pieceNumber = -1;
		
		HASH_PCE(piece, square);

		pieces[square] = PieceNone;
		material[color] -= PieceVal[piece];

		if (PieceBig[piece]) {
			bigPieces[color]--;
			if (PieceMaj[piece]) {
				majorPieces[color]--;
			}
			else {
				minorPieces[color]--;
			}
		}
		else {
			pawns[color].ClearBit(SQ64(square));
			pawns[Both].ClearBit(SQ64(square));
		}

		for (i = 0; i < pieceNumber[piece]; ++i) {
			if (pieceList[piece][i] == square) {
				t_pieceNumber = i;
				break;
			}
		}

		assert(t_pieceNumber != -1);

		pieceNumber[piece]--;
		pieceList[piece][t_pieceNumber] = pieceList[piece][pieceNumber[piece]];
	}

	void Board::AddPiece(const int square, const int piece) {
		assert(Utils::SquareOnBoard(square));
		assert(Utils::PieceValid(piece));

		int color = PieceCol[piece];

		HASH_PCE(piece, square);

		pieces[square] = piece;

		if (PieceBig[piece]) {
			bigPieces[color]++;
			if (PieceMaj[piece]) {
				majorPieces[color]++;
			}
			else {
				minorPieces[color]++;
			}
		}
		else {
			pawns[color].SetBit(SQ64(square));
			pawns[Both].SetBit(SQ64(square));
		}

		
		material[color] += PieceVal[piece];
		pieceList[piece][pieceNumber[piece]++] = square;
	}

	void Board::MovePiece(const int from, const int to) {
		assert(Utils::SquareOnBoard(from));
		assert(Utils::SquareOnBoard(to));

		int index = 0;
		int pce = pieces[from];
		int col = PieceCol[pce];
		assert(Utils::SideValid(col));
		assert(Utils::PieceValid(pce));

#ifdef _DEBUG
		int t_PieceNum = false;
#endif

		HASH_PCE(pce, from);
		pieces[from] = PieceNone;

		HASH_PCE(pce, to);
		pieces[to] = pce;

		if (!PieceBig[pce]) {
			CLRBIT(pawns[col], SQ64(from));
			CLRBIT(pawns[Both], SQ64(from));
			SETBIT(pawns[col], SQ64(to));
			SETBIT(pawns[Both], SQ64(to));
		}

		for (index = 0; index < pieceNumber[pce]; ++index) {
			if (pieceList[pce][index] == from) {
				pieceList[pce][index] = to;
#ifdef _DEBUG
				t_PieceNum = true;
#endif
				break;
			}
		}
		assert(t_PieceNum);

	}

	bool Board::MakeMove(int move) {
		std::cout << "Made move : " << Utils::MoveToString(move) << "\n";

		assert(CheckBoard());

		int from = FROM(move);
		int to = TO(move);
		int side = m_side;

		assert(Utils::SquareOnBoard(from));
		assert(Utils::SquareOnBoard(to));
		assert(Utils::SideValid(side));
		assert(Utils::PieceValid(pieces[from]));
		// assert(hisPly >= 0 && hisPly < MAXGAMEMOVES);
		// assert(ply >= 0 && ply < MAXDEPTH);

		history[hisPly].posKey = posKey;

		if (move & MOVEFLAGEP) {
			if (side == White) {
				ClearPiece(to - 10);
			}
			else {
				ClearPiece(to + 10);
			}
		}
		else if (move & MOVEFLAGCAS) {
			switch (to) {
			case C1:
				MovePiece(A1, D1);
				break;
			case C8:
				MovePiece(A8, D8);
				break;
			case G1:
				MovePiece(H1, F1);
				break;
			case G8:
				MovePiece(H8, F8);
				break;
			default: assert(false); break;
			}
		}

		if (enPassant != SQ_NONE) HASH_EP;
		HASH_CA;

		history[hisPly].move = move;
		history[hisPly].fiftyMove = fiftyMove;
		history[hisPly].enPassant = enPassant;
		history[hisPly].castlingPerm = castlingPermission;

		castlingPermission &= CastlePerm[from];
		castlingPermission &= CastlePerm[to];
		enPassant = SQ_NONE;

		HASH_CA;

		int captured = CAPTURED(move);
		fiftyMove++;

		if (captured != PieceNone) {
			assert(Utils::PieceValid(captured));
			ClearPiece(to);
			fiftyMove = 0;
		}

		hisPly++;
		ply++;

		// assert(hisPly >= 0 && hisPly < MAXGAMEMOVES);
		// assert(ply >= 0 && ply < MAXDEPTH);

		if (PiecePawn[pieces[from]]) {
			fiftyMove = 0;
			if (move & MOVEFLAGPS) {
				if (side == White) {
					enPassant = from + 10;
					assert(Utils::RanksBrd[enPassant] == RANK_3);
				}
				else {
					enPassant = from - 10;
					assert(Utils::RanksBrd[enPassant] == RANK_6);
				}
				HASH_EP;
			}
		}

		MovePiece(from, to);

		int prPce = PROMOTED(move);
		if (prPce != PieceNone) {
			assert(Utils::PieceValid(prPce) && !PiecePawn[prPce]);
			ClearPiece(to);
			AddPiece(to, prPce);
		}

		if (PieceKing[pieces[to]]) {
			KingSq[side] = to;
		}

		m_side ^= 1;
		HASH_SIDE;

		assert(CheckBoard());


		if (isSquareAttacked(KingSq[side], m_side)) {
			TakeMove();
			return false;
		}

		return true;
	}

	void Board::TakeMove() {
		std::cout << "Took move back.\n";

		assert(CheckBoard());

		hisPly--;
		ply--;

		// assert(hisPly >= 0 && hisPly < MAXGAMEMOVES);
		// assert(ply >= 0 && ply < MAXDEPTH);

		int move = history[hisPly].move;
		int from = FROM(move);
		int to = TO(move);

		assert(Utils::SquareOnBoard(from));
		assert(Utils::SquareOnBoard(to));

		if (enPassant != SQ_NONE) HASH_EP;
		HASH_CA;

		castlingPermission = history[hisPly].castlingPerm;
		fiftyMove = history[hisPly].fiftyMove;
		enPassant = history[hisPly].enPassant;

		if (enPassant != SQ_NONE) HASH_EP;
		HASH_CA;

		m_side ^= 1;
		HASH_SIDE;

		if (MOVEFLAGEP & move) {
			if (m_side == White) {
				AddPiece(to - 10, BlackPawn);
			}
			else {
				AddPiece(to + 10, WhitePawn);
			}
		}
		else if (MOVEFLAGCAS & move) {
			switch (to) {
			case C1: MovePiece(D1, A1); break;
			case C8: MovePiece(D8, A8); break;
			case G1: MovePiece(F1, H1); break;
			case G8: MovePiece(F8, H8); break;
			default: assert(false); 
			}
		}

		MovePiece(to, from);

		if (PieceKing[pieces[from]]) {
			KingSq[m_side] = from;
		}

		int captured = CAPTURED(move);
		if (captured != PieceNone) {
			assert(Utils::PieceValid(captured));
			AddPiece(to, captured);
		}

		if (PROMOTED(move) != PieceNone) {
			assert(Utils::PieceValid(PROMOTED(move)) && !PiecePawn[PROMOTED(move)]);
			ClearPiece(from);
			AddPiece(from, (PieceCol[PROMOTED(move)] == White ? WhitePawn : BlackPawn));
		}

		assert(CheckBoard());

	}
}