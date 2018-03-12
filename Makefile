.PHONY: all server clean

all: Makefile.qmake
	make -f Makefile.qmake
	./stbsrisrates

server:
	g++ -o stbsrisrates-dedicated -std=c++17 -O2 -DFREE_SERVER Server.cpp GameState.cpp Log.cpp network.cpp -pthread -s

Makefile.qmake: stbsrisrates.pro
	qmake $< -o $@

clean:
	make -f Makefile.qmake distclean
