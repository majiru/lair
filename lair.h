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

/* Utility macros */
#define MAX(a, b) a > b ? a : b
#define MIN(a, b) a < b ? a : b
#define RRANGE(min, max) rand() % (max - min) + min;

/* Tile options */
enum {
	TEmpty,
	TRoom,
	TPortal,
	TTunnel,
	TPlayer,

	TNUM
};

typedef
struct Tile {
	char type;
	char hardness;
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
} Floor;

Floor *curfloor;

/* floor.c */
void	drawtotile(Floor *f, Point p, char tile);
void	drawtofloor(Floor *f, Rectangle r, char tile);
void	drawtile(Floor *f, Point p, char tile);
Point	randempty(Floor *f);
Point	spawnentity(Floor *f, char tile);
int		moveentity(Floor *f, Point src, Point dest, char tile);
void	drawfloor(Floor *f);
int		additem(Floor *f, Point p, char tile);
void	inititems(Floor *f);
int		addroom(Floor *f, Rectangle r);
void	initrooms(Floor *f);
void	resizefloor(Floor *f);
void	path(Floor *f, Rectangle r1, Rectangle r2);

/* tile.c */
Floor* loadtilemap(char *file, char *wal);

/* utility.c */
int				overlaps(Rectangle r1, Rectangle r2);
int				isbigendian(void);