#include <u.h>
#include <libc.h>
#include <draw.h>
#include "lair.h"

Floor*
loadtilemap(char *tiles, char *walls)
{
	int fd;
	Floor *f;
	
	f = mallocz(sizeof(Floor), 1);
	f->map = malloc(sizeof(int));

	f->colorset = malloc(sizeof(Image*) * TNUM);
	f->tileorigin = malloc(sizeof(Point) * TNUM);
	if((fd = open(walls, OREAD)) < 0)
		sysfatal("Lair: Could not find tilemap %s", walls);
	
	f->colorset[TEmpty] = readimage(display, fd, 0);
	replclipr(f->colorset[TEmpty], 1, Rect(16, 128, 32, 144));
	f->tileorigin[TEmpty] = Pt(16, 128);

	close(fd);
	if((fd = open(tiles, OREAD)) < 0)
		sysfatal("Lair: Could not find tilemap %s", tiles);
	
	f->colorset[TRoom] 	= readimage(display, fd, 0);
	replclipr(f->colorset[TRoom], 1, Rect(16, 128, 32, 144));
	f->tileorigin[TRoom] = Pt(16, 128);

	close(fd);
	if((fd = open(walls, OREAD)) < 0)
		sysfatal("Lair: Could not find tilemap %s", walls);

	f->colorset[TPortal] = readimage(display, fd, 0);
	replclipr(f->colorset[TPortal], 1, Rect(32, 128, 48, 144));
	f->tileorigin[TPortal] = Pt(32, 128);
	
	close(fd);
	if((fd = open(tiles, OREAD)) < 0)
		sysfatal("Lair: Could not find tilemap %s", tiles);
	
	f->colorset[TTunnel] = readimage(display, fd, 0);
	replclipr(f->colorset[TTunnel], 1, Rect(96, 16, 112, 32));
	f->tileorigin[TTunnel] = Pt(96, 16);

	close(fd);
	return f;
}