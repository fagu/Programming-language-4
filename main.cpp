#include <iostream>
#include "build/bison_parser.h"
#include "expression.h"

int yyparse();

int main(int argc, char **argv) {
	init();
	yyparse();
	finalize();
	return 0;
}
