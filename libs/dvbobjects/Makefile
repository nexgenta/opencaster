DESTDIR = /usr/local/bin/

all:
	./setup.py build

install: all
	./setup.py install
clean:
	./setup.py clean
	rm -rf build/*

-include $(wildcard *.d) dummy

