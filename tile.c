#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

void
addtile(Floor *f, char *file, Point p, int tile)
{
	int fd;
	if((fd = open(file, OREAD)) < 0)
		sysfatal("Lair: Could not find tilemap %s", file);

	f->colorset[tile] = readimage(display, fd, 0);
	replclipr(f->colorset[tile], 1, Rpt(p, Pt(p.x + TILESIZE, p.y + TILESIZE)));
	f->tileorigin[tile] = p;
	close(fd);
}

Floor*
loadtilemap(char *tiles, char *walls)
{
	Floor *f;
	
	f = mallocz(sizeof(Floor), 1);
	f->map = malloc(sizeof(Tile));

	f->colorset = malloc(sizeof(Image*) * TNUM);
	f->tileorigin = malloc(sizeof(Point) * TNUM);

	addtile(f, walls, Pt(16, 128), TEmpty);
	addtile(f, tiles, Pt(16, 128), TRoom);
	addtile(f, walls, Pt(32, 128), TPortal);
	addtile(f, tiles, Pt(96, 16), TTunnel);
	addtile(f, tiles, Pt(128, 224), TPlayer);
	addtile(f, tiles, Pt(0, 160), TCreep);
	addtile(f, tiles, Pt(16, 160), TCreepM);
	addtile(f, tiles, Pt(32, 160), TCreepB);
	addtile(f, tiles, Pt(48, 160), TCreepE);

	return f;
}

