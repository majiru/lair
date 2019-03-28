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
	Floor *newf = mallocz(sizeof(Floor), 1);
	newf->map = malloc(sizeof(Tile));
	newf->tilesheet = (*f)->tilesheet;
	resizefloor(newf);

	freefloor(*f, 0);

	curdepth++;
	*f = newf;
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

Rectangle
point2tile(Floor *f, Point p)
{
	Rectangle r;

	r.min.x = p.x * TILESIZE + screen->r.min.x;
	r.min.y = p.y * TILESIZE + screen->r.min.y;
	r.max = addpt(r.min, Pt(TILESIZE, TILESIZE));
	return r;
}

void
drawtile(Floor *f, Point p, uchar tile)
{
	Rectangle r;
	r = point2tile(f, p);

	if(tile == THidden)
		draw(screen, r, black, nil, Pt(TILESIZE * tile, (curdepth % PALETTENUM) * TILESIZE));
	else
		draw(screen, r, f->tilesheet, nil, Pt(TILESIZE * tile, (curdepth % PALETTENUM) * TILESIZE));
}

void
drawstringtile(Floor *f, Point p, char *str)
{
	Image *i;
	Rectangle r = point2tile(f, p);
	i = f->map[MAPINDEXPT(f, p)].type == TEmpty && f->map[MAPINDEXPT(f, p)].seen == 1 ? black : white;
	string(screen, r.min, i, r.min, font, str);
}

void
drawfloor(Floor *f)
{
	int i, j;

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++)
			if(cheatDefog == 0 && f->map[i*f->rows + j].seen == 0)
				drawtile(f, Pt(i, j), THidden);
			else
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

/* TODO: Combine this with drawtofloor, taking whole Tile as argument */
void
discoverrect(Floor *f, Rectangle r)
{
	int i, j;

	for(i = r.min.x; i < r.max.x; i++)
		for(j = r.min.y; j < r.max.y; j++)
			f->map[i * f->rows + j].seen = 1;
}

void
marknearby(Floor *f, Point p)
{
	int i;
	Point tmp;

	tmp.x = f->playpos.x > p.x ? f->playpos.x - p.x : p.x - f->playpos.x;
	tmp.y = f->playpos.y > p.y ? f->playpos.y - p.y : p.y - f->playpos.y;
	if(f->map[MAPINDEXPT(f, tmp)].type == TEmpty)
		tmp = mulpt(tmp, 2);

	if(tmp.x > VIEWDIST || tmp.y > VIEWDIST)
		return;

	f->map[MAPINDEXPT(f, p)].seen = 1;
	for(i = 1; i <= VIEWDIST; ++i){
		tmp = addpt(p, Pt(0, i));
		if(f->map[MAPINDEXPT(f, tmp)].seen == 0)
			marknearby(f, tmp);

		tmp = addpt(p, Pt(i, 0));
		if(f->map[MAPINDEXPT(f, tmp)].seen == 0)
			marknearby(f, tmp);

		tmp = subpt(p, Pt(0, i));
		if(f->map[MAPINDEXPT(f, tmp)].seen == 0)
			marknearby(f, tmp);

		tmp = subpt(p, Pt(i, 0));
		if(f->map[MAPINDEXPT(f, tmp)].seen == 0)
			marknearby(f, tmp);
	}
}

void
discover(Floor *f)
{
	int i;
	for(i = 0; i < f->nrooms; ++i)
		if(within(f->rooms[i], f->playpos)){
			discoverrect(f, Rpt(subpt(f->rooms[i].min, Pt(2,2)), addpt(f->rooms[i].max, Pt(2,2))));
			return;
		}

	/* Player must be in a hallway, lets do this the hard way */
	marknearby(f, f->playpos);
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