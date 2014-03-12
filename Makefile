CFLAGS=`pkg-config --cflags poppler-glib glib-2.0`
LDFLAGS=`pkg-config --libs poppler-glib glib-2.0`

main: main.o

main.o: main.c

.PHONY: clean

clean:
	rm -f main main.o
