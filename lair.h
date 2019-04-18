/* Graphical Options */
#define TILESIZE 16

#define PALETTENUM 4

#define ROOMNUMMAX 20
#define ROOMMINX 4
#define ROOMMINY 3
#define ROOMMAXX 15
#define ROOMMAXY 15

#define PORTALMAX 4

#define ITEMMAX 10

#define CREEPMAX 100
#define NUMCREEP 25

#define MAXLEXICON 32

#define VIEWDIST 4

#ifdef __cplusplus
extern "C" void p9main(int,char**);
#endif

/* Utility macros */
#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define RRANGE(min, max) rand() % ((max) - (min)) + (min)
#define MAPINDEX(f, x, y) ((x) * f->rows + (y))
#define MAPINDEXPT(f, p) ((p.x) * f->rows + (p.y))
#define PCINDEX(f) (f->player->pos.x * f->rows + f->player->pos.y)

enum {
	ItemMenuWear = 1,
	ItemMenuRemove,
	ItemMenuDrop,
	ItemMenuDel,
	ItemMenuInv,
	ItemMenuEquip,
	ItemMenuInspect,
};

/* Tile options */
enum {
	TEmpty,
	TRoom,
	TTunnel,
	TPlayer,
	TPortalD,
	TPortalU,

	TGENERICMAX
};

/* Creep Tile Options */
enum {
	TCreep = TGENERICMAX,
	TCreepA,
	TCreepB,
	TCreepC,
	TCreepD,
	TCreepE,
	TCreepF,
	TCreepG,
	TCreepH,
	TCreepI,
	TCreepJ,
	TCreepK,
	TCreepL,
	TCreepM,
	TCreepN,
	TCreepO,
	TCreepP,
	TCreepQ,

	TCREEPMAX

};

/* Item Options */
enum {
	IArmor = TCREEPMAX,
	ICloak,
	IRing,
	IAmulet,
	IBoots,
	IHelmet,
	IGloves,
	ILight,
	IWeapon,
	IOffhand,

	IMAX
};

/* Special Tiles that do not have a place in the tilesheet */
enum {
	THidden = 250
};

/* TODO: Move this into it's own thing */
typedef struct List List;

struct
List {
	List *next, *prev;
	void *datum;
};

/* Creep ability options */
typedef
enum CreepAbil{
	CIntel		= 	1,
	CTele 		= 	2,
	CTunnel		=	4,
	CErratic 	=	8,
	CUniq		=   16,
	CPass		=	32,
	CPickup		=	64,
	CDestroy	=	128,
	CBoss		=	256,
} CreepAbil;

typedef
struct Dice {
	int base;
	int ndie;
	int nside;

	/* Stores last role, useful for one time calc */
	int last; 
} Dice;

typedef
struct CreepLex {
	char *name;
	char *desc;
	char *color;
	uchar tile;
	Dice *speed;
	Dice *dam;
	Dice *HP;
	CreepAbil type;
	int rarity;
	int spawned;
} CreepLex;

typedef
struct ItemLex {
	char *name;
	char *desc;
	char *color;
	uchar type;
	Dice *weight;
	Dice *dam;
	Dice *hit;
	Dice *attr;
	Dice *val;
	Dice *dodge;
	Dice *def;
	Dice *speed;
	int rarity;
	char art;
} ItemLex;


/* TODO: Maybe refactor into union */
typedef
struct Creep {
	CreepLex	*info;
	List		*equipment;
	List		*inventory;
	Point 		pos;
	int 		health;
} Creep;

typedef
struct Item {
	ItemLex *info;
	Point	pos;
	int		pickedup;
} Item;


typedef
struct Portal {
	uchar tile;
	Point pos;
} Portal;

typedef
struct Tile {
	uchar type;
	uchar hardness;
	uchar seen;
	int pcdistance;
	int tunneldistance;
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
	int			nitem;
	Item		*items[ITEMMAX];
	Tile		*map;
	Portal		stairs[PORTALMAX];
	Image 		*tilesheet;
	Creep		*player;	/* Mankind truly is the worst enemy of them all */
} Floor;

typedef
struct Path{
	HeapNode* n;
	Point pos, from;
	int cost;
} Path;

/* Declared in lair.c */
extern Floor *curfloor;

extern uchar curdepth;

extern Image *black;
extern Image *white;

extern int cheatDefog;
extern int cheatTeleport;


/* Declared in monster.y */
extern CreepLex *creeplexicon[MAXLEXICON];
extern ItemLex *itemlexicon[MAXLEXICON];
extern int ncreeplex;
extern int nitemlex;


/* floor.c */
Floor*	newfloor(void);
void	nextfloor(Floor**);
void	freefloor(Floor*, int);
void	initmap(Floor *f);
void	initfloor(Floor*);
void	drawtotile(Floor*, Point, uchar);
void	drawtofloor(Floor *, Rectangle, uchar);
void	drawtile(Floor*, Point, uchar);
Point	randempty(Floor*);
Point	spawnentity(Floor*, uchar);
int		moveentity(Floor*, Point, int, Creep*);
void	drawfloor(Floor*);
int		additem(Floor*, Point, uchar);
void	inititems(Floor*);
int		addroom(Floor*, Rectangle);
void	initrooms(Floor*);
void	resizefloor(Floor*);
void	path(Floor*, Rectangle, Rectangle);
void	assignhardness(Floor*);
void	drawhardness(Floor*);
void	discover(Floor*);
void 	drawstringtile(Floor*, Point, char*);
void	redrawitem(Floor*);
uchar	isonstair(Floor*);
Item*	isonitem(Floor*);

/* path.c */
void	djikstra(Floor*);
void	djikstratunnel(Floor*);
void	drawpath(Floor*);
void	drawpathtunnel(Floor*);

/* creep.c */
int		isoccupied(Floor*, Point, Creep*);
void	spawncreep(Floor*);
void	redrawcreep(Floor*);
void	tickcreep(Floor*);
Creep*	point2creep(Floor*, Point);

/* utility.c */
int		overlaps(Rectangle, Rectangle);
int		within(Rectangle, Point);
int		isbigendian(void);
void	roledie(Dice*);
Dice*	str2dice(char*);

/* menu.c */
void	monstermenu(Floor*, Point*);
void	resetcur(void);
int		drawnukmenu(struct nk_context*, Event*, int);
int		drawitemmenu(struct nk_context*, Event*, int, int);

/* fmt.c */
#pragma varargck type "D" Dice*
#pragma varargck type "U" CreepAbil
#pragma varargck type "C" CreepLex*
void	lairfmtinstall(void);

/* item.c */
List* 	createlist(void);
void	appendlist(List*, void*);
void	deletelistitem(List*);
void	dropitem(Floor*, List*);
void	equipitem(Floor*, List*);
