%code requires {
	#include <math.h>
	#include <iostream>
	#include <string>
	#include "../location.h"
	#include "../expression.h"
	#define YYLTYPE Location
	using namespace std;
	int yylex (void);
	void yyerror (char const *);
}
%token <name> IDENTIFIER
%token <num> NUMBER
%token IF ELSE WHILE FOR
%type <expression> expression
%left '+' '-'
%left '*' '/' '%'
%left '('
%union {
	string *name;
	int num;
	Expression *expression;
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
	  expression ';' {
	handleStatement($1);
}
	| error {
	printsyntaxerr(@$, "Syntax error!\n");
}

expression:
	  NUMBER {
	$$ = new NumberExpression($1);
}
	| IDENTIFIER {
	$$ = new VariableExpression(*$1);
}
	| expression '+' expression {
	$$ = new BinaryExpression('+',$1,$3);
}
	| expression '-' expression {
	$$ = new BinaryExpression('-',$1,$3);
}
	| expression '*' expression {
	$$ = new BinaryExpression('*',$1,$3);
}
	| expression '/' expression {
	$$ = new BinaryExpression('/',$1,$3);
}
	| expression '%' expression {
	$$ = new BinaryExpression('%',$1,$3);
}
	| '(' expression ')' {
	$$ = $2;
}
	| error {
	printsyntaxerr(@$, "Syntax error!\n");
}

%%

void yyerror (char const *s) {
	//fprintf (stderr, "%s\n", s);
}
