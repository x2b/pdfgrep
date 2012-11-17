CFLAGS=`pkg-config --cflags poppler-glib gtk+-2.0` -O0 -g
LDFLAGS=`pkg-config --libs poppler-glib gtk+-2.0` -lpthread

main: main.o

main.o: main.c

.PHONY: clean

clean:
	rm -f main main.o
