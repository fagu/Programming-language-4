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
%token IF ELSE WHILE FOR INT NEW DEF
%type <expression> expression
%type <type> type
%type <typelist> typelist netypelist
%type <arglist> arglist nearglist
%type <explist> explist neexplist
%left ';'
%left IF WHILE DEF
%right '='
%left '+' '-'
%left '*' '/' '%'
%left '(' '['
%union {
	string *name;
	int num;
	Expression *expression;
	Type *type;
	vector<Type*> *typelist;
	vector<Argument*> *arglist;
	vector<Expression*> *explist;
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
	| DEF type ':' '(' arglist ')' '(' expression ')' {
	$$ = new FunctionExpression($2,*$5,$8);
}
	| expression '(' explist ')' {
	$$ = new CallExpression($1,*$3);
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
	| type '(' typelist ')' {
	$$ = new FunctionType($1,*$3);
}
	| '(' type ')' {
	$$ = $2;
}

typelist:
	{
	$$ = new vector<Type*>;
}
	| netypelist {
	$$ = $1;
}

netypelist:
	  type {
	$$ = new vector<Type*>;
	$$->push_back($1);
}
	| netypelist ',' type {
	$$ = $1;
	$$->push_back($3);
}

arglist:
	{
	$$ = new vector<Argument*>;
}
	| nearglist {
	$$ = $1;
}


nearglist:
	  type IDENTIFIER {
	$$ = new vector<Argument*>;
	$$->push_back(new Argument($1,*$2));
}
	| nearglist ',' type IDENTIFIER {
	$$ = $1;
	$$->push_back(new Argument($3,*$4));
}

explist:
	{
	$$ = new vector<Expression*>;
}
	| neexplist {
	$$ = $1;
}

neexplist:
	  expression {
	$$ = new vector<Expression*>;
	$$->push_back($1);
}
	| neexplist ',' expression {
	$$ = $1;
	$$->push_back($3);
}

%%

void yyerror (char const *s) {
	//fprintf (stderr, "%s\n", s);
}
