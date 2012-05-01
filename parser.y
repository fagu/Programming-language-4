%code requires {
	#include <math.h>
	#include <iostream>
	#include <string>
	#include "../location.h"
	#include "../expression.h"
	#include "../type.h"
	#define YYLTYPE Location
	using namespace std;
	int yylex (void);
	void yyerror (char const *);
}
%token <name> IDENTIFIER
%token <num> NUMBER
%token IF ELSE WHILE FOR INT NEW
%type <expression> expression
%type <type> type
%left ';'
%left IF WHILE
%right '='
%left '+' '-'
%left '*' '/' '%'
%left '(' '['
%union {
	string *name;
	int num;
	Expression *expression;
	Type *type;
}

%%

input:
	  expression {
	handleStatement($1);
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
	| expression ';' expression {
	$$ = new BinaryExpression(';',$1,$3);
}
	| '(' expression ')' {
	$$ = $2;
}
	| expression '=' expression {
	$$ = $1->setExpression($3);
}
	| WHILE '(' expression ')' '(' expression ')' {
	$$ = new WhileExpression($3,$6);
}
	| IF '(' expression ')' '(' expression ')' {
	$$ = new IfExpression($3,$6);
}
	| IF '(' expression ')' '(' expression ')' ELSE '(' expression ')' {
	$$ = new IfExpression($3,$6,$10);
}
	| type IDENTIFIER '(' expression ')' {
	$$ = new VariableDeclarationExpression($1,*$2,$4);
}
	| NEW type '[' expression ']' {
	$$ = new ArrayExpression($2,$4);
}
	| expression '[' expression ']' {
	$$ = new ArrayAccessExpression($1,$3);
}
	| error {
	printsyntaxerr(@$, "Syntax error!\n");
}

type:
	  INT {
	$$ = new IntegerType();
}
	| type '*' {
	$$ = new ArrayType($1);
}
	| '(' type ')' {
	$$ = $2;
}

%%

void yyerror (char const *s) {
	//fprintf (stderr, "%s\n", s);
}
