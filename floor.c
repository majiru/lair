#include <u.h>
#include <libc.h>
#include <draw.h>
#include "lair.h"

void
drawtofloor(Floor *f, Rectangle r, int tile)
{
	int i, j;

	for(i = r.min.x; i < r.max.x; i++)
		for(j = r.min.y; j < r.max.y; j++)
			f->map[i * f->rows + j] = tile;
}

void
drawtile(Floor *f, Point p, int tile)
{
	Point min, max;

	min.x = p.x * TILESIZE + screen->r.min.x;
	min.y = p.y * TILESIZE + screen->r.min.y;
	max = addpt(min, Pt(TILESIZE, TILESIZE));
	draw(screen, Rpt(min, max), f->colorset[tile], nil, f->tileorigin[tile]);
}

void
drawfloor(Floor *f)
{
	int i, j;

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++)
			drawtile(f, Pt(i, j), f->map[i*f->rows + j]);
}

Point
randempty(Floor *f)
{
	Point p;

	do{
		p.x	= RRANGE(1, f->cols - 2);
		p.y	= RRANGE(1, f->rows - 2);
	}while(f->map[p.x * f->rows + p.y] != TRoom);

	return p;
}

Point
spawnentity(Floor *f, int tile)
{
	Point p = randempty(f);
	drawtile(f, p, tile);
	return p;
}

int
moveentity(Floor *f, Point src, Point dest, int tile)
{
	if(f->map[dest.x * f->rows + dest.y] == TEmpty)
		return 0;
	drawtile(f, src, f->map[src.x*f->rows + src.y]);
	drawtile(f, dest, tile);
	return 1;
}

void
inititems(Floor *f)
{
	Point p;
	int i;
	for(i = 0; i < PORTALMAX; i++){
		p = randempty(f);
		f->map[p.x * f->rows + p.y] = TPortal;
	}
}

int
addroom(Floor *f, Rectangle r)
{
	int i;
	Rectangle room;
	for(i = 0; i < f->nrooms; i++){
		//Rooms are not allowed to be directly adjacent
		room.min = subpt(f->rooms[i].min, Pt(1,1));
		room.max = addpt(f->rooms[i].max, Pt(1,1));
		if(overlaps(r, room))
			return 1;
	}
	f->rooms[f->nrooms++] = r;
	return 0;
}

void
initrooms(Floor *f)
{
	Rectangle r;
	int failed;

	for(failed = 0; f->nrooms < ROOMNUMMAX && failed < ROOMNUMMAX;){
		r.max.x = RRANGE(ROOMMINX, ROOMMAXX);
		r.max.y = RRANGE(ROOMMINY, ROOMMAXY);
		r.min.x	= RRANGE(1, f->cols - 2 - r.max.x);
		r.min.y	= RRANGE(1, f->rows - 2 - r.max.y);
		r.max 	= addpt(r.min, r.max);

		failed += addroom(f, r);
	}
}

void
path(Floor *f, Rectangle r1, Rectangle r2)
{
	Rectangle r;
	if(r1.min.x * r1.min.y < r2.min.x * r2.min.y)
		r = Rpt(r1.min, r2.min);
	else
		r = Rpt(r2.min, r1.min);
	print("%R %R %R\n", r1, r2, r);
	drawtofloor(f, Rect(r.min.x, r.min.y, r.max.x+1, r.min.y+1), TTunnel);
	drawtofloor(f, Rect(r.max.x, r.min.y, r.max.x+1, r.max.y+1), TTunnel);
}

void
resizefloor(Floor *f)
{
	int cursize, newsize;
	int newrows, newcols;

	newrows = Dy(screen->r) / TILESIZE;
	newcols = Dx(screen->r) / TILESIZE;
	if(newcols < 5 || newrows < 5)
		sysfatal("Not enough space to create game board...");

	cursize = f->rows * f->cols;
	newsize = newrows * newcols;
	if(newsize != cursize){
		f->map = realloc(f->map, sizeof(int) * newsize);
		f->map = memset(f->map, TEmpty, sizeof(int) * newsize);
	}

	f->rows = newrows;
	f->cols = newcols;
}