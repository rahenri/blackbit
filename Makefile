CXX=g++

chess : main.cc main.hh Makefile
	${CXX} main.cc -Wall -o chess -pipe -ggdb3
	#${CXX} main.cc -Wall -o chess -pipe -g -pg -O2
	#${CXX} main.cc -O3 -Wall -o chess -pipe -fomit-frame-pointer -march=native -funroll-loops
