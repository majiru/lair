#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

int
creeptypefmt(Fmt *f)
{
	CreepAbil type;
	int r = 0;

	type = va_arg(f->args, CreepAbil);
	if((type & CIntel) == CIntel)
		r += fmtprint(f, "SMART ");
	if((type & CTele) == CTele)
		r += fmtprint(f, "TELE ");
	if((type & CErratic) == CErratic)
		r += fmtprint(f, "ERRATIC ");
	if((type & CTunnel) == CTunnel)
		r += fmtprint(f, "TUNNEL ");
	if((type & CUniq) == CUniq)
		r += fmtprint(f, "UNIQ ");
	if((type & CPass) == CPass)
		r += fmtprint(f, "PASS ");
	if((type & CPickup) == CPickup)
		r += fmtprint(f, "PICKUP ");
	if((type & CDestroy) == CDestroy)
		r += fmtprint(f, "DESTROY ");
	if((type & CBoss) == CBoss)
		r += fmtprint(f, "BOSS ");

	return r;
}

int
dicefmt(Fmt *f)
{
	Dice *d;

	d = va_arg(f->args, Dice*);
	return fmtprint(f, "%d+%dd%d", d->base, d->ndie, d->nside);
}

int
creeplexfmt(Fmt *f)
{
	CreepLex *l;
	int r;

	l = va_arg(f->args, CreepLex*);
	r = fmtprint(f, "%s\n", l->name);
	r += fmtprint(f, "%s", l->desc);
	r += fmtprint(f, "%s\n", l->color);
	r += fmtprint(f, "%D\n", l->speed);
	r += fmtprint(f, "%U\n", l->type);
	r += fmtprint(f, "%D\n", l->HP);
	r += fmtprint(f, "%D\n", l->dam);
	r += fmtprint(f, "%c\n", l->tile);
	r += fmtprint(f, "%d\n", l->rarity);
	return r;
}

void
lairfmtinstall(void)
{
	fmtinstall('U', creeptypefmt);
	fmtinstall('C', creeplexfmt);
	fmtinstall('D', dicefmt);
}