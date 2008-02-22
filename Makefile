CXX=g++

chess : main.cc main.hh Makefile
	${CXX} main.cc -g -Wall -o chess -pipe -pg
	#${CXX} main.cc -O3 -g -Wall -o chess -pipe -funroll_loops -fomit-frame-pointer
