#include <u.h>
#include <libc.h>
#include <draw.h>

int
overlaps(Rectangle r1, Rectangle r2)
{
	return 	r1.min.x < r2.max.x && r1.max.x > r2.min.x &&
    		r1.min.y < r2.max.y && r1.max.y > r2.min.y;
}

static Image*
openimage(char *file)
{
	int fd;
	Image *i;

	if((fd = open(file, OREAD)) < 0)
		return nil;

	i = readimage(display, fd, 0);
	close(fd);
	return i;
}