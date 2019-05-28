#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <keyboard.h>
#include <cursor.h>
#include <mouse.h>
#include <heap.h>
#include <memdraw.h>
#include <nuklear.h>

#include "lair.h"

char *buttons[] = {"exit", 0};
Menu menu = {buttons};

enum{MOUSE, RESIZE, KEYBD, NET, NONE};

mainstacksize = 32 * 1024;

Mousectl *mctl;
Keyboardctl *kctl;
struct nk_context *ctx;

/* Flag for user control to tell us if we are in the monster menu */
int inmenu;
int inmainmenu;
int initemmenu;

/* Our current floor */
Floor *curfloor;

/* How many floors has the user gone? */
uchar curdepth;

/* Used for drawing text */
Image *black;
Image *white;

int inspection;
int cheatDefog;
int cheatTeleport;

void
quit(void){
	killdjikstra();
	closedisplay(display);
	closemouse(mctl);
	closekeyboard(kctl);
	threadexitsall(nil);
}

void
redrawfloor(void)
{
	drawfloor(curfloor);
	redrawcreep(curfloor);
	redrawitem(curfloor);
	drawtile(curfloor, curfloor->player->pos, TPlayer);
	flushimage(display, Refnone);
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
	redrawfloor();
	resetcur();
}

void
handleaction(Rune rune)
{
	static Point dst = {0, 0};
	Point menudst;

	Item *topickup;
	Creep *toinspect;

	Portal *p;

	if(cheatTeleport == 0 && inspection == 0)
		dst = curfloor->player->pos;

	switch(rune){
	/* Quit the game */
	case Kbs:
	case Kdel:
	case 'Q':
		quit();

	/* Menu/Debug keys, does not count as player turn */
	case Kesc:
		inmenu = 0;
		cheatTeleport = 0;
		inspection = 0;
	/* fallthrough */
	case 's':
		resettileskip(curfloor);
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

	case 'i':
		drawitemmenu(ctx, mctl->c, kctl->c, ItemMenuInv);
		goto draw;

	case 'w':
		drawitemmenu(ctx, mctl->c, kctl->c, ItemMenuWear);
		goto draw;

	case 't':
		drawitemmenu(ctx, mctl->c, kctl->c, ItemMenuRemove);
		goto draw;

	case 'd':
		drawitemmenu(ctx, mctl->c, kctl->c, ItemMenuDrop);
		goto draw;

	case 'x':
		drawitemmenu(ctx, mctl->c, kctl->c, ItemMenuDel);
		goto draw;

	case 'e':
		drawitemmenu(ctx, mctl->c, kctl->c, ItemMenuEquip);
		goto draw;

	case 'I':
		drawitemmenu(ctx, mctl->c, kctl->c, ItemMenuInspect);
		goto draw;

	case 'f':
		resettileskip(curfloor);
		cheatDefog = !cheatDefog;
		goto draw;

	case 'P':
		cheatTeleport = !cheatTeleport;
		/* Landing from teleport, update position */
		if(cheatTeleport == 0){
			curfloor->player->pos = dst;
			resettileskip(curfloor);
			goto draw;
		}
		break;

	case 'L':
		inspection = !inspection;
		/* Viewing selected monster */
		if(inspection == 0 && !eqpt(dst, curfloor->player->pos)){
			int count = 1;
			char *tmp, *buf, *cursor;

			resettileskip(curfloor);
			toinspect = point2creep(curfloor, dst);
			if(toinspect == nil)
				goto draw;
			cursor = buf = strdup(toinspect->info->desc);
			draw(screen, Rpt(screen->r.min, Pt(screen->r.max.x, screen->r.min.y + 200)), white, nil, ZP);
			string(screen, screen->r.min, black, screen->r.min, font, toinspect->info->name);
			while((tmp = strchr(cursor, '\n')) != nil){
				*tmp = '\0';
				string(screen, addpt(screen->r.min, Pt(0, 20*count)), black, addpt(screen->r.min, Pt(0, 20*count)), font, cursor);
				cursor = tmp+1;
				count++;
			}
			flushimage(display, Refnone);
			recv(kctl->c, nil);
			free(buf);
			goto draw;
		}
		goto draw;

	/* Movement/Action keys */
	case '<':
		p = isonstair(curfloor);
		if(p != nil && p->tile == TPortalD){
			nextfloor(&curfloor, p);
			goto draw;
		}
		return;

	case '>':
		p = isonstair(curfloor);
		if(p != nil && p->tile == TPortalU){
			nextfloor(&curfloor, p);
			goto draw;
		}
		return;

	case ',':
		topickup = isonitem(curfloor);
		if(topickup != nil){
			topickup->pickedup = 1;
			appendlist(curfloor->player->inventory, topickup->info);
		}
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
		tickcreep(curfloor);
		goto draw;

	default:
		return;
	}
	if(cheatTeleport != 0 || inspection != 0){
		drawstringtile(curfloor, dst, "*");
		flushimage(display, Refnone);
		return;
	}
	if(moveentity(curfloor, dst, 0, curfloor->player)){
			curfloor->player->pos = dst;
	}
	tickcreep(curfloor);

draw:
	discover(curfloor);
	redrawfloor();
	return;
}

void
usage(void)
{
	fprint(2, "Usage: %s [--save] [--load]\n", argv0);
	exits("usage");
}

void
threadmain(int argc, char *argv[])
{
	int i;
	int saveflg, loadflg;
	struct nk_context sctx;
	struct nk_user_font nkfont;
	Mouse mouse;
	Rune kbd;
	char netin;
	Channel *netc;
	int resize[2];

	if(initdraw(nil, nil, "lair") < 0)
		sysfatal("lair: Failed to init screen %r");

	if((mctl = initmouse(nil, screen)) == nil)
		sysfatal("%s: %r", argv0);
	if((kctl = initkeyboard(nil)) == nil)
		sysfatal("%s: %r", argv0);

	netc = coninit(10);

	Alt alts[] = {
		{mctl->c, &mouse, CHANRCV},
		{mctl->resizec, &resize, CHANRCV},
		{kctl->c, &kbd, CHANRCV},
		{netc, &netin, CHANRCV},
		{nil, nil, CHANEND},
	};

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

	nk_plan9_makefont(&nkfont, font);
	if(!nk_init_default(&sctx, &nkfont))
		sysfatal("nk_init: %r");

	ctx = &sctx;

	black = allocimage(display, Rect(0, 0, 1, 1), screen->chan, 1, DBlack);
	white = allocimage(display, Rect(0, 0, 1, 1), screen->chan, 1, DWhite);

	srand(time(0));

	drawnukmenu(ctx, mctl->c, kctl->c);
	inmainmenu = 0;

	curfloor = newfloor();

	eresized(0);

	startdjikstra();

	for(;;){
		switch(alt(alts)){
			case KEYBD:
				handleaction(kbd);
				break;
			case RESIZE:
				eresized(1);
				break;
			case NET:
				handleaction(netin);
				break;
		}
	}
}
