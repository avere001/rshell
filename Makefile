all: rshell ls

rshell:
	g++ src/main.cpp -o rshell -Wall -Werror -pedantic --std=c++11
	mkdir -p bin
	mv rshell bin/

ls:
	g++ src/ls.cpp -o ls -Wall -Werror -pedantic --std=c++11
	mkdir -p bin
	mv ls bin/

ls_debug:
	g++ src/ls.cpp -o ls -Wall -Werror -pedantic --std=c++11 -g
	mkdir -p bin
	mv ls bin/


