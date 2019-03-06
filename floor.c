#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

void
drawtotile(Floor *f, Point p, char tile)
{
	f->map[p.x * f->rows + p.y].type = tile;
}

void
drawtofloor(Floor *f, Rectangle r, char tile)
{
	int i, j;

	for(i = r.min.x; i < r.max.x; i++)
		for(j = r.min.y; j < r.max.y; j++)
			f->map[i * f->rows + j].type = tile;
}

void
drawtile(Floor *f, Point p, char tile)
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
			drawtile(f, Pt(i, j), f->map[i*f->rows + j].type);
}

Point
randempty(Floor *f)
{
	Point p;

	do{
		p.x	= RRANGE(1, f->cols - 2);
		p.y	= RRANGE(1, f->rows - 2);
	}while(f->map[p.x * f->rows + p.y].type != TRoom);

	return p;
}

Point
spawnentity(Floor *f, char tile)
{
	Point p = randempty(f);
	drawtile(f, p, tile);
	return p;
}

int
minewall(Floor *f, Point p)
{
	int n = MAPINDEXPT(f, p);
	if(f->map[n].hardness <= 85){
		f->map[n].hardness = 0;
		f->map[n].type = TTunnel;
		return 1;
	}
	f->map[n].hardness -= 85;
	return 0;
}


int
moveentity(Floor *f, Point src, Point dest, char tile, int canmine)
{
	if(f->map[dest.x * f->rows + dest.y].type == TEmpty)
		if(canmine == 0 || minewall(f, dest) == 0)
			return 0;
	drawtile(f, src, f->map[src.x*f->rows + src.y].type);
	drawtile(f, dest, tile);
	return 1;
}

void
inititems(Floor *f)
{
	int i;
	for(i = 0; i < PORTALMAX; i++)
		drawtotile(f, randempty(f), TPortal);
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
	Point p1 = r1.min;
	Point p2 = r2.min;
	if(p1.x > p2.x){
		drawtofloor(f, Rpt(p2, Pt(p1.x, p2.y + 1)), TTunnel);
		if(p1.y > p2.y)
			drawtofloor(f, Rpt(Pt(p1.x - 1, p2.y), Pt(p1.x + 1, p2.y + 1)), TTunnel);
		else
			drawtofloor(f, Rpt(Pt(p1.x - 1, p1.y - 1), Pt(p1.x, p2.y + 1)), TTunnel);
	}else{
		drawtofloor(f, Rpt(p1, Pt(p2.x + 1, p1.y + 1)), TTunnel);
		if(p2.y > p1.y)
			drawtofloor(f, Rpt(Pt(p2.x, p1.y), Pt(p2.x + 1, p2.y)), TTunnel);
		else
			drawtofloor(f, Rpt(Pt(p2.x, p2.y + Dy(r2)), Pt(p2.x + 1, p1.y + 1)), TTunnel);
	}
}

void
assignhardness(Floor *f)
{
	int i, j;
	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++)
			switch(f->map[MAPINDEX(f, i, j)].type){
			case TTunnel:
			case TRoom:
				f->map[MAPINDEX(f, i, j)].hardness = 0;
				break;
			case TEmpty:
				if(i == 0 || j == 0 || i == f->cols - 1 || j == f->rows - 1)
					f->map[MAPINDEX(f, i, j)].hardness = 255;
				else
					f->map[MAPINDEX(f, i, j)].hardness = RRANGE(1, 254);
				break;
			}
}

void
resizefloor(Floor *f)
{
	int newsize;
	int newrows, newcols;

	newrows = Dy(screen->r) / TILESIZE;
	newcols = Dx(screen->r) / TILESIZE;
	if(newcols < 10 || newrows < 10)
		sysfatal("Not enough space to create game board...");

	if(f->rows == newrows && f->cols == newcols)
		return;

	newsize = newrows * newcols;
	f->map = realloc(f->map, sizeof(Tile) * newsize);
	f->map = memset(f->map, 0, sizeof(Tile) * newsize);

	f->rows = newrows;
	f->cols = newcols;
}