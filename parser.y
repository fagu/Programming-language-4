%code requires {
	#include <math.h>
	#include <stdio.h>
	#include <string>
	#include "../location.h"
	#define YYLTYPE Location
	using namespace std;
	int yylex (void);
	void yyerror (char const *);
}
%token <name> IDENTIFIER
%token <num> NUMBER
%token IF ELSE WHILE FOR
%left LAND LOR
%left '!'
%left EQ LE GE '<' '>' NE
%left '+' '-'
%left '*' '/' '%'
%left NEG
%left '.'
%left '(' '[' '{'
%union {
	string *name;
	int num;
}

%%

input: outerstatements {
}

outerstatements:
	    {
}
	| outerstatements outerstatement {
}

outerstatement:
	  error {
	printsyntaxerr(@$, "Syntax error!\n");
}

%%

void yyerror (char const *s) {
	//fprintf (stderr, "%s\n", s);
}
