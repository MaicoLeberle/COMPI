%{
#include <cstdio>
#include <cstdlib>
#include <iostream>

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

void yyerror(const char *s);
%}

%union {
	bool l_bool;
	int l_int;
	float l_float;
	std::string l_str;
	std::string id;
}

%token <l_bool> L_BOOL
%token <l_int> L_INT
%token <l_float> L_FLOAT
%token <l_str> L_STR
%token <id> ID

%token CLASS VOID EXTERN INT FLOAT BOOLEAN
%token IF ELSE FOR WHILE RETURN BREAK CONTINUE
%token PLUS_ASSIGN MINUS_ASSIGN LESS_EQUAL GREAT_EQUAL
%token EQUAL DISTINCT AND OR

// TODO: Declarar tipos de los no terminales
// TODO: Generar acciones
// TODO: Terminan union

%%
program
	: program class_decl
	| class_decl
	;

class_decl
	: CLASS ID '{' class_block '}'
	;

class_block
	:
	| class_block field_decl
	| class_block method_decl
	;

field_decl
	: type ids ';'
	;

ids
	: ids ',' ID
	| ID
	| ids ',' ID '[' INTL ']'
	| ID '[' INTL ']'
	;

method_decl
	: method_type ID '(' params ')' body
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
	: param_decls ',' type ID
	| type ID
	;

body
	: block
	| EXTERN ';'
	;

block
	: '{' statements '}'
	;

type
	: INT | FLOAT | BOOLEAN | ID
	;

statements
	:
	| statements statement
	| statement
	;

statement
	: field_decl
	| location assign_op expr ';'
	| method_call ';'
	| IF '(' expr ')' statement else_stmt
	| FOR ID '=' expr ',' expr statement
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

expr_stmt
	:
	| expr
	;

assign_op
	: '=' | PLUS_ASSIGN | MINUS_ASSIGN
	;

method_call
	: ID ids_reference '(' expr_params ')'
	| ID ids_reference '(' ')'
	;

location
	: ID ids_reference
	| ID ids_reference '[' expr ']'
	;

ids_reference
	:
	| ids_reference '.' ID
	| '.' ID
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

literal
	: L_INT
	| L_FLOAT
	| L_BOOL
	| L_STR
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