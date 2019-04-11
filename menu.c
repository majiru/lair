#include <u.h>
#include <libc.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

static Point cur;

void
resetcur(void)
{
	Rectangle tmp;
	tmp.min.x = 0;
	tmp.min.y = Dy(screen->r) - 250;
	cur = addpt(tmp.min, screen->r.min);	
}

void
monstermenu(Floor *f, Point *p)
{
	Rectangle menu;
	Point tmp;

	static int initialized = 0;
	Point drawpos;
	int i;
	char buf[256];
	char *cardinal[2];

	menu.min.x = 0;
	menu.min.y = Dy(screen->r) - 250;
	menu.min = addpt(menu.min, screen->r.min);
	menu.max = addpt(menu.min, Pt(300, 300));

	if(initialized == 0){
		cur = menu.min;
		initialized++;
	}

	if(p != nil){
		tmp = addpt(cur, mulpt(*p, font->height));
		cur = tmp.x >= menu.min.x && tmp.y >= menu.min.y ? tmp : menu.min;
	}

	draw(screen, menu, display->white, nil, ZP);
	for(i = (cur.y - menu.min.y) / font->height, drawpos = subpt(cur, Pt(0, i * font->height)); i < f->ncreep && drawpos.y < menu.max.y; ++i, drawpos.y += font->height){
		tmp = subpt(f->playpos, f->creeps[i]->pos);

		if(tmp.x > 0)
			cardinal[0] = "west";
		else if(tmp.x < 0){
			cardinal[0] = "east";
			tmp.x *= -1;
		}else
			cardinal[0] = "weast";

		if(tmp.y > 0)
			cardinal[1] = "north";
		else if(tmp.y < 0){
			cardinal[1] = "south";
			tmp.y *= -1;
		}else
			cardinal[1] = "sorth";

		draw(screen, Rpt(drawpos, addpt(drawpos, Pt(TILESIZE, TILESIZE))), f->tilesheet, nil, Pt(TILESIZE * f->creeps[i]->info->tile, (curdepth % PALETTENUM) * TILESIZE));
		snprint(buf, 256, "%s: %d %s, %d %s", f->creeps[i]->info->name, tmp.x, cardinal[0], tmp.y, cardinal[1]);
		string(screen, addpt(drawpos, Pt(TILESIZE, 0)), black, addpt(drawpos, Pt(TILESIZE, 0)), font, buf);
	}
}
