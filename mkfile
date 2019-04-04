</$objtype/mkfile
BIN=/$objtype/bin
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
