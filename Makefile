all: rshell

rshell:
	g++ src/main.cpp -o rshell -Wall -Werror -ansi -pedantic --std=c++11
	mkdir -p bin
	mv rshell bin/
