CXX=g++

CXXFLAGS=-O3 -fomit-frame-pointer -mfpmath=sse
#CXXFLAGS=-ggdb3
#CXXFLAGS=-ggdb3 -O3 -pg -D_NOINLINE
#CXXFLAGS=-ggdb3 -O3 -D_NOINLINE
#CXXFLAGS=-O3 -fprofile-arcs
#CXXFLAGS=-O3 -fbranch-probabilities

#CXXFLAGS+=-D_DEBUG

CXXFLAGS+=-O3 -Wall -pipe -march=native -flto


bin/chess: src/main.cc src/board_code.hh src/board.hh src/debug.hh src/hash.hh src/move.hh src/random.hh src/search.hh src/bitboard.hh Makefile
	mkdir bin
	${CXX} src/main.cc -o bin/chess -pipe ${CXXFLAGS}


clean:
	rm bin/chess
