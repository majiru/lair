#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <heap.h>

#include "lair.h"

char *buttons[] = {"exit", 0};
Menu menu = {buttons};

void
eresized(int new)
{
	int i;
	int init = (curfloor->nrooms == 0);

	if(new && getwindow(display, Refnone) < 0)
		sysfatal("Can't reattach to window");
	resizefloor(curfloor);

	//print("Cols %d\Rows %d\nSize %d\n", curfloor->cols, curfloor->rows, curfloor->rows * curfloor->cols);

	//Create room cordinates
	if(init)
		initrooms(curfloor);

	//Draw paths between rooms to floor map
	for(i = 0; i < curfloor->nrooms - 1; i++)
		path(curfloor, curfloor->rooms[i], curfloor->rooms[i+1]);

	//Draw rooms to floor map
	for(i = 0; i < curfloor->nrooms; i++)
		drawtofloor(curfloor, curfloor->rooms[i], TRoom);
	
	//Draw player, items, portals to floor map
	if(init){
		inititems(curfloor);
		curfloor->playpos = spawnentity(curfloor, TPlayer);
	}

	//Assign hardness of tiles based on type
	assignhardness(curfloor);

	drawfloor(curfloor);

	drawtile(curfloor, curfloor->playpos, TPlayer);

	djikstra(curfloor);
	spawncreep(curfloor);
}

void
handleaction(Rune rune)
{
	Point dst = curfloor->playpos;

	switch(rune){
	/* Quit the game */
	case Kbs:
	case Kdel:
	case 'Q':
		exits(nil);

	/* Menu/Debug keys, does not count as player turn */
	case 's':
		drawfloor(curfloor);
		redrawcreep(curfloor);
		drawtile(curfloor, curfloor->playpos, TPlayer);
		return;

	case 'D':
		drawpath(curfloor);
		return;

	case 'H':
		drawhardness(curfloor);
		return;

	/* Movement/Action keys */
	case Khome:
	case '7':
	case 'y':
		dst = subpt(dst, Pt(1, 1));
		break;

	case Kup:
	case '8':
	case 'k':
		dst = subpt(dst, Pt(0, 1));
		break;

	case Kpgup:
	case '9':
	case 'u':
		dst = Pt(dst.x + 1, dst.y - 1);
		break;

	case Kdown:
	case '2':
	case 'j':
		dst = addpt(dst, Pt(0, 1));
		break;

	case Kleft:
	case '4':
	case 'h':
		dst = subpt(dst, Pt(1,0));
		break;

	case Kright:
	case '6':
	case 'l':
		dst = addpt(dst, Pt(1,0));
		break;

	case Kend:
	case '1':
	case 'b':
		dst = Pt(dst.x - 1, dst.y + 1);
		break;

	case Kpgdown:
	case '3':
	case 'n':
		dst = addpt(dst, Pt(1, 1));
		break;

	case '.':
	case ' ':
	case '5':
		break;

	default:
		return;
	}
	if(moveentity(curfloor, curfloor->playpos, dst, TPlayer, 0)){
			djikstra(curfloor);
			curfloor->playpos = dst;
	}
	tickcreep(curfloor);
}

void
usage(void)
{
	fprint(2, "Usage: %s [--save] [--load]\n", argv0);
	exits("usage");
}

void
main(int argc, char *argv[])
{
	Event ev;
	int e, i;
	int saveflg, loadflg;

	saveflg = loadflg = 0;

	for(i = 1; i < argc; ++i)
		if(strlen(argv[i]) != 6)
			usage();
		else
			switch(argv[i][2]){
			case 's':
				saveflg++;
				break;
			case 'l':
				loadflg++;
				break;
			}

	if(initdraw(nil, nil, "lair") < 0)
		sysfatal("lair: Failed to init screen %r");

	srand(time(0));

	curfloor = loadtilemap("tiles.img", "walls.img");
	einit(Emouse | Ekeyboard);

	eresized(0);

	for(;;){
		e = event(&ev);

		if((e == Emouse) && 
			(ev.mouse.buttons & 4) &&
			(emenuhit(3, &ev.mouse, &menu) == 0)) exits(nil);
		if(e == Ekeyboard){
			handleaction(ev.kbdc);
		}
	}
}