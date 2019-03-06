/* Graphical Options */
#define TILESIZE 16
#define TILEBORDER 1

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
	TPortal,
	TTunnel,
	TPlayer,
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
	char type;
	char tile;
	Point pos;
} Creep;

typedef
struct Tile {
	char type;
	char hardness;
	int pcdistance;
	union {
		char portaldest;
		char itemID;
	};
} Tile;

/* Floor represents a single 'level' of the map */
typedef
struct Floor
{
	int 		rows, cols;
	int			nrooms;
	Rectangle 	rooms[ROOMNUMMAX];
	Tile		*map;
	Image 		**colorset;
	Point		*tileorigin;
	Point		playpos;
	Creep		*creeps[CREEPMAX];
	int			ncreep;
} Floor;

typedef
struct Path{
	HeapNode* n;
	Point pos, from;
	int cost;
} Path;

Floor *curfloor;

/* floor.c */
void	drawtotile(Floor *f, Point p, char tile);
void	drawtofloor(Floor *f, Rectangle r, char tile);
void	drawtile(Floor *f, Point p, char tile);
Point	randempty(Floor *f);
Point	spawnentity(Floor *f, char tile);
int		moveentity(Floor *f, Point src, Point dest, char tile, int canmove);
void	drawfloor(Floor *f);
int		additem(Floor *f, Point p, char tile);
void	inititems(Floor *f);
int		addroom(Floor *f, Rectangle r);
void	initrooms(Floor *f);
void	resizefloor(Floor *f);
void	path(Floor *f, Rectangle r1, Rectangle r2);
void	assignhardness(Floor *f);

/* tile.c */
Floor* loadtilemap(char *file, char *wal);

/* path.c */
void	djikstra(Floor*);
void	drawpath(Floor*);

/* creep.c */
void	spawncreep(Floor*);
void	tickcreep(Floor*);

/* utility.c */
int				overlaps(Rectangle r1, Rectangle r2);
int				isbigendian(void);