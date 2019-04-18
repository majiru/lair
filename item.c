#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <heap.h>
#include <memdraw.h>
#include <nuklear.h>

#include "lair.h"

List*
createlist(void)
{
	List *l = mallocz(sizeof(List), 1);

	l->next = l;
	l->prev = l;
	return l;
}

/* 
* Root needs to be the first item in the list, 
* bad design but keeps things simple
*/
void
appendlist(List *root, void *data)
{
	List *new;

	new = mallocz(sizeof(List), 1);
	new->datum = data;

	if(root->next == nil){
		assert(root->prev == nil);
		root->next = root;
		root->prev = root;
		root->datum = data;
		return;
	}

	assert(root->prev != nil);

	root->prev->next = new;
	new->prev = root->prev;
	new->next = root;
	root->prev = new;
}

void
deletelistitem(List *item)
{
	item->datum = nil;
	if(item->next == item){
		assert(item->prev == item);
		item->next = nil;
		item->prev = nil;
		return;
	}

	item->prev->next = item->next;
	item->next->prev = item->prev;

	free(item);
}

void
deletelistval(List *node, void *val)
{
	List *start;

	if(node->datum == val){
		deletelistitem(node);
		return;
	}

	start = node;
	for(node = node->next; node != start; node = node->next)
		if(node->datum == val)
			deletelistitem(node);
}

void
dropitem(Floor *f, List *item)
{
	int i;
	for(i = 0; i < ITEMMAX; i++)
		if(f->items[i]->info == item->datum && f->items[i]->pickedup == 1){
			f->items[i]->pos = f->player->pos;
			f->items[i]->pickedup = 0;
			return;
		}
}

void
equipitem(Floor *f, List *item)
{
	List *start, *cur;
	List *toswap = nil;
	char itemtype = ((ItemLex*)item->datum)->type;
	int isequiped = 0;

	start = cur = f->player->equipment;

	for(cur = cur->next; cur != start; cur = cur->next)
		if(cur->datum != nil)
			if(itemtype == ((ItemLex*)cur->datum)->type){
				isequiped++;
				toswap = cur;
			}

	if((itemtype == IRing && isequiped < 2) || (itemtype != IRing && isequiped == 0)){
		appendlist(f->player->equipment, item->datum);
		deletelistitem(item);
		return;
	}

	assert(toswap == nil);
	/* Player has item already equiped, we must swap them */
	appendlist(f->player->inventory, toswap->datum);
	deletelistitem(toswap);
	appendlist(f->player->equipment, item->datum);
	deletelistitem(item);
}
