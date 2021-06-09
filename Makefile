CXX=g++

CXXFLAGS=-O3 -fomit-frame-pointer -mfpmath=sse
#CXXFLAGS=-ggdb3
#CXXFLAGS=-ggdb3 -O3 -pg -D_NOINLINE
#CXXFLAGS=-ggdb3 -O3 -D_NOINLINE
#CXXFLAGS=-O3 -fprofile-arcs
#CXXFLAGS=-O3 -fbranch-probabilities

#CXXFLAGS+=-D_DEBUG

CXXFLAGS+=-O3 -Wall -pipe -march=native -flto

TESTS=$(wildcard tests/*.out)

HEADERS=src/board_code.hh src/board.hh src/debug.hh src/hash.hh src/move.hh src/random.hh src/search.hh src/bitboard.hh Makefile

OBJECTS=build/main.o build/search.o build/debug.o build/board.o build/bitboard.o build/random.o build/hash.o

all: blackbit

build/%.o: src/%.cpp ${HEADERS} Makefile
	@mkdir -p build
	${CXX} $< -c -o $@ ${CXXFLAGS}

blackbit: ${OBJECTS} Makefile
	${CXX} ${OBJECTS} -o blackbit -pipe ${CXXFLAGS}

%.out: %.in blackbit
	time ./blackbit < $< > $@

test: ${TESTS}

clean:
	rm blackbit
	rm -rf bin
