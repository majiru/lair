%{
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <draw.h>
#include <heap.h>

#include "lair.h"

int fail;
extern int yylineno;
extern int yylex(void);
CreepLex *cur;

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

void
addlexelem(CreepLex *l, char *key, void *val)
{
	if(strcmp(key, "NAME") == 0)
		l->name = (char*)val;

	else if(strcmp(key, "SYMB") == 0)
		l->tile = ((char*)val)[0];

	else if(strcmp(key, "COLOR") == 0)
		l->color = (char*)val;

	else if(strcmp(key, "DESC\n") == 0)
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

}

%}

%union {
	int num;
	char *str;
	Dice *d;
}

%token TokBegin TokEnd TokLBr

%token <num> 	TokInt
%token <str> 	TokStr TokKey TokLine
%token <d>		TokDice

%type <str> text

%%
file:
	| file TokBegin	{ cur = mallocz(sizeof(CreepLex), 1); }
	| file expr ';'
	| file TokEnd  { print("%C\n", cur); }

expr:
	| TokKey text 		{ addlexelem(cur, $1, $2); }
	| TokKey TokInt		{ addlexelem(cur, $1, &$2); }
	| TokKey TokDice	{ addlexelem(cur, $1, $2); }

text:
	TokStr			{ $$ = $1; }
	| text TokStr 	{ $$ = strcat(strcat($1, " "), $2); }
	| text TokLBr	{ $$ = strcat($1, "\n"); }

%%

void
yyerror(char *s)
{
	error("line %uld: %s", yylineno, s);
}