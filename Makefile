all: rshell ls

rshell:
	g++ src/main.cpp -o rshell -Wall -Werror -pedantic --std=c++11
	mkdir -p bin
	mv rshell bin/

cp:
	g++ src/cp.cpp -o cp -Wall -Werror -pedantic --std=c++11
	mkdir -p bin
	mv cp bin/
	
ls:
	g++ src/ls.cpp -o ls -Wall -Werror -pedantic --std=c++11
	mkdir -p bin
	mv ls bin/

ls_debug:
	g++ src/ls.cpp -o ls -Wall -Werror -pedantic --std=c++11 -g
	mkdir -p bin
	mv ls bin/

rm:
	mkdir -p ./bin
	g++ -Wall -Werror --std=c++11 -pedantic ./src/rm.cpp -o ./bin/rm

mv:
	mkdir -p ./bin
	g++ -Wall -Werror --std=c++11 -pedantic ./src/mv.cpp -o ./bin/mv
