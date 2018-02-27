all: Makefile.qmake
	make -f Makefile.qmake
	./stbsrisrates

Makefile.qmake: stbsrisrates.pro
	qmake $< -o $@

clean:
	make -f Makefile.qmake distclean
