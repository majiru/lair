CC := 9c
LD := 9l

NOP9P: export PLAN9 = .
NOP9P: export CC = ./9c
NOP9P: export LD = ./9l
NOP9P: lair

lair: floor.c lair.c util.c path.c creep.c menu.c
	$(CC) floor.c lair.c util.c path.c creep.c menu.c
	$(LD) -o o.lair floor.o lair.o util.o path.o creep.o menu.o
	mv o.lair lair

clean:
	rm -f o.* *.o lair

vendor:
	mkdir -p lib include
	cp $(PLAN9)/lib/*.a lib/
	cp $(PLAN9)/include/*.h include/
	cp $(PLAN9)/bin/9c ./
	cp $(PLAN9)/bin/9l ./

nuke:
	rm -rf ./lib ./include ./9c ./9l
