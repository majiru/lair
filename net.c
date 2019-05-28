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

/* Required because of proccreate argument funciton signature */
typedef
struct Arg{
	Channel *c;
	int fd;
	char ldir[40];
} Arg;

void
conhandler(void *arg)
{
	char buf;
	int dfd;
	Arg *a = (Arg*)arg;

	dfd = accept(a->fd, a->ldir);
	if(dfd < 0){
		fprint(2, "Error opening data fd: %r\n");
		threadexits(nil);
	}
	while(read(dfd, &buf, 1) == 1)
		switch(buf){
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case '>':
		case '<':
			send(a->c, &buf);
		}

	free(arg);
	close(dfd);
	close(a->fd);
	threadexits(nil);
}

void
conmanager(void *arg)
{
	int acfd;
	char  adir[40];

	Channel *c = arg;

	threadsetgrp(5);

	acfd = announce("tcp!*!327", adir);
	if(acfd < 0){
		fprint(2, "Could not announce: %r\n");
		threadexits(nil);
	}

	for(;;){
		Arg *a = mallocz(sizeof(Arg), 1);
		a->c = c;
		a->fd = listen(adir, a->ldir);
		if(a->fd < 0){
			fprint(2, "Could not listen: %r\n");
			threadexits(nil);
		}
		proccreate(conhandler, a, 8192);
	}
}

Channel*
coninit(int buffsize)
{
	Channel *c = chancreate(sizeof(char), buffsize);
	proccreate(conmanager, c, 8192);
	return c;
}

void
condestroy(Channel *c)
{
	chanfree(c);
	threadkillgrp(5);
}
