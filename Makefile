CXX=g++

CXXFLAGS=-Wall -O3 -fomit-frame-pointer -march=native -funroll-loops -g
#CXXFLAGS=-Wall -ggdb3
#CXXFLAGS=-Wall -ggdb3 -O2 -pg

#CXXFLAGS+=-D_DEBUG

chess : main.cc main.hh Makefile
	${CXX} main.cc -o chess -pipe ${CXXFLAGS}
