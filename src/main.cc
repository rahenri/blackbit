#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdint.h>
#include <string>

#include "bitboard.hh"
#include "board.hh"
#include "debug.hh"
#include "hash.hh"
#include "search.hh"

using namespace std;

/* default depth */
#define dificuldade 8

typedef int8_t place;

#define printf(...) printf(__VA_ARGS__), fprintf(log_file, "<- " __VA_ARGS__)

int main(int argc, char **argv) {
  char comando[1024];
  char *args;
  int play = BLACK;
  int go = 0;
  int depth = dificuldade;
  int my_time = 0, op_time = 0;
  bool ponder = false, post = false;

  if (argc == 1)
    log_file = fopen("tmp.txt", "w");
  else
    log_file = fopen(argv[1], "w");

  /* start with 100mb o hash size */
  hash_set_size(100 * (1 << 20));

  board.setInitial();
  ASSERT(board.checkBoard());
  board.check_hash_key(__LINE__);

  while (1) {
    if (fgets(comando, sizeof(comando), stdin) == NULL)
      break;
    fprintf(log_file, "-> %s", comando);
    comando[strlen(comando) - 1] = 0;
    fflush(log_file);
    int i;
    for (i = 0; comando[i] != 0 and !isspace(comando[i]); ++i)
      ;
    if (comando[i] != 0) {
      comando[i] = 0;
      ++i;
      for (; isspace(comando[i]); ++i)
        ;
    }
    args = comando + i;

    if (strcmp(comando, "xboard") == 0) {
      /* so uso o xboard mesmo, não faça nada aqui */
    } else if (strcmp(comando, "protover") == 0) {
      /* features */
      printf("feature myname=\"raphael-chess%d\" ping=1 usermove=1 draw=0 "
             "variants=\"normal\" sigint=0 sigterm=0 setboard=1 playother=1 "
             "analyze=0 colors=0 done=1\n",
             dificuldade);
    } else if (strcmp(comando, "new") == 0) {
      /* reinicia tudo */
      board.setInitial();
      play = BLACK;
      go = 1;
    } else if (strcmp(comando, "variant") == 0) {
      /* não suportamos nenhuma variante */
    } else if (strcmp(comando, "quit") == 0) {
      /* fecha */
      break;
    } else if (strcmp(comando, "random") == 0) {
      /* not used */
    } else if (strcmp(comando, "force") == 0) {
      /* pare de jogar */
      go = 0;
    } else if (strcmp(comando, "go") == 0) {
      /* jogue! */
      play = board.turn;
      go = 1;
    } else if (strcmp(comando, "playother") == 0) {
      /* jogue com o outra cor */
      play = board.turn ^ 1;
      go = 1;
    } else if (strcmp(comando, "white") == 0) {
      /* obsoleto, não faça nada */
    } else if (strcmp(comando, "black") == 0) {
      /* obsoleto, não faça nada */
    } else if (strcmp(comando, "level") == 0) {
    } else if (strcmp(comando, "st") == 0) {
    } else if (strcmp(comando, "sd") == 0) {
      /* profundidade limite */
      depth = atoi(args);
    } else if (strcmp(comando, "time") == 0) {
      /* meu tempo */
      my_time = atoi(args);
    } else if (strcmp(comando, "otim") == 0) {
      /* tempo do opoente */
      op_time = atoi(args);
    } else if (strcmp(comando, "board") == 0) {
      /* extensão, imprime o tabuleiro */
      board.print(stdout);
    } else if (strcmp(comando, "ping") == 0) {
      printf("pong %s\n", args);
    } else if (strcmp(comando, "draw") == 0) {
    } else if (strcmp(comando, "result") == 0) {
      /* temos um resultado, não jogue mais */
      go = 0;
    } else if (strcmp(comando, "setboard") == 0) {
      /* seta para a posição dada */
      board.setFen(args);
    } else if (strcmp(comando, "edit") == 0) {
    } else if (strcmp(comando, "hint") == 0) {
    } else if (strcmp(comando, "bk") == 0) {
    } else if (strcmp(comando, "undo") == 0) {
    } else if (strcmp(comando, "remove") == 0) {
    } else if (strcmp(comando, "hard") == 0) {
      /* podemos pensar o tempo todo */
      ponder = true;
    } else if (strcmp(comando, "easy") == 0) {
      /* pense apenas nossa vez */
      ponder = false;
    } else if (strcmp(comando, "nopost") == 0) {
      /* não coloca resultados da busca */
      post = false;
    } else if (strcmp(comando, "post") == 0) {
      /* imprime resultados da busca */
      post = true;
    } else if (strcmp(comando, "accepted") == 0) {
    } else if (strcmp(comando, "analyze") == 0) {
    } else if (strcmp(comando, "name") == 0) {
    } else if (strcmp(comando, "rating") == 0) {
    } else if (strcmp(comando, "ics") == 0) {
    } else if (strcmp(comando, "computer") == 0) {
    } else if (strcmp(comando, "pause") == 0) {
    } else if (strcmp(comando, "resume") == 0) {
    } else {
      /* jogada do oponente */
      Move m;
      if (strcmp(comando, "usermove") == 0)
        m = parse_move_string(args);
      else
        m = parse_move_string(comando);
      if (not board.validateMove(m)) {
        printf("Illegal move: ");
        print_move(stdout, m);
        printf("\n");
      } else {
        board.move(m);
        play = board.turn;
        fprintf(log_file, "%s\n\n", board.getFen().c_str());
        fprintf(log_file, "rough score %+d\n", board.eval());
      }
    }
    fflush(stdout);
    /* joga se for o caso */
    if (play == board.turn and go) {
      Move m = ComputerPlay(depth, post);
      printf("move %c%c%c%c\n", m.oc() + 'a', m.ol() + '1', m.dc() + 'a',
             m.dl() + '1');
      board.move(m);
      fprintf(log_file, "%s\n", board.getFen().c_str());
      fprintf(log_file, "rough score %+d\n\n", board.eval());
    }
    /* verifica fim de jogo */
    if (board.done()) {
      printf("%s\n", board.result.c_str());
      go = 0;
    }

    /* descarga dos buffers */
    fflush(stdout);
    fflush(log_file);
  }
  fprintf(log_file, "saindo...\n");
  return 0;
}

