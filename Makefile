CXX=g++

CXXFLAGS=-O3 -fomit-frame-pointer -mfpmath=sse
#CXXFLAGS=-ggdb3
#CXXFLAGS=-ggdb3 -O3 -pg -D_NOINLINE
#CXXFLAGS=-ggdb3 -O3 -D_NOINLINE
#CXXFLAGS=-O3 -fprofile-arcs
#CXXFLAGS=-O3 -fbranch-probabilities

#CXXFLAGS+=-D_DEBUG

CXXFLAGS+=-O3 -Wall -pipe -march=native -flto

TESTS_INPUTS=$(wildcard tests/*.in)
TESTS_OUTPUTS=$(patsubst %.in, %.out, ${TESTS_INPUTS})

HEADERS=src/board_code.h src/board.h src/debug.h src/hash.h src/move.h src/random.h src/search.h src/bitboard.h Makefile

SOURCES=$(wildcard src/*.cpp)
OBJECTS=$(shell find src | grep cpp$ | sed 's:^src:build:' | sed 's:cpp$$:o:')

all: blackbit

build/%.o: src/%.cpp ${HEADERS} Makefile
	@mkdir -p build
	${CXX} $< -c -o $@ ${CXXFLAGS}

blackbit: ${OBJECTS} Makefile
	${CXX} ${OBJECTS} -o blackbit -pipe ${CXXFLAGS}

%.out: %.in blackbit
	time ./blackbit < $< > $@

test: ${TESTS_OUTPUTS}
	echo ${TESTS_OUTPUTS}

clean:
	rm -f blackbit
	rm -rf build
