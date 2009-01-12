#ifndef SEARCH_HH
#define SEARCH_HH

#include <algorithm>

#include "hash.hh"
#include "board.hh"

/* configurable params */

const int mate_score = 100000;

uint32_t pv_fail = 0, node_count = 0;

const int cap_table[7]={0,1,2,3,4,5,6};

const int mob_pref_table[7]={0,1,5,4,3,2,0};


int move_history[1024][64][64];

struct MoveScore {
    Move m;
    int score;
    void setScore() {
        int c = cap_table[board.b[m.d].type];
        score = move_history[board.move_count][m.o][m.d] + c * 512;
    }
    inline bool operator<(const MoveScore& ms) const {
        return score > ms.score;
    }
};

namespace {
    struct init_search {
        init_search() {
            memset(move_history, 0, sizeof(move_history));
        }
    } _init_search;
}

void sort_moves(Move* moves, int n) {
    MoveScore ms[512];
    for(int i=0;i<n;++i) {
        ms[i].m = moves[i];
        ms[i].setScore();
    }
    std::sort(ms, ms+n);
    for(int i=0;i<n;++i) {
        moves[i] = ms[i].m;
    }
}

#define SEARCH_PV 2
#define SEARCH_ROOT 4
#define INFINITE_SCORE (mate_score + 128)

int search(int depth, int alfa = -INFINITE_SCORE, int beta = INFINITE_SCORE, int flags = SEARCH_ROOT) {
    int n, t, i;
    MoveInfo mi;
    hash_slot* slot;
    Move list[512];
    Move best_move(0,0,0,0);
    int list_offset = 0;
    int best_score = -INFINITE_SCORE;
    bool is_pv = (flags & SEARCH_PV) != 0;
    bool is_root = (flags & SEARCH_ROOT) != 0;
    bool is_quiescent = (depth <= 0);

    flags &= ~SEARCH_ROOT;

    if(board.repeated()) {
        return 0;
    }

    /* poda os limites de mate */
    if(alfa >= mate_score + depth)
        return alfa;
    if(beta <= -(mate_score + depth - 1))
        return beta;


    if(not is_quiescent) {
        slot = hash_find(board);
        
        /* Iterative deepening */
        if(depth >= 3 and slot == 0) {
            search(depth - 2, alfa, beta, SEARCH_PV);
            slot = hash_find(board);
        }

        /* consulta a tabela hash */
        if(slot != 0) {
            if(slot->depth == depth and not is_root) {
                if(slot->lower_bound == slot->upper_bound) {
                    return slot->lower_bound;
                } else if(slot->lower_bound >= beta) {
                    return slot->lower_bound;
                } else if(slot->upper_bound <= alfa) {
                    return slot->upper_bound;
                }
            }
            /* pega uma dica da tabela se houver */
            if(slot->move.o != slot->move.d) {
                list[list_offset++] = slot->move;
            }
        }
    }


    /* em quiescent testa apenas as capturas */
    if(is_quiescent) {
        n = board.listAttacksg(list + list_offset, board.turn) + list_offset;
        best_score = std::max(best_score, board.eval());
    } else {
        n = board.listMovesg(list + list_offset, board.turn) + list_offset;
    }


    sort_moves(list+list_offset, n-list_offset);

    ASSERT(n < 512);

    for(i=0;i<n;i++) {
        node_count++;

        /* poda */
        if(best_score >= beta) {
            break;
        }

        /* mate */
        if(board.b[list[i].d].type == KING) {
            best_score = (mate_score + depth);
            best_move = list[i];
            break;
        }

        ASSERT(list[i].is_valid());

        /* move */
        mi = board.move(list[i]);

        /* recursão */
        if(depth <= 1) {
            t = -search(depth-1, -beta, -std::max(alfa, best_score), flags);
        } else { 
            t = best_score + 1;

            if(i > 0 and not is_pv) {
                t = -search(depth-1, -best_score-1, -best_score, flags);
            }

            if(t > best_score) {
                t = -search(depth-1, -beta, -std::max(alfa, best_score), flags | SEARCH_PV);
            }
        }

        /* volta movimento */
        board.volta(list[i],mi);

        /* atualiza resultado */
        if(t > best_score) {
            best_score = t;
            best_move = list[i];
        }

        /* loga o progresso */
        /*if(is_root) {
            fprintf(log_file, "%d/%d %d/%d ", i, n, best_score, t);
            print_move(log_file, list[i]);
            fprintf(log_file, "\n");
            fflush(log_file);
        }*/

    }

    /* verifica afogamento */
    if(best_score == -(mate_score + depth - 1) and not board.checkCheck()) {
        best_score = 0;
    }

    /* registra resultado na tabela hash */
    if(not is_quiescent) {
        if(best_score <= alfa) {
            /* upper bound */
            hash_insert(board, depth, - (mate_score + depth - 1), best_score, best_move);
        } else if(best_score >= beta) {
            /* lower bound */
            hash_insert(board, depth, best_score, mate_score + depth, best_move);
        } else {
            /* exact eval */
            hash_insert(board, depth, best_score, best_score, best_move);
        }
    }

    if(best_move != Move(0,0,0,0)) {
        ASSERT(board.move_count < 1024);
        if((++move_history[board.move_count][best_move.o][best_move.d])>=512) {
            for(int i=0;i<64;++i) for(int j=0;j<64;++j) {
                move_history[board.move_count][i][j]/=2;
            }
        }

    }

    return best_score;
}

Move ComputerPlay(int depth, bool post)
{
    Move best;
    hash_slot* slot;
    MoveInfo mis[128];
    Move hist[128];
    int score = 0;

    node_count = 0;
    pv_fail = 0;

    score = search(depth);

    fprintf(log_file, "%s\n", board.getFen().c_str());

    slot = hash_find(board);

    best = slot->move;

    fprintf(log_file, "\n%+d ", slot->lower_bound);
    print_move(log_file, slot->move);
    fprintf(log_file, "\n");

    if(post) {
        printf("%d %d %d %d", depth, score, 0, node_count);
        for(int d=depth;d>=1;--d) {
            search(d, score-1, score);
            slot = hash_find(board);
            printf(" ");
            print_move(stdout, slot->move);
            mis[d] = board.move(slot->move);
            hist[d] = slot->move;
            score = -score;
            if(d == 1 or board.done()) {
                for(;d<=depth;++d) {
                    board.volta(hist[d], mis[d]);
                }
                break;
            }
        }
        printf("\n");
    }

    fprintf(log_file, "\n");

    fprintf(log_file,"nodes visited %d\n", node_count);
    fprintf(log_file,"pv fails %d\n", pv_fail);

    fprintf(log_file,"\n");

    fflush(log_file);

    return best;
}

#endif
