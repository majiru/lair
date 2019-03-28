#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <heap.h>

#include "lair.h"

char *buttons[] = {"exit", 0};
Menu menu = {buttons};

int inmenu;

Floor *curfloor;

uchar curdepth;

Image *black;
Image *white;

int cheatDefog;
int cheatTeleport;

void
eresized(int isnew)
{
	if(isnew && getwindow(display, Refnone) < 0)
		sysfatal("Can't reattach to window");
	resizefloor(curfloor);

	//Create room cordinates
	if(curfloor->nrooms == 0)
		initfloor(curfloor);
	else
		/*BUG: This causes resize to 'move' the objectives */
		initmap(curfloor);

	discover(curfloor);
	drawfloor(curfloor);
	redrawcreep(curfloor);
	drawtile(curfloor, curfloor->playpos, TPlayer);
	resetcur(curfloor);
}

void
handleaction(Rune rune)
{
	static Point dst = {0, 0};
	Point menudst;

	if(cheatTeleport == 0)
		dst = curfloor->playpos;

	switch(rune){
	/* Quit the game */
	case Kbs:
	case Kdel:
	case 'Q':
		exits(nil);

	/* Menu/Debug keys, does not count as player turn */
	case Kesc:
		inmenu = 0;
	/* fallthrough */
	case 's':
		goto draw;

	case 'D':
		drawpath(curfloor);
		return;

	case 'H':
		drawhardness(curfloor);
		return;

	case 'T':
		drawpathtunnel(curfloor);
		return;

	case 'm':
		monstermenu(curfloor, nil);
		inmenu = 1;
		return;

	case 'f':
		cheatDefog = !cheatDefog;
		goto draw;

	case 't':
		cheatTeleport = !cheatTeleport;
		/* Landing from teleport, update position */
		if(cheatTeleport == 0){
			curfloor->playpos = dst;
			goto draw;
		}
		break;

	/* Movement/Action keys */
	case '<':
		if(curfloor->map[PCINDEX(curfloor)].type == TPortalD)
			goto draw;
		return;

	case '>':
		if(curfloor->map[PCINDEX(curfloor)].type == TPortalU)
			goto draw;
		return;

	case Khome:
	case '7':
	case 'y':
		dst = subpt(dst, Pt(1, 1));
		break;

	case Kup:
	case '8':
	case 'k':
		if(inmenu == 1){
			menudst = Pt(0, -1);
			monstermenu(curfloor, &menudst);
			return;
		}else
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
		if(inmenu == 1){
			menudst = Pt(0, 1);
			monstermenu(curfloor, &menudst);
			return;
		}else
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
	if(cheatTeleport != 0){
		drawstringtile(curfloor, dst, "*");
		return;
	}
	if(moveentity(curfloor, curfloor->playpos, dst, TPlayer, 0)){
			curfloor->playpos = dst;
	}
	tickcreep(curfloor);

draw:
	discover(curfloor);
	drawfloor(curfloor);
	redrawcreep(curfloor);
	drawtile(curfloor, curfloor->playpos, TPlayer);

}

void
usage(void)
{
	fprint(2, "Usage: %s [--save] [--load]\n", argv0);
	exits("usage");
}

void
p9main(int argc, char *argv[])
{
	Event ev;
	int e, i;
	int saveflg, loadflg;

	curdepth = 0;
	cheatDefog = cheatTeleport = 0;

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

	black = allocimage(display, Rect(0, 0, 1, 1), screen->chan, 1, DBlack);
	white = allocimage(display, Rect(0, 0, 1, 1), screen->chan, 1, DWhite);

	srand(time(0));

	curfloor = newfloor();
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
