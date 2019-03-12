/* Graphical Options */
#define TILESIZE 16
#define TILEBORDER 1

#define PALETTENUM 3

#define ROOMNUMMAX 20
#define ROOMMINX 4
#define ROOMMINY 3
#define ROOMMAXX 15
#define ROOMMAXY 15

#define PORTALMAX 4

#define ITEMMAX 10

#define CREEPMAX 100
#define NUMCREEP 10

/* Utility macros */
#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define RRANGE(min, max) rand() % ((max) - (min)) + (min)
#define MAPINDEX(f, x, y) (x) * f->rows + (y)
#define MAPINDEXPT(f, p) (p.x) * f->rows + (p.y)
#define PCINDEX(f) f->playpos.x * f->rows + f->playpos.y

/* Tile options */
enum {
	TEmpty,
	TRoom,
	TTunnel,
	TPlayer,
	TPortalD,
	TPortalU,
	TCreep,
	TCreepM,
	TCreepB,
	TCreepE,

	TNUM
};

/* Creep options */
enum {
	CIntel = 	1,
	CTele = 	2,
	CTunnel	=	4,
	CErratic =	8,
};

typedef
struct Creep {
	uchar type;
	uchar tile;
	Point pos;
} Creep;

typedef
struct Tile {
	uchar type;
	uchar hardness;
	int pcdistance;
	uchar itemID;
} Tile;

/* Floor represents a single 'level' of the map */
typedef
struct Floor
{
	int 		rows, cols;
	int			nrooms;
	Rectangle 	rooms[ROOMNUMMAX];
	int			ncreep;
	Creep		*creeps[CREEPMAX];
	Tile		*map;
	Point		playpos;
	Image 		*tilesheet;
} Floor;

typedef
struct Path{
	HeapNode* n;
	Point pos, from;
	int cost;
} Path;

Floor *curfloor;

uchar curdepth = 0;

/* floor.c */
Floor*	newfloor(void);
void	nextfloor(Floor**);
void	freefloor(Floor*, int);
void	initfloor(Floor*);
void	drawtotile(Floor*, Point, uchar);
void	drawtofloor(Floor *, Rectangle, uchar);
void	drawtile(Floor*, Point, uchar);
Point	randempty(Floor*);
Point	spawnentity(Floor*, uchar);
int		moveentity(Floor*, Point, Point, uchar, int);
void	drawfloor(Floor*);
int		additem(Floor*, Point, uchar);
void	inititems(Floor*);
int		addroom(Floor*, Rectangle);
void	initrooms(Floor*);
void	resizefloor(Floor*);
void	path(Floor*, Rectangle, Rectangle);
void	assignhardness(Floor*);
void	drawhardness(Floor*);


/* path.c */
void	djikstra(Floor*);
void	drawpath(Floor*);

/* creep.c */
void	spawncreep(Floor*);
void	redrawcreep(Floor*);
void	tickcreep(Floor*);

/* utility.c */
int				overlaps(Rectangle, Rectangle);
int				isbigendian(void);