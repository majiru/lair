#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <memdraw.h>
#include <mouse.h>
#include <nuklear.h>
#include <heap.h>

#include "lair.h"

Channel *normaldjikstra[2] = {nil, nil};
Channel *tunneldjikstra[2] = {nil, nil};

int workers[2] = {0, 0};

/* Note: use of curfloor is a hack */
static int
playdistcmp(const void *key, const void *with) {
	return curfloor->map[MAPINDEX(curfloor, ((Path*)key)->pos.x, ((Path*)key)->pos.y)].pcdistance -
		curfloor->map[MAPINDEX(curfloor, ((Path*)with)->pos.x, ((Path*)with)->pos.y)].pcdistance;
}

static int
tunneldistcmp(const void *key, const void *with) {
	return curfloor->map[MAPINDEX(curfloor, ((Path*)key)->pos.x, ((Path*)key)->pos.y)].tunneldistance -
		curfloor->map[MAPINDEX(curfloor, ((Path*)with)->pos.x, ((Path*)with)->pos.y)].tunneldistance;
}

void
djikstra(void*)
{
	int i, j;
	Heap h;

	Path *path, *p;

	Floor *f;
	Channel *in = normaldjikstra[0];
	Channel *out = normaldjikstra[1];
	for(;;){
	recv(in, nil);
	f = curfloor;

	path = mallocz(sizeof(Path) * f->cols * f->rows, 1);
	if(path == nil)
		sysfatal("%s: Could not alloc Path", argv0);

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++)
			path[MAPINDEX(f, i, j)] = (Path){nil, Pt(i, j), ZP, 99999};

	heap_init(&h, playdistcmp, nil);

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++)
			f->map[MAPINDEX(f, i, j)].pcdistance = 99999;

	f->map[PCINDEX(f)].pcdistance = 0;

	for(i = 1; i < f->cols - 1; i++)
		for(j = 1; j < f->rows - 1; j++)
			path[MAPINDEX(f, i, j)].n = heap_insert(&h, &path[MAPINDEX(f, i, j)]);

	while((p = heap_remove_min(&h)) != nil){
		p->n = nil;

		j = MAPINDEX(f, p->pos.x, p->pos.y);

		i = MAPINDEX(f, p->pos.x - 1, p->pos.y);
		if(path[i].n != nil && f->map[i].pcdistance > f->map[j].pcdistance + 1){
			f->map[i].pcdistance = f->map[j].pcdistance + 1;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x + 1, p->pos.y);
		if(path[i].n != nil && f->map[i].pcdistance > f->map[j].pcdistance + 1){
			f->map[i].pcdistance = f->map[j].pcdistance + 1;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x, p->pos.y - 1);
		if(path[i].n != nil && f->map[i].pcdistance > f->map[j].pcdistance + 1){
			f->map[i].pcdistance = f->map[j].pcdistance + 1;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x, p->pos.y + 1);
		if(path[i].n != nil && f->map[i].pcdistance > f->map[j].pcdistance + 1){
			f->map[i].pcdistance = f->map[j].pcdistance + 1;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x + 1, p->pos.y + 1);
		if(path[i].n != nil && f->map[i].pcdistance > f->map[j].pcdistance + 1){
			f->map[i].pcdistance = f->map[j].pcdistance + 1;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x - 1, p->pos.y - 1);
		if(path[i].n != nil && f->map[i].pcdistance > f->map[j].pcdistance + 1){
			f->map[i].pcdistance = f->map[j].pcdistance + 1;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x + 1, p->pos.y - 1);
		if(path[i].n != nil && f->map[i].pcdistance > f->map[j].pcdistance + 1){
			f->map[i].pcdistance = f->map[j].pcdistance + 1;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x - 1, p->pos.y + 1);
		if(path[i].n != nil && f->map[i].pcdistance > f->map[j].pcdistance + 1){
			f->map[i].pcdistance = f->map[j].pcdistance + 1;
			heap_decrease_key_no_replace(&h, path[i].n);
		}
	}
	free(path);
	send(out, nil);
	}
}


void
djikstratunnel(void*)
{
	int i, j;
	Heap h;
	int size;
	int tunnelcost;

	Path *path, *p;

	Floor *f;
	Channel *in = tunneldjikstra[0];
	Channel *out = tunneldjikstra[1];

	for(;;){
	recv(in, nil);
	f = curfloor;

	path = mallocz(sizeof(Path) * f->cols * f->rows, 1);
	if(path == nil)
		sysfatal("%s: Could not alloc Path", argv0);

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++)
			path[MAPINDEX(f, i, j)] = (Path){nil, Pt(i, j), ZP, 99999};

	heap_init(&h, tunneldistcmp, nil);

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++)
			f->map[MAPINDEX(f, i, j)].tunneldistance = 99999;

	f->map[PCINDEX(f)].tunneldistance = 0;

	for(i = 1; i < f->cols - 1; i++)
		for(j = 1; j < f->rows - 1; j++)
			if(f->map[MAPINDEX(f, i, j)].hardness != 255)
				path[MAPINDEX(f, i, j)].n = heap_insert(&h, &path[MAPINDEX(f, i, j)]);

	size = h.size;
	while((p = heap_remove_min(&h)) != nil){
		if(--size != h.size){
			sysfatal("%s: Bad state in tunnel djikstra", argv0);
		}
		p->n = nil;

		j = MAPINDEX(f, p->pos.x, p->pos.y);

		i = MAPINDEX(f, p->pos.x - 1, p->pos.y);
		tunnelcost = (f->map[i].hardness / 85) + 1;
		if(path[i].n != nil && f->map[i].tunneldistance > f->map[j].tunneldistance + tunnelcost){
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x + 1, p->pos.y);
		tunnelcost = (f->map[i].hardness / 85) + 1;
		if(path[i].n != nil && f->map[i].tunneldistance > f->map[j].tunneldistance + tunnelcost){
			f->map[i].tunneldistance = f->map[j].tunneldistance + tunnelcost;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x, p->pos.y - 1);
		tunnelcost = (f->map[i].hardness / 85) + 1;
		if(path[i].n != nil && f->map[i].tunneldistance > f->map[j].tunneldistance + tunnelcost){
			f->map[i].tunneldistance = f->map[j].tunneldistance + tunnelcost;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x, p->pos.y + 1);
		tunnelcost = (f->map[i].hardness / 85) + 1;
		if(path[i].n != nil && f->map[i].tunneldistance > f->map[j].tunneldistance + tunnelcost){
			f->map[i].tunneldistance = f->map[j].tunneldistance + tunnelcost;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x + 1, p->pos.y + 1);
		tunnelcost = (f->map[i].hardness / 85) + 1;
		if(path[i].n != nil && f->map[i].tunneldistance > f->map[j].tunneldistance + tunnelcost){
			f->map[i].tunneldistance = f->map[j].tunneldistance + tunnelcost;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x - 1, p->pos.y - 1);
		tunnelcost = (f->map[i].hardness / 85) + 1;
		if(path[i].n != nil && f->map[i].tunneldistance > f->map[j].tunneldistance + tunnelcost){
			f->map[i].tunneldistance = f->map[j].tunneldistance + tunnelcost;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x + 1, p->pos.y - 1);
		tunnelcost = (f->map[i].hardness / 85) + 1;
		if(path[i].n != nil && f->map[i].tunneldistance > f->map[j].tunneldistance + tunnelcost){
			f->map[i].tunneldistance = f->map[j].tunneldistance + tunnelcost;
			heap_decrease_key_no_replace(&h, path[i].n);
		}

		i = MAPINDEX(f, p->pos.x - 1, p->pos.y + 1);
		tunnelcost = (f->map[i].hardness / 85) + 1;
		if(path[i].n != nil && f->map[i].tunneldistance > f->map[j].tunneldistance + tunnelcost){
			f->map[i].tunneldistance = f->map[j].tunneldistance + tunnelcost;
			heap_decrease_key_no_replace(&h, path[i].n);
		}
	}
	free(path);
	send(out, nil);
	}
}

void
drawpath(Floor *f)
{
	Point min;
	char buf[2];
	int i, j;

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++){
			min.x = i * TILESIZE + screen->r.min.x;
			min.y = j * TILESIZE + screen->r.min.y;
			snprint(buf, 2, "%d", f->map[MAPINDEX(f, i, j)].pcdistance % 10);
			string(screen, min, white, min, font, buf);
		}
}

void
drawpathtunnel(Floor *f)
{
	Point min;
	char buf[2];
	int i, j;

	for(i = 0; i < f->cols; i++)
		for(j = 0; j < f->rows; j++){
			min.x = i * TILESIZE + screen->r.min.x;
			min.y = j * TILESIZE + screen->r.min.y;
			snprint(buf, 2, "%d", f->map[MAPINDEX(f, i, j)].tunneldistance % 10);
			string(screen, min, white, min, font, buf);
		}

}

void
startdjikstra(void)
{
	/* Assert to avoid memory and proc leak */
	assert(normaldjikstra[0] == nil && normaldjikstra[1] == nil);
	assert(workers[0] == 0);
	assert(tunneldjikstra[0] == nil && tunneldjikstra[1] == nil);
	assert(workers[1] == 0);

	normaldjikstra[0] = chancreate(sizeof(char), 0);
	normaldjikstra[1] = chancreate(sizeof(char), 0);
	workers[0] = proccreate(djikstra, nil, 2048);

	tunneldjikstra[0] = chancreate(sizeof(char), 0);
	tunneldjikstra[1] = chancreate(sizeof(char), 0);
	workers[1] = proccreate(djikstratunnel, nil, 2048);
}

void
killdjikstra(void)
{
	if(normaldjikstra[0] != nil)
		chanfree(normaldjikstra[0]);
	if(normaldjikstra[1] != nil)
		chanfree(normaldjikstra[1]);
	if(workers[0] != 0)
		threadkill(workers[0]);

	if(tunneldjikstra[0] != nil)
		chanfree(tunneldjikstra[0]);
	if(tunneldjikstra[1] != nil)
		chanfree(tunneldjikstra[1]);
	if(workers[1] != 0)
		threadkill(workers[1]);
}

void
tickdjikstra(void)
{
	/* Wake up the threads */
	send(normaldjikstra[0], nil);
	send(tunneldjikstra[0], nil);

	/* Wait for threads to finish */
	recv(normaldjikstra[1], nil);
	recv(tunneldjikstra[1], nil);
}
