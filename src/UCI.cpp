#include "UCI.h"
#include "Board.h"

#include "tinycthread.h"

#include <memory>

#define NAME "BalouxEngine (from VICE) 0.1"
#define INPUTBUFFER 400 * 6

namespace BalouxEngine {

	thrd_t mainSearchThread;

	thrd_t LaunchSearchThread(Board* board, SearchInfo* searchInfo) {
		SearchThreadData* data = new SearchThreadData(); // NEED TO DELETE AT SOME POINT!!!
		data->board = board;
		data->info = searchInfo;

		thrd_t thread;
		thrd_create(&thread, &Search::SearchPosition_Thread, (void*)data);

		return thread;
	}

	void JoinSearchThread() {
		thrd_join(mainSearchThread, nullptr);
	}

	void UCI::Loop(Board* pos, SearchInfo* info) {

		fflush(stdin);
		fflush(stdout);

		char line[INPUTBUFFER];
		std::cout << "id name " << NAME << "\n";
		std::cout << "id author Bluefever and balouxe\n";
		std::cout << "uciok\n";

		Utils::globalHashTable->InitHashTable();

		while (true) {
			memset(&line[0], 0, sizeof(line));
			fflush(stdout);
			if (!fgets(line, INPUTBUFFER, stdin))
				continue;

			if (line[0] == '\n')
				continue;

			if (!strncmp(line, "isready", 7)) {
				printf("readyok\n");
				continue;
			}
			else if (!strncmp(line, "position", 8)) {
				ParsePosition(line, pos);
			}
			else if (!strncmp(line, "ucinewgame", 10)) {
				Utils::globalHashTable->ClearHashTable();
				ParsePosition("position startpos\n", pos);
			}
			else if (!strncmp(line, "go", 2)) {
				printf("Seen Go..\n");
				ParseGo(line, info, pos);
			}
			else if (!strncmp(line, "run", 3)) {
				pos->ParseFEN(START_FEN);
				ParseGo("go infinite", info, pos);
			}
			else if (!strncmp(line, "quit", 4)) {
				info->quit = true;
				info->stopped = true;
				JoinSearchThread();
				break;
			}
			else if (!strncmp(line, "stop", 4)) {
				info->stopped = true;
				JoinSearchThread();
				continue;
			}
			else if (!strncmp(line, "uci", 3)) {
				printf("id name %s\n", NAME);
				printf("id author Bluefever\n");
				printf("uciok\n");
			}
			else if (!strncmp(line, "debug", 4)) {
				// DebugAnalysisTest(pos, info);
				break;
			}
			/* else if (!strncmp(line, "setoption name Hash value ", 26)) {
				sscanf(line, "%*s %*s %*s %*s %d", &MB);
				if (MB < 4) MB = 4;
				if (MB > MAX_HASH) MB = MAX_HASH;
				printf("Set Hash to %d MB\n", MB);
				InitHashTable(pos->HashTable, MB);
			}
			else if (!strncmp(line, "setoption name Book value ", 26)) {
				char* ptrTrue = NULL;
				ptrTrue = strstr(line, "true");
				if (ptrTrue != NULL) {
					EngineOptions->UseBook = TRUE;
				}
				else {
					EngineOptions->UseBook = FALSE;
				}
			}*/
			if (info->quit) break;
		}

	}

	void UCI::ParseGo(const char* line, SearchInfo* info, Board* board) {
		int depth = -1, movestogo = 40, movetime = -1;
		int time = -1, inc = 0;
		const char* ptr = NULL;
		info->timeSet = false;

		if ((ptr = strstr(line, "infinite"))) {
			;
		}

		if ((ptr = strstr(line, "binc")) && board->GetSide() == Black) {
			inc = atoi(ptr + 5);
		}

		if ((ptr = strstr(line, "winc")) && board->GetSide() == White) {
			inc = atoi(ptr + 5);
		}

		if ((ptr = strstr(line, "wtime")) && board->GetSide() == White) {
			time = atoi(ptr + 6);
		}

		if ((ptr = strstr(line, "btime")) && board->GetSide() == Black) {
			time = atoi(ptr + 6);
		}

		if ((ptr = strstr(line, "movestogo"))) {
			movestogo = atoi(ptr + 10);
		}

		if ((ptr = strstr(line, "movetime"))) {
			movetime = atoi(ptr + 9);
		}

		if ((ptr = strstr(line, "depth"))) {
			depth = atoi(ptr + 6);
		}

		if (movetime != -1) {
			time = movetime;
			movestogo = 1;
		}

		info->startTime = Utils::GetTimeInMs();
		info->depth = depth;

		if (time != -1) {
			info->timeSet = true;
			time /= movestogo;
			time -= 50;
			info->endTime = info->startTime + time + inc;
		}

		if (depth == -1) {
			info->depth = MAXDEPTH;
		}

		printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
			time, info->startTime, info->endTime, info->depth, info->timeSet);
		mainSearchThread = LaunchSearchThread(board, info);
	}

	void UCI::ParsePosition(const char* lineIn, Board* board) {
		lineIn += 9;
		const char* ptrChar = lineIn;

		if (strncmp(lineIn, "startpos", 8) == 0) {
			board->ParseFEN(START_FEN);
		}
		else {
			ptrChar = strstr(lineIn, "fen");
			if (ptrChar == NULL) {
				board->ParseFEN(START_FEN);
			}
			else {
				ptrChar += 4;
				board->ParseFEN(ptrChar);
			}
		}

		ptrChar = strstr(lineIn, "moves");
		int move;

		if (ptrChar != NULL) {
			ptrChar += 6;
			while (*ptrChar) {
				move = Utils::ParseMove((char*)ptrChar, board);
				if (move == NOMOVE) break;
				board->MakeMove(move);
				board->ply = 0;
				while (*ptrChar && *ptrChar != ' ') ptrChar++;
				ptrChar++;
			}
		}
	}

}