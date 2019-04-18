</$objtype/mkfile
BIN=/$objtype/bin
CFLAGS=$CFLAGS -p
LDFLAGS=-p
TARG=lair
OFILES=\
	lair.$O \
	floor.$O \
	util.$O \
	file.$O \
	path.$O \
	creep.$O \
	menu.$O \
	fmt.$O \
	item.$O \
	y.tab.$O\
	lex.yy.$O \

YFILES=\
	monster.y\

LFILES=\
	monster.l\


LFLAGS=-9
HFILES=lair.h

</sys/src/cmd/mkone

lex.yy.$O: y.tab.h
