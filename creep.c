#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <memdraw.h>
#include <mouse.h>
#include <nuklear.h>
#include <heap.h>

#include "lair.h"

Creep*
point2creep(Floor *f, Point p)
{
	int i;
	for(i = 0; i < f->ncreep; i++)
		if(eqpt(f->creeps[i]->pos, p))
			return f->creeps[i];

	return nil;
}

void
docombat(Creep *player, Creep *creep)
{
	List *cur, *start;
	int dam = RRANGE(1, 5);

	start = cur = curfloor->player->equipment;

	for(cur = cur->next; cur != start; cur = cur->next)
		dam += ((ItemLex*)cur->datum)->dam->last;

	creep->health -= dam;

	player->health -= creep->info->dam->last;

	if(player->health < 0){
		sysfatal("You lost...");
	}

	if((creep->info->type & CBoss) == CBoss && creep->health < 0)
		sysfatal("You Won!");
}

int
isoccupied(Floor *f, Point p, Creep *c)
{
	int i;

	if(eqpt(f->player->pos, p)){
		docombat(f->player, c);
		return 1;
	}

	for(i = 0; i < f->ncreep; i++)
		if(eqpt(f->creeps[i]->pos, p) && f->creeps[i]->health > 0){
			/* Hacky way to check if c is player */
			if(c->info == nil)
				docombat(c, f->creeps[i]);
			return 1;
		}

	return 0;
}

void
spawncreep(Floor *f)
{
	int i, n;
	Creep *c;
	CreepLex *l;

	for(i = NUMCREEP; i > 0;){
		l = creeplexicon[RRANGE(0, ncreeplex)];
		n = RRANGE(0, 100);
		if(n >= l->rarity)
			continue;

		if(l->spawned != 0 && ((l->type & CUniq) == CUniq))
			continue;

		l->spawned = 1;

		c = mallocz(sizeof(Creep), 1);
		c->info = l;
		c->pos = spawnentity(f, c->info->tile);
		f->creeps[f->ncreep++] = c;
		c->health = c->info->HP->last;
		roledie(c->info->HP);
		--i;
	}
}

void
redrawcreep(Floor *f)
{
	int i;
	for(i = 0; i < f->ncreep; i++)
		if(f->creeps[i]->health > 0)
		if(cheatDefog == 1 || f->map[MAPINDEXPT(f, f->creeps[i]->pos)].seen == 1)
			drawtile(f, f->creeps[i]->pos, f->creeps[i]->info->tile);
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

		if(f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y + 1)].pcdistance < lowest)
			dest = Pt(c->pos.x - 1, c->pos.y + 1);

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

		if(f->map[MAPINDEX(f, c->pos.x - 1, c->pos.y + 1)].tunneldistance < lowest)
			dest = Pt(c->pos.x - 1, c->pos.y + 1);

		return dest;
}

Point
movedumb(Floor *f, Creep *c)
{
	Point dst = c->pos;
	Point tmp;

	tmp = subpt(f->player->pos, dst);

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

	tickdjikstra();

	for(i = 0; i < f->ncreep; i++){
		if(f->creeps[i]->health < 0)
			continue;
		if((f->creeps[i]->info->type & CIntel) == CIntel){
			if((f->creeps[i]->info->type & CTunnel) == CTunnel)
				dest = moveinteltunnel(f, f->creeps[i]);
			else
				dest = moveintel(f, f->creeps[i]);
		}else
			dest = movedumb(f, f->creeps[i]);

		if((f->creeps[i]->info->type & CErratic) == CErratic)
			if(rand() % 2 == 0)
				dest = moveerratic(f->creeps[i]);

		if(moveentity(f, dest, (f->creeps[i]->info->type & CTunnel) == CTunnel, f->creeps[i]) != 0)
			f->creeps[i]->pos = dest;
	}
}
