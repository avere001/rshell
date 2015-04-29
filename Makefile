all: rshell

rshell:
	g++ src/main.cpp -o rshell -Wall -Werror -pedantic --std=c++11
	mkdir -p bin
	mv rshell bin/
cp:
	g++ src/cp.cpp -o cp -Wall -Werror -pedantic --std=c++11
	mkdir -p bin
	mv cp bin/
	
