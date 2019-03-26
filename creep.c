#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

int
isoccupied(Floor *f, Point p)
{
	int i;

	if(eqpt(f->playpos, p))
		return 1;

	for(i = 0; i < f->ncreep; i++)
		if(eqpt(f->creeps[i]->pos, p))
			return 1;

	return 0;
}

void
spawncreep(Floor *f)
{
	int i;
	Creep *c;
	for(i = 0; i < NUMCREEP; i++){
		c = mallocz(sizeof(Creep), 1);
		if(rand() % 2 == 0)
			c->type |= CIntel;

		if(rand() % 2 == 0)
			c->type |= CTele;

		if(rand() % 2 == 0)
			c->type |= CTunnel;

		if(rand() % 2 == 0)
			c->type |= CErratic;

		c->tile = RRANGE(TCreep, TCreepE);
		c->pos = spawnentity(f, c->tile);
		f->creeps[f->ncreep++] = c;
	}
}

void
redrawcreep(Floor *f)
{
	int i;
	for(i = 0; i < f->ncreep; i++)
		if(cheatDefog == 1 || f->map[MAPINDEXPT(f, f->creeps[i]->pos)].seen == 1)
			drawtile(f, f->creeps[i]->pos, f->creeps[i]->tile);
}

Point
moveintel(Floor *f, Creep *c)
{
		int lowest = f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y)].pcdistance;
		Point dest = c->pos;

		if(f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y)].pcdistance;
			dest = subpt(c->pos, Pt(1, 0));
		}

		if(f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y)].pcdistance;
			dest = addpt(c->pos, Pt(1, 0));
		}

		if(f->map[MAPINDEX(f, c->pos.x, c->pos.y + 1)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x, c->pos.y + 1)].pcdistance;
			dest = addpt(c->pos, Pt(0, 1));
		}

		if(f->map[MAPINDEX(f, c->pos.x, c->pos.y - 1)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x, c->pos.y - 1)].pcdistance;
			dest = subpt(c->pos, Pt(0, 1));
		}

		if(f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y + 1)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y + 1)].pcdistance;
			dest = addpt(c->pos, Pt(1, 1));
		}

		if(f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y - 1)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y - 1)].pcdistance;
			dest = subpt(c->pos, Pt(1, 1));
		}

		if(f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y - 1)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y - 1)].pcdistance;
			dest = Pt(c->pos.x + 1, c->pos.y - 1);
		}

		if(f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y + 1)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y + 1)].pcdistance;
			dest = Pt(c->pos.x - 1, c->pos.y + 1);
		}

		return dest;
}

Point
moveinteltunnel(Floor *f, Creep *c)
{
		int lowest = f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y)].tunneldistance;
		Point dest = c->pos;

		if(f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y)].tunneldistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y)].tunneldistance;
			dest = subpt(c->pos, Pt(1, 0));
		}

		if(f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y)].tunneldistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y)].tunneldistance;
			dest = addpt(c->pos, Pt(1, 0));
		}

		if(f->map[MAPINDEX(f, c->pos.x, c->pos.y + 1)].tunneldistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x, c->pos.y + 1)].tunneldistance;
			dest = addpt(c->pos, Pt(0, 1));
		}

		if(f->map[MAPINDEX(f, c->pos.x, c->pos.y - 1)].tunneldistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x, c->pos.y - 1)].tunneldistance;
			dest = subpt(c->pos, Pt(0, 1));
		}

		if(f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y + 1)].tunneldistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y + 1)].tunneldistance;
			dest = addpt(c->pos, Pt(1, 1));
		}

		if(f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y - 1)].tunneldistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y - 1)].tunneldistance;
			dest = subpt(c->pos, Pt(1, 1));
		}

		if(f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y - 1)].tunneldistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x + 1, c->pos.y - 1)].tunneldistance;
			dest = Pt(c->pos.x + 1, c->pos.y - 1);
		}

		if(f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y + 1)].tunneldistance < lowest){
			lowest = f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y + 1)].tunneldistance;
			dest = Pt(c->pos.x - 1, c->pos.y + 1);
		}

		return dest;
}

Point
movedumb(Floor *f, Creep *c)
{
	Point dst = c->pos;
	Point tmp;

	tmp = subpt(f->playpos, dst);

	if(tmp.x > 0)
		dst = addpt(dst, Pt(1,0));
	else if(tmp.x < 0)
		dst = subpt(dst, Pt(1,0));

	if(tmp.y > 0)
		dst = addpt(dst, Pt(0,1));
	else if(tmp.y < 0)
		dst = subpt(dst, Pt(0,1));

	return dst;
}

Point
moveerratic(Creep *c)
{
	int x = (rand() % 2) - 1;
	int y = (rand() % 2) - 1;

	return addpt(c->pos, Pt(x, y));
}

void
tickcreep(Floor *f)
{
	int i;
	Point dest;
	int lowest;

	djikstra(f);
	djikstratunnel(f);

	for(i = 0; i < f->ncreep; i++){
		if((f->creeps[i]->type & CIntel) != 0){
			if((f->creeps[i]->type & CTunnel) != 0)
				dest = moveinteltunnel(f, f->creeps[i]);
			else
				dest = moveintel(f, f->creeps[i]);
		}else
			dest = movedumb(f, f->creeps[i]);

		if((f->creeps[i]->type & CErratic) != 0)
			if(rand() % 2 == 0)
				dest = moveerratic(f->creeps[i]);

		if(moveentity(f, f->creeps[i]->pos, dest, f->creeps[i]->tile, (f->creeps[i]->type & CTunnel) != 0) != 0)
			f->creeps[i]->pos = dest;
	}
}