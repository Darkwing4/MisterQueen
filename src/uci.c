#include <stdio.h>
#include <string.h>
#include "board.h"
#include "move.h"
#include "search.h"
#include "tinycthread.h"
#include "uci.h"
#include "util.h"

static Board board;
static thrd_t thrd;

void handle_uci() {
    printf("id name Chess\n");
    printf("id author Michael Fogleman\n");
    printf("uciok\n");
}

void handle_isready() {
    printf("readyok\n");
}

void handle_fen(char *fen) {
    board_load_fen(&board, fen);
}

void handle_startpos() {
    board_reset(&board);
}

void handle_startpos_moves(char *moves) {
    Move move;
    board_reset(&board);
    char *key;
    char *token = tokenize(moves, " ", &key);
    while (token) {
        move_from_string(&move, token);
        make_move(&board, &move);
        token = tokenize(NULL, " ", &key);
    }
}

int thread_func(void *arg) {
    Move move;
    search(&board, 0, &move);
    return 0;
}

void handle_go() {
    thrd_create(&thrd, thread_func, NULL);
}

void handle_stop() {
    stop_search();
    thrd_join(thrd, NULL);
}

int parse_line() {
    char data[1024];
    if (fgets(data, 1024, stdin) == NULL) {
        return 0;
    }
    char arg[1024];
    char *line = strip(data);
    if (strcmp(line, "uci") == 0) {
        handle_uci();
    }
    if (strcmp(line, "isready") == 0) {
        handle_isready();
    }
    if (strcmp(line, "position startpos") == 0) {
        handle_startpos();
    }
    if (sscanf(line, "position startpos moves %[^]", arg) == 1) {
        handle_startpos_moves(arg);
    }
    if (sscanf(line, "position fen %[^]", arg) == 1) {
        handle_fen(arg);
    }
    if (starts_with(line, "go")) {
        handle_go();
    }
    if (strcmp(line, "stop") == 0) {
        handle_stop();
    }
    if (strcmp(line, "quit") == 0) {
        return 0;
    }
    return 1;
}

void uci_main() {
    setbuf(stdout, NULL);
    board_reset(&board);
    while (parse_line());
}