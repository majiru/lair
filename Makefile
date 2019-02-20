CC := ./9c
LD := ./9l
all: floor.c lair.c tile.c util.c
	PLAN9=./ ./9c floor.c lair.c tile.c util.c
	PLAN9=./ ./9l -o o.lair floor.o lair.o tile.o util.o

clean:
	rm -f o.* *.o

vendor:
	mkdir -p lib include
	cp $(PLAN9)/lib/*.a lib/
	cp $(PLAN9)/include/*.h include/
	cp $(PLAN9)/bin/9c ./
	cp $(PLAN9)/bin/9l ./

nuke:
	rm -rf ./lib ./include ./9c ./9l
