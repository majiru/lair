#include <u.h>
#include <libc.h>
#include <draw.h>
#include <mp.h>
#include <heap.h>

#include "lair.h"

u16int
readbe16(int fd, mpint **b)
{
	u16int x;

	readn(fd, &x, sizeof(u16int));
	*b = betomp((uchar *)&x, sizeof(u16int), *b);
	mptole(*b, (uchar *)&x, sizeof(u16int), nil);
	return x;
}

u32int
readbe32(int fd, mpint **b)
{
	u32int x;

	readn(fd, &x, sizeof(u32int));
	*b = betomp((uchar *)&x, sizeof(u32int), *b);
	mptole(*b, (uchar *)&x, sizeof(u32int), nil);
	return x;
}

Point
readbept(int fd, mpint **b)
{
	uchar buf[16];
	Point p;

	readn(fd, buf, 16);

	*b = betomp(buf, 8, *b);
	mptole(*b, buf, 8, nil);
	p.x = (u8int)*buf;
	
	*b = betomp(buf+8, 8, *b);
	mptole(*b, buf, 8, nil);
	p.y = (u8int)*buf;

	return p;
}

void
openfile(Floor *f, char *name)
{
	mpint *b = nil;
	int fd;
	int i, j;
	uchar buf[2048];
	u32int size;

	if((fd = open(name, OWRITE)) < 0)
		sysfatal("Lair: Could not find dungeon file %s", name);
	
	/* Read Header */
	readn(fd, buf, 12);
	buf[13] = '\0';
	if(strcmp((char*)buf, "RLG327-S2019") != 0)
		sysfatal("Lair: Invalid File Header");

	/* Read version */
	readn(fd, buf, 32);
	if((u32int)*buf != 0)
		sysfatal("Lair: Invalid File Version");

	/* Read size */
	size = readbe32(fd, &b);
	USED(size);

	/* Read Player Position */
	f->playpos = readbept(fd, &b);

	/* These files are built for ncurses
	* Meaning they are for 80x21 by default. Gross.
	*/
	f->rows = 21;
	f->cols = 80;
	f->map = realloc(f->map, 80 * 21 * sizeof(Tile));
	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++)
			read(fd, &((f->map+(j * f->rows + i))->hardness), 1);

	/* Read number of rooms */
	f->nrooms = readbe16(fd, &b);

	for(i = 0; i < f->nrooms; i++){
		f->rooms[i] = Rpt(readbept(fd, &b), readbept(fd, &b));
		drawtofloor(f, f->rooms[i], TRoom);
	}
	
	j = readbe16(fd, &b);
	for(i = 0; i < j; i++)
		drawtotile(f, readbept(fd, &b), TPortalD);

	j = readbe16(fd, &b);
	for(i = 0; i < j; i++)
		drawtotile(f, readbept(fd, &b), TPortalU);

	close(fd);
}
