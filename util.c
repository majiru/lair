#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

int
overlaps(Rectangle r1, Rectangle r2)
{
	return 	r1.min.x < r2.max.x && r1.max.x > r2.min.x &&
    		r1.min.y < r2.max.y && r1.max.y > r2.min.y;
}

int
within(Rectangle r, Point p)
{
	return p.x >= r.min.x && p.x <= r.max.x &&
		p.y >= r.min.y && p.y <= r.max.y;
}

int
isbigendian(void)
{
	uint x = 0x76543210;
	char *c = (char*) &x;
	
	if(*c == 0x10)
		return 0;

	return 1;
}

void
roledie(Dice *d)
{
	int i;
	d->last = d->base;
	for(i = 0; i < d->ndie; ++i)
		d->last += RRANGE(1, d->nside);
}

Dice*
str2dice(char *str)
{
	char *fields[3];
	Dice *d = mallocz(sizeof(Dice), 1);
	int n;

	n = getfields(str, fields, 3, 0, "+d");
	if(n != 3)
		return nil;

	d->base = atoi(fields[0]);
	d->ndie = atoi(fields[1]);
	d->nside = atoi(fields[2]);

	roledie(d);

	return d;
}
