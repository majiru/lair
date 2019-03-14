#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

void
freefloor(Floor *f, int freesheet)
{
	while(f->ncreep != 0)
		free(f->creeps[f->ncreep--]);

	free(f->map);

	if(freesheet)
		freeimage(f->tilesheet);

	free(f);
}

void
initmap(Floor *f)
{
	int i;
	for(i = 0; i < f->nrooms - 1; i++)
		path(f, f->rooms[i], f->rooms[i+1]);

	for(i = 0; i < f->nrooms; i++)
		drawtofloor(f, f->rooms[i], TRoom);

	inititems(f);
	assignhardness(f);
}

void
initfloor(Floor *f)
{
	initrooms(f);
	initmap(f);
	f->playpos = spawnentity(f, TPlayer);
	spawncreep(f);
}

Floor*
newfloor(void)
{
	Floor *f;
	int fd;

	f = mallocz(sizeof(Floor), 1);
	f->map = malloc(sizeof(Tile));

	if((fd = open("tiles.img", OREAD)) < 0)
		sysfatal("%s: Could not find tilemap", argv0);

	f->tilesheet = readimage(display, fd, 0);
	close(fd);

	return f;
}

void
nextfloor(Floor **f)
{
	Floor *new = mallocz(sizeof(Floor), 1);
	new->map = malloc(sizeof(Tile));
	new->tilesheet = (*f)->tilesheet;
	resizefloor(new);

	freefloor(*f, 0);

	curdepth++;
	*f = new;
	initfloor(*f);
}

void
drawtotile(Floor *f, Point p, uchar tile)
{
	f->map[p.x * f->rows + p.y].type = tile;
}

void
drawtofloor(Floor *f, Rectangle r, uchar tile)
{
	int i, j;

	for(i = r.min.x; i < r.max.x; i++)
		for(j = r.min.y; j < r.max.y; j++)
			f->map[i * f->rows + j].type = tile;
}

void
drawtile(Floor *f, Point p, uchar tile)
{
	Point min, max;

	min.x = p.x * TILESIZE + screen->r.min.x;
	min.y = p.y * TILESIZE + screen->r.min.y;
	max = addpt(min, Pt(TILESIZE, TILESIZE));
	draw(screen, Rpt(min, max), f->tilesheet, nil, Pt(TILESIZE * tile, (curdepth % PALETTENUM) * TILESIZE));
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
spawnentity(Floor *f, uchar tile)
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
moveentity(Floor *f, Point src, Point dest, uchar tile, int canmine)
{
	if(isoccupied(f, dest))
		return 0;

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
	for(i = 0; i < PORTALMAX / 2; i++)
		drawtotile(f, randempty(f), TPortalD);
	for(i = 0; i < PORTALMAX / 2; i++)
		drawtotile(f, randempty(f), TPortalU);
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
	Point tmp;

	while(eqpt(r1.min,r2.min) == 0){
		tmp = subpt(r1.min, r2.min);

		if(tmp.x < 0)
			r1.min = addpt(r1.min, Pt(1,0));
		else if(tmp.x > 0)
			r1.min = subpt(r1.min, Pt(1,0));

		drawtotile(f, r1.min, TTunnel);

		if(tmp.y < 0)
			r1.min = addpt(r1.min, Pt(0,1));
		else if(tmp.y > 0)
			r1.min = subpt(r1.min, Pt(0,1));

		drawtotile(f, r1.min, TTunnel);
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
drawhardness(Floor *f)
{
	Point min;
	char buf[2];
	int i, j;

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++){
			min.x = i * TILESIZE + screen->r.min.x;
			min.y = j * TILESIZE + screen->r.min.y;
			snprint(buf, 2, "%d", (uchar) f->map[MAPINDEX(f, i, j)].hardness % 10);
			string(screen, min, display->white, min, font, buf);
		}

}

void
resizefloor(Floor *f)
{
	int newsize, oldsize;
	int newrows, newcols;

	newrows = Dy(screen->r) / TILESIZE;
	newcols = Dx(screen->r) / TILESIZE;
	if(newcols < 10 || newrows < 10)
		sysfatal("Not enough space to create game board...");

	if(f->rows == newrows && f->cols == newcols)
		return;

	newsize = newrows * newcols;
	oldsize = f->rows * f->cols;

	if(newsize > oldsize)
		f->map = realloc(f->map, sizeof(Tile) * newsize);
	f->map = memset(f->map, TEmpty, sizeof(Tile) * newsize);

	f->rows = newrows;
	f->cols = newcols;
}