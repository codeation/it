CFLAGS=$(shell pkg-config --cflags gtk4)
LDFLAGS=$(shell pkg-config --libs gtk4)

it: call.o draw.o event.o idlist.o io.o layout.o main.o menu.o pipe.o version.o window.o
	gcc -o it call.o draw.o event.o idlist.o io.o layout.o main.o menu.o pipe.o version.o window.o $(LDFLAGS) -Wall -O3 -ffast-math -lm

%.o: %.c terminal.h idlist.h
	gcc -c $(CFLAGS) -Wall -O3 -ffast-math $<

clean:
	rm *.o
	rm it

format:
	clang-format -i terminal.h
	clang-format -i call.c
	clang-format -i draw.c
	clang-format -i event.c
	clang-format -i idlist.h
	clang-format -i idlist.c
	clang-format -i io.c
	clang-format -i layout.c
	clang-format -i main.c
	clang-format -i menu.c
	clang-format -i pipe.c
	clang-format -i version.c
	clang-format -i window.c
