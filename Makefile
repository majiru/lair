CC := 9c
LD := 9l
YACC := yacc
LEX := lex
LIBNUKLEAR := ../../libnuklear

NOP9P: export PLAN9 = .
NOP9P: export CC = ./9c
NOP9P: export LD = ./9l
NOP9P: export LEX = ./lex
NOP9P: export YACC = ./yacc
NOP9P: lair

lair: floor.c lair.c util.c path.c creep.c menu.c
	$(YACC) -d monster.y
	$(LEX) -9 monster.l
	$(CC) floor.c lair.c util.c path.c creep.c menu.c fmt.c item.c nuklear.c lex.yy.c y.tab.c
	$(LD) -o lair floor.o lair.o util.o path.o creep.o menu.o fmt.o item.o nuklear.o lex.yy.o y.tab.o

clean:
	rm -f o.* *.o lair y.tab.c lex.yy.c y.tab.h

vendor:
	mkdir -p lib include
	cp $(PLAN9)/lib/*.a lib/
	cp $(PLAN9)/lib/yacc* lib/
	cp $(PLAN9)/lib/lex* lib/
	cp $(PLAN9)/include/*.h include/
	cp $(PLAN9)/bin/9c ./
	cp $(PLAN9)/bin/9l ./
	cp $(PLAN9)/bin/devdraw ./
	cp $(PLAN9)/bin/lex ./
	cp $(PLAN9)/bin/yacc ./
	cp $(LIBNUKLEAR)/nuklear.c ./
	cp $(LIBNUKLEAR)/nuklear.h ./


nuke:
	rm -rf ./lib ./include ./9c ./9l ./devdraw ./lex ./yacc ./nuklear.*
