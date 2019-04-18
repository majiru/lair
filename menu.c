#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <keyboard.h>
#include <event.h>
#include <nuklear.h>
#include <heap.h>

#include "lair.h"

static Point cur;

Image *splash = nil;

int
mainmenu(struct nk_context *ctx)
{
	int fd;
	struct nk_image nkimg;

	if(splash == nil){
		if((fd = open("lair.img", OREAD)) < 0)
			sysfatal("%s: Could not find tilemap", argv0);
		splash = readimage(display, fd, 0);
	}

	nkimg = nk_image_ptr(splash);

	if(nk_begin_titled(ctx, "Main Menu", "Lair", nk_rect(((Dx(screen->r)/2) - Dx(splash->r)/2), 0, Dx(splash->r),Dy(screen->r)),
		NK_WINDOW_BORDER)){
		nk_layout_row_dynamic(ctx, Dy(splash->r), 1);

		nk_image(ctx, nkimg);
		if(nk_button_label(ctx, "Start")){
			nk_end(ctx);
			return 1;
		}

		if(nk_button_label(ctx, "Quit"))
			exits(nil);
	}
	nk_end(ctx);

	return 0;
}

List*
inventorymenu(struct nk_context *ctx, List *l)
{
	List *start, *cur;

	cur = start = l;
	if(nk_begin_titled(ctx, "Inventory", "Lair", nk_rect(0, 0, Dx(splash->r),Dy(screen->r) / 4), 
		NK_WINDOW_BORDER)){
		nk_layout_row_dynamic(ctx, 50, 1);
		for(cur = cur->next; cur != start; cur = cur->next){
			if(cur->datum != nil)
				if(nk_button_label(ctx, ((ItemLex*) cur->datum)->name)){
					nk_end(ctx);
					return cur;
				}
		}
	}
	nk_end(ctx);
	return nil;
}

int
drawnukmenu(struct nk_context *ctx, Event *e, int flag)
{
	nk_input_begin(ctx);
	switch(flag){
	case Emouse:
		nk_plan9_handle_mouse(ctx, e->mouse, screen->r.min);
		break;

	case Ekeyboard:
		/* Broken: Blame event(2)
		nk_plan9_handle_kbd(ctx, (char*)e->data, e->n);
		*/
		if(e->kbdc == Kdel){
			exits(nil);
		}
		break;
	}
	nk_input_end(ctx);

	/* Right way to do things, nuklear expects raw /dev/kdb reads
	if(nk_input_is_key_down(&ctx->input, NK_KEY_DEL))
			exits(nil);

	*/

	draw(screen, screen->r, black, nil, ZP);
	if(mainmenu(ctx) != 0)
		return 1;

	nk_plan9_render(ctx, screen);
	flushimage(display, 1);
	return 0;
}

/* TODO: less copy paste */
int
drawitemmenu(struct nk_context *ctx, Event *e, int flag, int menu)
{
	List *l = nil;

	/* TODO: this is bad */
	extern void redrawfloor(void);
	
	nk_input_begin(ctx);
	switch(flag){
	case Emouse:
		nk_plan9_handle_mouse(ctx, e->mouse, screen->r.min);
		break;


	case Ekeyboard:
		/* Broken: Blame event(2)
		nk_plan9_handle_kbd(ctx, (char*)e->data, e->n);
		*/
		if(e->kbdc == Kdel || e->kbdc == Kesc){
			nk_input_end(ctx);
			redrawfloor();
			return 1;
		}
		break;
	}
	nk_input_end(ctx);

	/* Right way to do things, nuklear expects raw /dev/kdb reads
	if(nk_input_is_key_down(&ctx->input, NK_KEY_DEL))
			exits(nil);

	*/

	switch(menu){
	case ItemMenuInv:
		l = inventorymenu(ctx, curfloor->player->inventory);
		break;
	case ItemMenuEquip:
		l = inventorymenu(ctx, curfloor->player->equipment);
		break;
	case ItemMenuWear:
		l = inventorymenu(ctx, curfloor->player->inventory);
		if(l != nil){
			equipitem(curfloor, l);
		}
		break;
	case ItemMenuDrop:
		l = inventorymenu(ctx, curfloor->player->inventory);
		if(l != nil){
			dropitem(curfloor, l);
			deletelistitem(l);
		}
		break;
	case ItemMenuRemove:
		l = inventorymenu(ctx, curfloor->player->equipment);
		if(l != nil){
			appendlist(curfloor->player->inventory, l->datum);
			deletelistitem(l);
		}
		break;
	case ItemMenuDel:
		l = inventorymenu(ctx, curfloor->player->inventory);
		if(l != nil)
			deletelistitem(l);
		break;
	case ItemMenuInspect:
		l = inventorymenu(ctx, curfloor->player->inventory);
		if(l != nil){
			draw(screen, Rpt(screen->r.min, Pt(screen->r.max.x, screen->r.min.y + 20)), white, nil, ZP);
			string(screen, screen->r.min, black, screen->r.min, font, ((ItemLex*)l->datum)->desc);
			/* Display the desc until next keyboard event */
			while(event(e) != Ekeyboard)
				;
		}
		break;
	}

	nk_plan9_render(ctx, screen);
	flushimage(display, 1);
	if(l != nil){
		redrawfloor();
		return 1;
	}
	return 0;
}

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
		tmp = subpt(f->player->pos, f->creeps[i]->pos);

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
