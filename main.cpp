#include <iostream>
#include "build/bison_parser.h"

int yyparse();

int main(int argc, char **argv) {
	yyparse();
	return 0;
}
