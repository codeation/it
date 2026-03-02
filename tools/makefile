CFLAGS=$(shell pkg-config --cflags gtk4)
LDFLAGS=$(shell pkg-config --libs gtk4)

font-families: font-families.o
	gcc -o font-families font-families.o $(LDFLAGS) -lm

%.o: %.c
	gcc -c $(CFLAGS) $<

clean:
	rm *.o
	rm font-families

format:
	clang-format -i font-families.c
