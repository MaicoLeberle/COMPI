#include <iostream>
#include <iomanip>
#include "node2.h"

extern node_program* ast;
extern int yyparse();
extern FILE *yyin;

int main(int argc, const char* argv[]) {
	// Check if the file name was given
	if (argc != 2) {
		std::cout << "Usage: ./compi file" << std::endl;
		exit(EXIT_FAILURE);
	}

	// open a file handle to a particular file:
	FILE *file = fopen(argv[1], "r");

	// make sure it is valid:
	if (!file) {
		std::cout << "Invalid file!" << std::endl;
		return -1;
	}

	// set flex to read from it instead of defaulting to STDIN:
	yyin = file;

	// parse through the input until there is no more:
	do {
		yyparse();
	} while (!feof(yyin));
}