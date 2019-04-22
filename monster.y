%{
#include <u.h>
#include <libc.h>
#include <thread.h>
#include <bio.h>
#include <draw.h>
#include <mouse.h>
#include <memdraw.h>
#include <nuklear.h>
#include <heap.h>

#include "lair.h"

enum {
	OBJECT,
	MONSTER
};

int fail;
extern int yylineno;
extern int yylex(void);

/* At any given point we are either parsing an object or a monster */
CreepLex *curcreep;
ItemLex *curitem;
char cur;


/* Global lexicons */
CreepLex *creeplexicon[MAXLEXICON];
ItemLex *itemlexicon[MAXLEXICON];

/* Number of parsed elements */
int ncreeplex = 0;
int nitemlex = 0;

void *
error(char *fmt, ...)
{
	char buf[1024];
	va_list arg;

	sprint(buf, "warning: ");
	va_start(arg, fmt);
	vseprint(buf+strlen(buf), buf+sizeof(buf), fmt, arg);
	va_end(arg);
	fprint(2, "%s\n", buf);
	fail++;
	return nil;
}


/* Turn unique color into unique tile */
uchar
color2type(char *str)
{
	if(strstr(str, "YELLOW") != 0)
		return TCreep;

	if(strstr(str, "RED") != 0)
		return TCreepA;

	if(strstr(str, "BLUE") != 0)
		return TCreepB;

	if(strstr(str, "GREEN") != 0)
		return TCreepC;

	if(strstr(str, "BLACK") != 0)
		return TCreepD;

	if(strstr(str, "MAGENTA") != 0)
		return TCreepE;

	if(strstr(str, "CYAN") != 0)
		return TCreepF;

	if(strstr(str, "WHITE") != 0)
		return TCreepG;

	sysfatal("Unknown color: %s", str);
	return 250;
}

CreepAbil
str2abil(char *str)
{
	CreepAbil a = 0;
	char *fields[10];
	int n, i;

	n = getfields(str, fields, 10, 1, " ");

	for(i = 0; i < n; ++i){
		if(strcmp(fields[i], "SMART") == 0)
			a |= CIntel;

		else if(strcmp(fields[i], "TELE") == 0)
			a |= CTele;

		else if(strcmp(fields[i], "ERRATIC") == 0)
			a |= CErratic;

		else if(strcmp(fields[i], "TUNNEL") == 0)
			a |= CTunnel;

		else if(strcmp(fields[i], "UNIQ") == 0)
			a |= CUniq;

		else if(strcmp(fields[i], "PASS") == 0)
			a |= CPass;

		else if(strcmp(fields[i], "PICKUP") == 0)
			a |= CPickup;

		else if(strcmp(fields[i], "DESTROY") == 0)
			a |= CDestroy;

		else if(strcmp(fields[i], "BOSS") == 0)
			a |= CBoss;
		else
			sysfatal("Unknown abil: %s", fields[i]);
	}

	return a;
}

int
str2itemtype(char *str)
{
	if(strcmp(str, "WEAPON") == 0)
		return IWeapon;

	if(strcmp(str, "ARMOR") == 0)
		return IArmor;

	if(strcmp(str, "BOOTS") == 0)
		return IBoots;

	if(strcmp(str, "HELMET") == 0)
		return IHelmet;

	if(strcmp(str, "CLOAK") == 0)
		return ICloak;

	if(strcmp(str, "RING") == 0)
		return IRing;

	if(strcmp(str, "LIGHT") == 0)
		return ILight;

	if(strcmp(str, "OFFHAND") == 0)
		return IOffhand;

	if(strcmp(str, "GLOVES") == 0)
		return IGloves;

	if(strcmp(str, "AMULET") == 0)
		return IAmulet;

	sysfatal("Unknown item type: %s", str);
	return 250;
}

void
additemelem(ItemLex *l, char *key, void *val)
{
	if(strcmp(key, "NAME") == 0)
		l->name = (char*)val;

	else if(strcmp(key, "COLOR") == 0)
		l->color = (char*)val;
	else if(strcmp(key, "DESC\n") == 0)
		l->desc = (char*)val;

	else if(strcmp(key, "SPEED") == 0)
		l->speed = (Dice*)val;

	else if(strcmp(key, "DEF") == 0)
		l->def = (Dice*)val;

	else if(strcmp(key, "DODGE") == 0)
		l->dodge = (Dice*)val;

	else if(strcmp(key, "VAL") == 0)
		l->val = (Dice*)val;

	else if(strcmp(key, "ATTR") == 0)
		l->attr = (Dice*)val;

	else if(strcmp(key, "DAM") == 0)
		l->dam = (Dice*)val;

	else if(strcmp(key, "HIT") == 0)
		l->hit = (Dice*)val;

	else if(strcmp(key, "WEIGHT") == 0)
		l->weight = (Dice*)val;

	else if(strcmp(key, "TYPE") == 0)
		l->type = str2itemtype((char*)val);

	else if(strcmp(key, "RRTY") == 0)
		l->rarity = *((int*)val);

	else if(strcmp(key, "ART") == 0)
		l->art = strstr(val, "FALSE") != 0 ? 0 : 1;

	else
		sysfatal("Unknown token: %s", key);


	free(key);

}

void
addmonsterelem(CreepLex *l, char *key, void *val)
{
	if(strcmp(key, "NAME") == 0)
		l->name = (char*)val;

	else if(strcmp(key, "SYMB") == 0)
		l->tile = ((char*)val)[0];

	else if(strcmp(key, "COLOR") == 0){
		l->color = (char*)val;
		l->tile = color2type(l->color);
	}else if(strcmp(key, "DESC\n") == 0)
		l->desc = (char*)val;

	else if(strcmp(key, "SPEED") == 0)
		l->speed = (Dice*)val;

	else if(strcmp(key, "DAM") == 0)
		l->dam = (Dice*)val;

	else if(strcmp(key, "HP") == 0)
		l->HP = (Dice*)val;

	else if(strcmp(key, "HP") == 0)
		l->HP = (Dice*)val;

	else if(strcmp(key, "ABIL") == 0)
		l->type = str2abil((char*)val);

	else if(strcmp(key, "RRTY") == 0)
		l->rarity = *((int*)val);

	else
		sysfatal("Unknown token: %s", key);

	free(key);
}

%}

%union {
	int num;
	char *str;
	Dice *d;
}

%token TokEnd TokLBr

%token <num> 	TokInt
%token <str> 	TokStr TokKey TokLine TokBegin
%token <d>		TokDice

%type <str> text

%%
file:
	| file TokBegin	{
						if(strstr($2, "MONSTER") != 0){
							cur = MONSTER;
							curcreep = mallocz(sizeof(CreepLex), 1);
						}else if(strstr($2, "OBJECT") != 0){
							cur = OBJECT;
							curitem = mallocz(sizeof(ItemLex), 1);
						}
					}
	| file expr ';'
	| file TokEnd	{
						if(cur == MONSTER)
							creeplexicon[ncreeplex++] = curcreep;
						else if(cur == OBJECT)
							itemlexicon[nitemlex++] = curitem;
					}

expr:
	| TokKey text 		{
							if(cur == MONSTER)
								addmonsterelem(curcreep, $1, $2);
							else if(cur == OBJECT)
								additemelem(curitem, $1, $2);
						}
	| TokKey TokInt		{
							if(cur == MONSTER)
								addmonsterelem(curcreep, $1, &$2);
							else if(cur == OBJECT)
								additemelem(curitem, $1, &$2);
						}
	| TokKey TokDice	{
							if(cur == MONSTER)
								addmonsterelem(curcreep, $1, $2);
							else if(cur == OBJECT)
								additemelem(curitem, $1, $2);
						}

text:
	TokStr			{ $$ = $1; }
	| text TokStr 	{
						char *tmp = mallocz(strlen($1) + strlen($2) + 4, 1);
						tmp = strcat(tmp, $1);
						tmp = strcat(tmp, " ");
						tmp = strcat(tmp, $2);
						free($2);
						$$ = tmp;
					}
	| text TokLBr	{
						char *tmp = mallocz(strlen($1) + 2, 1);
						tmp = strcat(tmp, $1);
						tmp = strcat(tmp, "\n");
						$$ = tmp;
					}

%%

void
yyerror(char *s)
{
	error("line %uld: %s", yylineno, s);
}