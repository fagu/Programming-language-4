%{
#include <stdio.h>
#include <math.h>
#include <string>
#include "bison_parser.h"
# define YY_USER_ACTION yylloc.first_line = yylloc.last_line; yylloc.first_column = yylloc.last_column; yylloc.last_column += yyleng;
# define YY_USER_INIT yylloc.first_line = 1; yylloc.first_column = 1; yylloc.last_line = 1; yylloc.last_column = 1;
%}

DIGIT    [0-9]
ID       [a-zA-Z_][a-zA-Z0-9_]*

%x ST_COMMENT

%%

	int commentdepth;
	string *str;
	int fl, fc;

"if" {return IF;}
"else" {return ELSE;}
"while" {return WHILE;}
"for" {return FOR;}
"int" {return INT;}
"new" {return NEW;}
"def" {return DEF;}
"snew" {return SNEW;}

{DIGIT}+ {
	yylval.num = atoi(yytext);
	return NUMBER;
}

{ID} {
	yylval.name = new string(yytext);
	return IDENTIFIER;
}

"//".*"\n" {
	yylloc.last_line++;
	yylloc.last_column = 1;
}

"/*" {
	commentdepth = 1;
	BEGIN(ST_COMMENT);
}

<ST_COMMENT>{
"/*" {
	commentdepth++;
}
"*/" {
	commentdepth--;
	if (commentdepth == 0)
		BEGIN(0);
}
.
\n+ {
	yylloc.last_line += yyleng;
	yylloc.last_column = 1;
}
}

[ \t]+

[\n]+ {
	yylloc.last_line += yyleng;
	yylloc.last_column = 1;
}

. {return yytext[0];}

%%
