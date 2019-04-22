%{
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <memdraw.h>
#include <nuklear.h>
#include <heap.h>

#include "lair.h"
#include "y.tab.h"
%}

%Start thing desc

%a 10000

%%
RLG327\ (MONSTER|OBJECT)\ DESCRIPTION\ 1		/* Ignore version info */;
^BEGIN\ (MONSTER|OBJECT)\n						{ BEGIN(thing); yylval.str = strdup(yytext); return TokBegin; }
^END											{ BEGIN(INITIAL); return TokEnd;}
^DESC\n											{ BEGIN(desc); yylval.str = strdup(yytext); return TokKey; }
<desc>^"."										{ BEGIN(thing); return ';';}
<desc>\n										return TokLBr;
<desc>^.+										{ yylval.str = strdup(yytext); return TokStr; }
^[A-Z]+											{ yylval.str = strdup(yytext); return TokKey; }
[0-9]+"+"[0-9]+d[0-9]+							{ yylval.d = str2dice(strdup(yytext)); return TokDice; }
<thing>[0-9]+									{ yylval.num = atoi(yytext); return TokInt; }
[A-Za-z0-9"()"\+]+								{ yylval.str = strdup(yytext); return TokStr; }
<thing>\n										return ';';
\n												/* Ignore newlines outside of blocks */;
[ \t]+											/* nop */;
%%

int
yywrap(void)
{
	return 1;
}
