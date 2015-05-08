%{
#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

void yyerror(const char *s);
%}

// Bison fundamentally works by asking flex to get the next token, which it
// returns as an object of type "yystype".  But tokens could be of any
// arbitrary data type!  So we deal with that in Bison by defining a C union
// holding each of the types of tokens that Flex could return, and have Bison
// use that union instead of "int" for the definition of "yystype":
%union {
	int ival;
	float fval;
	char *sval;
}

// define the "terminal symbol" token types I'm going to use (in CAPS
// by convention), and associate each with a field of the union:
%token <ival> INT
%token <fval> FLOAT
%token <sval> STRING

%%
// this is the actual grammar that bison will parse, but for right now it's just
// something silly to echo to the screen what bison gets from flex.  We'll
// make a real one shortly:
snazzle:
	INT snazzle      { cout << "bison found an int: " << $1 << endl; }
	| FLOAT snazzle  { cout << "bison found a float: " << $1 << endl; }
	| STRING snazzle { cout << "bison found a string: " << $1 << endl; }
	| INT            { cout << "bison found an int: " << $1 << endl; }
	| FLOAT          { cout << "bison found a float: " << $1 << endl; }
	| STRING         { cout << "bison found a string: " << $1 << endl; }
	;
%%

int main(int argc, char **argv) {
	// Check if the file name was given
	if (argc != 2) {
		cout << "Usage: ./parser file" << endl;
		exit(EXIT_FAILURE);
	}

	// open a file handle to a particular file:
	FILE *file = fopen(argv[1], "r");

	// make sure it is valid:
	if (!file) {
		cout << "I can't open a.snazzle.file!" << endl;
		return -1;
	}

	// set flex to read from it instead of defaulting to STDIN:
	yyin = file;

	// parse through the input until there is no more:
	do {
		yyparse();
	} while (!feof(yyin));
}

void yyerror(const char *s) {
	cout << "Parse error!  Message: " << s << endl;

	// might as well halt now:
	exit(EXIT_FAILURE);
}