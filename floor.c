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
drawfloor(Floor *f)
{
	int i, j;

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++){
			Point p1 = {i * TILESIZE + screen->r.min.x, j * TILESIZE + screen->r.min.y};
			Point p2 = addpt(p1, Pt(TILESIZE, TILESIZE));
			Rectangle r = {p1, p2};
			int tiletype = f->map[i*f->rows + j];
			draw(screen, r, f->colorset[tiletype], nil, f->tileorigin[tiletype]);
		}
}

int
additem(Floor *f, Point p, int tile)
{
	int n;
	n = p.x * f->rows + p.y;
	if(f->map[n] == TRoom){
		f->map[n] = tile;
		return 1;
	}
	return 0;
}

void
inititems(Floor *f)
{
	Point p;
	int failed, passed;
	for(failed = passed = 0; failed < 100 && passed < PORTALMAX;){
		int success;
		p.x	= RRANGE(1, f->cols - 2);
		p.y	= RRANGE(1, f->rows - 2);
		success = additem(f, p, TPortal);
		failed += !success;
		passed += success;
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
	drawtofloor(f, Rect(r.min.x, r.min.y, r.max.x, r.min.y+1), TTunnel);
	drawtofloor(f, Rect(r.min.x, r.min.y, r.min.x+1, r.max.y), TTunnel);
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
	if(newsize > cursize){
		f->map = realloc(f->map, sizeof(int) * newsize);
		f->map = memset(f->map, TEmpty, sizeof(int) * newsize);
	}

	f->rows = newrows;
	f->cols = newcols;
}