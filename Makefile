CXX=g++

CXXFLAGS=-O3 -fomit-frame-pointer -mfpmath=sse
#CXXFLAGS=-ggdb3
#CXXFLAGS=-ggdb3 -O3 -pg -D_NOINLINE
#CXXFLAGS=-ggdb3 -O3 -D_NOINLINE
#CXXFLAGS=-O3 -fprofile-arcs
#CXXFLAGS=-O3 -fbranch-probabilities

#CXXFLAGS+=-D_DEBUG

CXXFLAGS+=-O3 -Wall -pipe -march=native -flto

all: blackbit


blackbit: src/main.cc src/board_code.hh src/board.hh src/debug.hh src/hash.hh src/move.hh src/random.hh src/search.hh src/bitboard.hh Makefile
	${CXX} src/main.cc -o blackbit -pipe ${CXXFLAGS}

tests/test1.out: blackbit tests/test1.in
	./blackbit < tests/test1.in > tests/test1.out

tests/test2.out: blackbit tests/test2.in
	./blackbit < tests/test2.in > tests/test2.out

test: tests/test1.out tests/test2.out



clean:
	rm blackbit
