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
program
	: class_decl
	;

class_decl
	: CLASS id '{' field_decls method_decls '}'
	;

field_decls
	:
	| field_decls field_decl
	;

field_decl
	: type ids ';'
	;

ids
	: ids ',' id
	| id
	| ids ',' id '[' int_literal ']'
	| id '[' int_literal ']'
	;

method_decls
	:
	| method_decls method_decl
	;

method_decl
	: method_type id '(' params ')' body
	;

method_type
	: type
	| VOID
	;

params
	:
	| param_decls
	;

param_decls
	: param_decls ',' type id
	| type id
	;

body
	: block
	| EXTERN ';'
	;

block
	: '{' field_decls statements '}'
	;

type
	: INT | FLOAT | BOOLEAN | id
	;

statements
	: statements statement
	| statement
	;

statement
	: location assign_op expr ';'
	| method_call ';'
	| IF '(' expr ')' statement else_stmt
	| FOR id '=' expr ',' expr statement
	| WHILE expr statement
	| RETURN expr_stmt ';'
	| BREAK ';'
	| CONTINUE ';'
	| ';'
	| block
	;

else_stmt
	:
	| ELSE statement
	;

assign_op
	: '=' | PLUS_ASSIGN | MINUS_ASSIGN
	;

method_call
	: id ids_reference '(' expr_params ')'
	;

location
	: id ids_reference
	| id ids_reference '[' expr ']'
	;

expr
	: location
	| method_call
	| literal
	| expr bin_op expr
	| '-' expr
	| '!' expr
	| '(' expr ')'
	;

expr_params
	: expr_params ',' expr
	| expr
	;

expr_stmt
	:
	| expr
	;

bin_op
	: arith_op
	| rel_op
	| eq_op
	| cond_op
	;

arith_op
	: '+' | '-' | '*' | '/' | '%'
	;

rel_op
	: '<' | '>' | LESS_EQUAL | GREAT_EQUAL
	;

eq_op
	: EQUAL | DISTINCT
	;

cond_op
	: AND | OR
	;

ids_reference
	:
	| '.' id
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