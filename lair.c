#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
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
	if(init)
		initrooms(curfloor);

	for(i = 0; i < curfloor->nrooms - 1; i++)
		path(curfloor, curfloor->rooms[i], curfloor->rooms[i+1]);

	for(i = 0; i < curfloor->nrooms; i++)
		drawtofloor(curfloor, curfloor->rooms[i], TRoom);
	
	if(init)
		inititems(curfloor);

	drawfloor(curfloor);
}

void
main(int argc, char *argv[])
{
	USED(argc);
	USED(argv);
	Event ev;
	int e;

	if(initdraw(nil, nil, "rlg327") < 0)
		sysfatal("rlg327: Failed to init screen %r");

	srand(time(0));

	curfloor = loadtilemap("tiles.img", "walls.img");
	einit(Emouse);

	eresized(0);

	for(;;){
		e = event(&ev);

		if((e == Emouse) && 
			(ev.mouse.buttons & 4) &&
			(emenuhit(3, &ev.mouse, &menu) == 0)) exits(nil);
	}
}