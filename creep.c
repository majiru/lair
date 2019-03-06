#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

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

		if(rand() % 5 > 2)
			c->type |= CTunnel;

		if(rand() % 2 == 0)
			c->type |= CErratic;

		c->tile = RRANGE(TCreep, TCreepE);
		c->pos = spawnentity(f, c->tile);
		f->creeps[f->ncreep++] = c;
	}
}

void
tickcreep(Floor *f)
{
	int i;
	Point dest;
	int lowest;

	for(i = 0; i < f->ncreep; i++){
		lowest = f->map[MAPINDEX(f, f->creeps[i]->pos.x + 1, f->creeps[i]->pos.y)].pcdistance;
		dest = addpt(f->creeps[i]->pos, Pt(1, 0));

		if(f->map[MAPINDEX(f, f->creeps[i]->pos.x - 1, f->creeps[i]->pos.y)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, f->creeps[i]->pos.x - 1, f->creeps[i]->pos.y)].pcdistance;
			dest = subpt(f->creeps[i]->pos, Pt(1, 0));
		}

		if(f->map[MAPINDEX(f, f->creeps[i]->pos.x, f->creeps[i]->pos.y + 1)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, f->creeps[i]->pos.x - 1, f->creeps[i]->pos.y + 1)].pcdistance;
			dest = addpt(f->creeps[i]->pos, Pt(0, 1));
		}

		if(f->map[MAPINDEX(f, f->creeps[i]->pos.x, f->creeps[i]->pos.y - 1)].pcdistance < lowest){
			lowest = f->map[MAPINDEX(f, f->creeps[i]->pos.x - 1, f->creeps[i]->pos.y - 1)].pcdistance;
			dest = subpt(f->creeps[i]->pos, Pt(0, 1));
		}

		if(moveentity(f, f->creeps[i]->pos, dest, f->creeps[i]->tile, (f->creeps[i]->type && CTunnel) == 0) != 0)
			f->creeps[i]->pos = dest;
	}
}
