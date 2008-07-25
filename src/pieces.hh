#ifndef PIECES_HH
#define PIECES_HH


/* pieces types */
#define CLEAR 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

/* players */
#define BLACK 0
#define WHITE 1  
#define NONE 2  

/* gives the opponent of a player */
#define OPPONENT(p) ((p)^1)


#endif
