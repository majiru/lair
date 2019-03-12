#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

Sprite*
mksprite(char sheetindex, int x, int y)
{
	Sprite *s = mallocz(sizeof(Sprite), 1);
	s->p = Pt(x * TILESIZE, y * TILESIZE);
	s->src = sheetindex;

	return s;
}

void
loadsheet(Floor *f, char *file, char sheetindex)
{
	int fd;
	if((fd = open(file, OREAD)) < 0)
		sysfatal("%s: Could not find tilemap %s", argv0, file);

	f->tilesheet[sheetindex] = readimage(display, fd, 0);
	close(fd);
}

Floor*
loadtilemap(char *tiles, char *walls)
{
	Floor *f;
	
	f = mallocz(sizeof(Floor), 1);
	f->map = malloc(sizeof(Tile));

	loadsheet(f, tiles, CSHEET);
	loadsheet(f, walls, WSHEET);

	f->sprites[TEmpty] = mksprite(WSHEET, 1, 8);
	f->sprites[TRoom] = mksprite(CSHEET, 1, 8);
	f->sprites[TPortal] = mksprite(WSHEET, 2, 8);
	f->sprites[TTunnel] = mksprite(CSHEET, 6, 1);
	f->sprites[TPlayer] = mksprite(CSHEET, 8, 14);
	f->sprites[TCreep] = mksprite(CSHEET, 0, 10);
	f->sprites[TCreepM] = mksprite(CSHEET, 1, 10);
	f->sprites[TCreepB] = mksprite(CSHEET, 2, 10);
	f->sprites[TCreepE] = mksprite(CSHEET, 3, 10);

	return f;
}

