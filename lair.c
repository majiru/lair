#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <heap.h>
#include <memdraw.h>
#include <nuklear.h>

#include "lair.h"

char *buttons[] = {"exit", 0};
Menu menu = {buttons};

/* Flag for user control to tell us if we are in the monster menu */
int inmenu;
int inmainmenu;
int initemmenu;

/* Our current floor */
Floor *curfloor;

/* How many floors has the user gone? */
uchar curdepth;

/* BUG: Plan9port doesn't like display->black and display->white */
Image *black;
Image *white;

int inspection;
int cheatDefog;
int cheatTeleport;


void
redrawfloor(void)
{
	drawfloor(curfloor);
	redrawcreep(curfloor);
	redrawitem(curfloor);
	drawtile(curfloor, curfloor->player->pos, TPlayer);
}

void
eresized(int isnew)
{
	if(isnew && getwindow(display, Refnone) < 0)
		sysfatal("Can't reattach to window");

	if(inmainmenu != 0)
		return;

	resizefloor(curfloor);

	//Create room cordinates
	if(curfloor->nrooms == 0)
		initfloor(curfloor);
	else
		initmap(curfloor);

	discover(curfloor);
	drawfloor(curfloor);
	redrawcreep(curfloor);
	redrawitem(curfloor);
	drawtile(curfloor, curfloor->player->pos, TPlayer);
	resetcur();
}

int
handleaction(Rune rune)
{
	static Point dst = {0, 0};
	Point menudst;

	Item *topickup;
	Creep *toinspect;

	if(cheatTeleport == 0 && inspection == 0)
		dst = curfloor->player->pos;

	switch(rune){
	/* Quit the game */
	case Kbs:
	case Kdel:
	case 'Q':
		exits(nil);

	/* Menu/Debug keys, does not count as player turn */
	case Kesc:
		inmenu = 0;
		cheatTeleport = 0;
		inspection = 0;
	/* fallthrough */
	case 's':
		goto draw;

	case 'D':
		drawpath(curfloor);
		return 0;

	case 'H':
		drawhardness(curfloor);
		return 0;

	case 'T':
		drawpathtunnel(curfloor);
		return 0;

	case 'm':
		monstermenu(curfloor, nil);
		inmenu = 1;
		return 0;

	case 'i':
		return ItemMenuInv;

	case 'w':
		return ItemMenuWear;

	case 't':
		return ItemMenuRemove;

	case 'd':
		return ItemMenuDrop;

	case 'x':
		return ItemMenuDel;

	case 'e':
		return ItemMenuEquip;

	case 'I':
		return ItemMenuInspect;

	case 'f':
		cheatDefog = !cheatDefog;
		goto draw;

	case 'P':
		cheatTeleport = !cheatTeleport;
		/* Landing from teleport, update position */
		if(cheatTeleport == 0){
			curfloor->player->pos = dst;
			goto draw;
		}
		break;

	case 'L':
		inspection = !inspection;
		/* Viewing selected monster */
		if(inspection == 0){
			Event e;
			int count = 1;
			char *tmp, *buf, *cursor;

			toinspect = point2creep(curfloor, dst);
			if(toinspect == nil)
				break;
			cursor = buf = strdup(toinspect->info->desc);
			draw(screen, Rpt(screen->r.min, Pt(screen->r.max.x, screen->r.min.y + 200)), white, nil, ZP);
			string(screen, screen->r.min, black, screen->r.min, font, toinspect->info->name);
			while((tmp = strchr(cursor, '\n')) != nil){
				*tmp = '\0';
				string(screen, addpt(screen->r.min, Pt(0, 20*count)), black, addpt(screen->r.min, Pt(0, 20*count)), font, cursor);
				cursor = tmp+1;
				count++;
			}
			/* Display the desc until next keyboard event */
			while(event(&e) != Ekeyboard)
				;
			free(buf);
			goto draw;
		}
		return 0;

	/* Movement/Action keys */
	case '<':
		if(isonstair(curfloor) == TPortalD){
			nextfloor(&curfloor);
			goto draw;
		}
		return 0;

	case '>':
		if(isonstair(curfloor) == TPortalU){
			nextfloor(&curfloor);
			goto draw;
		}
		return 0;

	case ',':
		topickup = isonitem(curfloor);
		if(topickup != nil){
			topickup->pickedup = 1;
			appendlist(curfloor->player->inventory, topickup->info);
		}
		return 0;

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
			return 0;
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
			return 0;
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
		return 0;
	}
	if(cheatTeleport != 0 || inspection != 0){
		drawstringtile(curfloor, dst, "*");
		return 0;
	}
	if(moveentity(curfloor, dst, 0, curfloor->player)){
			curfloor->player->pos = dst;
	}
	tickcreep(curfloor);

draw:
	discover(curfloor);
	redrawfloor();
	return 0;
}

void
usage(void)
{
	fprint(2, "Usage: %s [--save] [--load]\n", argv0);
	exits("usage");
}

#ifdef __cplusplus
void
p9main(int argc, char *argv[])
#else
void
main(int argc, char *argv[])
#endif
{
	Event ev;
	int e, i;
	int saveflg, loadflg;
	struct nk_context sctx;
	struct nk_user_font nkfont;
	int curitemmenu = 0;

	inmainmenu = 1;
	initemmenu = 0;
	curdepth = 0;
	inspection = 0;
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

	lairfmtinstall();

	extern void yyparse(void);

	extern int infd;
	extern int outfd;
	outfd = open("/dev/null", OREAD|OWRITE);
	infd = open("./monster.txt", OREAD);
	yyparse();
	close(infd);

	infd = open("./object.txt", OREAD);
	yyparse();
	close(infd);
	close(outfd);

	if(initdraw(nil, nil, "lair") < 0)
		sysfatal("lair: Failed to init screen %r");

	nk_plan9_makefont(&nkfont, font);
	if(!nk_init_default(&sctx, &nkfont))
		sysfatal("nk_init: %r");

	black = allocimage(display, Rect(0, 0, 1, 1), screen->chan, 1, DBlack);
	white = allocimage(display, Rect(0, 0, 1, 1), screen->chan, 1, DWhite);

	srand(time(0));

	curfloor = newfloor();
	einit(Ekeyboard | Emouse);

	eresized(0);

	for(;;){
		e = event(&ev);

skipinput:
		if(inmainmenu != 0){
			inmainmenu = !(drawnukmenu(&sctx, &ev, e));
			if(inmainmenu == 1)
				continue;
			else
				eresized(0);
		}


		if(initemmenu != 0){
			initemmenu = !(drawitemmenu(&sctx, &ev, e, curitemmenu));
			continue;
		}

		if((e == Emouse) && 
			(ev.mouse.buttons & 4) &&
			(emenuhit(3, &ev.mouse, &menu) == 0)) exits(nil);
		if(e == Ekeyboard){
			curitemmenu = handleaction(ev.kbdc);
			if(curitemmenu != 0){
				initemmenu = 1;
				goto skipinput;
			}
		}
	}
}
