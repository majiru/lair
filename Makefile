CC := ./9c
LD := ./9l
PLAN9 := ./
all: floor.c lair.c tile.c util.c
	./9c floor.c lair.c tile.c util.c
	./9l -o o.lair floor.o lair.o tile.o util.o

clean:
	rm -f o.* *.o
