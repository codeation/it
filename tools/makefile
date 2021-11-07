CFLAGS=$(shell pkg-config --cflags gtk+-3.0)
LDFLAGS=$(shell pkg-config --libs gtk+-3.0)

font-families: font-families.o
	gcc -o font-families $(LDFLAGS) -lm font-families.o

%.o: %.c terminal.h idlist.h
	gcc -c $(CFLAGS) $<

clean:
	rm *.o
	rm font-families

format:
	clang-format -i font-families.c
