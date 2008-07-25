CXX=g++

CXXFLAGS=-Wall -O3 -fomit-frame-pointer -march=native -mfpmath=sse
#CXXFLAGS=-Wall -ggdb3
#CXXFLAGS=-Wall -ggdb3 -O3 -pg -D_NOINLINE
#CXXFLAGS=-Wall -ggdb3 -O3 -D_NOINLINE
#CXXFLAGS=-Wall -O3 -fprofile-arcs
#CXXFLAGS=-Wall -O3 -fbranch-probabilities

#CXXFLAGS+=-D_DEBUG

bin/chess : src/main.cc src/board_code.hh src/board.hh src/debug.hh src/hash.hh src/move.hh src/random.hh src/search.hh src/bitboard.hh Makefile
	${CXX} src/main.cc -o bin/chess -pipe ${CXXFLAGS}
