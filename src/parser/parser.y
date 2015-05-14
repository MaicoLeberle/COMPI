%{
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "../node2.h"

extern int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

void yyerror(const char *s);

node_program* ast; // Pointer to the AST
%}

%union {
    node_program *program;
    node_class_decl *class_decl;
    std::vector<node_class_block*> *class_block;
    node_field_decl *field_decl;
    node_method_decl *method_decl;
    std::vector<node_typed_identifier*> *params;
    node_ids *ids;
    node_body *body;
    std::vector<node_statement*> *block;
    node_statement *statement;
    std::vector<std::string*> *ids_reference;
    std::vector<node_expr*> *expr_params;
    node_literal *literal;
    node_expr *expr;
    std::vector<node_identifier*> ids_reference;
    bool l_bool;
	int l_int;
	float l_float;
    std::string *type;
	std::string *l_str;
	std::string *id;
    int token; // Type of token identifier
}

%token <l_bool> L_BOOL
%token <l_int> L_INT
%token <l_float> L_FLOAT
%token <l_str> L_STR
%token <id> ID

%token <token> CLASS VOID EXTERN INT FLOAT BOOLEAN
%token <token> IF ELSE FOR WHILE RETURN BREAK CONTINUE
%token <token> PLUS_ASSIGN MINUS_ASSIGN LESS_EQUAL GREAT_EQUAL
%token <token> EQUAL DISTINCT AND OR

// TODO: Declarar tipos de los no terminales
// TODO: Generar acciones
// TODO: Terminan union
%type <program> program
%type <class_decl> class_decl
%type <class_block> class_block
%type <field_decl> field_decl
%type <method_decl> method_decl
%type <ids> ids
%type <type> type method_type 
%type <token> assign_op
%type <params> params
%type <body> body
%type <block> block statements
%type <statement> statement
%type <ids_reference> ids_reference
%type <expr_params> expr_params
%type <expr> expr location method_call literal
%type <token> bin_op arith_op rel_op eq_op cond_op

%%
program
	: program class_decl    {$1->classes.push_back($2);
                             $$ = $1;}
	| class_decl            {$$ = new node_program(new std::vector<node_class_decl*>($1));
                             ast = $$;}
	;

class_decl
	: CLASS ID '{' class_block '}' {$$ = new node_class_decl(*$2, $4);}
	;

class_block
	:                              {$$ = new std::vector<node_class_block*>();}
	| class_block field_decl       {$1.push_back($2); $$ = $1;}
	| class_block method_decl      {$1.push_back($2); $$ = $1;}
	;

field_decl
	: type ids ';'                 {$$ = new node_field_decl($1, $2);}
	;

ids
	: ids ',' ID                   {$1->ids.push_back($3); $$ = $1;}
	| ID                           {$$ = new node_ids($1);}
	| ids ',' ID '[' L_INT ']'     {$1->ids.push_back($3); $1->index = $3; $$ = $1;}
	| ID '[' L_INT ']'             {$$ = new node_ids($1, $3);}
	;

method_decl
	: method_type ID '(' params ')' body    {$$ = new node_method_decl($1, *$2, $6);}
    | method_type ID '(' ')' body           {$$ = new node_method_decl($1, NULL, $5);}
	;

method_type
	: type
	| VOID                                  {$$ = new std::string("void");}
	;

type
	: INT                                   {$$ = new std::string("int");}
    | FLOAT                                 {$$ = new std::string("float");}
    | BOOLEAN                               {$$ = new std::string("boolean");}
    | ID                                    {$$ = $<type>1}
	;

params
	: params ',' type ID            {$1->push_back(new node_typed_identifier($3, $4));
                                     $$ = $1;}
	| type ID                       {$$ = new std::vector<node_typed_identifier*> 
                                          (new node_typed_identifier($1, $2));}
	;

body
	: block                         {$$ = new node_body($1);}
	| EXTERN ';'                    {$$ = new node_body();}
	;

block
	: '{' statements '}'            {$$ = $2;}
	;

statements
	:                               {$$ = new std::vector<node_statement*>();}
	| statements statement          {$1->push_back($2); $$ = $1;}
	| statement                     {$$ = new std::vector<node_statement*>($1);}
	;

statement
	: field_decl                                 {$$ = new node_statement();//TODO: y aca?
                                                 }
	| location assign_op expr ';'                {$$ = new node_assignment_statement($1, $2, $3);}
	| method_call ';'                            {$$ = $1; //TODO: y aca?
                                                    }
	| IF '(' expr ')' statement                  {$$ = new node_if_statement($3, $5);}
    | IF '(' expr ')' statement ELSE statement   {$$ = new node_if_statement($3, $5, $7);}
	| FOR ID '=' expr ',' expr statement         {$$ = new node_for_statement($2, $4, $6, $7);}
	| WHILE expr statement                       {$$ = new node_while_statement($2, $3);}
	| RETURN expr ';'                            {$$ = new node_return_statement($2);}
    | RETURN ';'                            {$$ = new node_return_statement();}
	| BREAK ';'                                  {$$ = new node_break_statement();}
	| CONTINUE ';'                               {$$ = new node_continue_statement();}
	| ';'                                        {$$ = new node_skip_statement();}
	| block                                      {$$ = $1; //TODO: y aca?
                                                 }
	;

assign_op
	: '='                                   {$$ = $<token>1;}
    | PLUS_ASSIGN 
    | MINUS_ASSIGN
	;

method_call
	: ID ids_reference '(' expr_params ')'  {$2->insert($2->begin(), $1);
                                             $$ = new node_method_call($2, $4);}
	| ID ids_reference '(' ')'              {$2->insert($2->begin(), $1);
                                             $$ = new node_method_call($2);}
	;

location
	: ID ids_reference                      {$2->insert($2->begin(), $1);
                                             $$ = new node_location($2);}
	| ID ids_reference '[' expr ']'         {$2->insert($2->begin(), $1);
                                             $$ = new node_location($2, $4);}
	;

ids_reference
	:                                       {$$ = new std::vector<std::string*>();}
	| ids_reference '.' ID                  {$1->push_back($3); $$ = $1;}
	| '.' ID                                {$$ = new std::vector<std::string*>($2);}
	;

expr
	: location
	| method_call
	| literal
	| expr bin_op expr                      {$$ = new node_binary_operation_expr($1, $2, $3);}
	| '-' expr                              {$$ = new node_minus_expr($2);}
	| '!' expr                              {$$ = new node_negate_expr($2);}
	| '(' expr ')'                          {$$ = new node_parenthesis_expr($2);}
	;

expr_params
	: expr_params ',' expr                  {$1->push_back($3); $$ = $1;}
	| expr                                  {$$ = new std::vector<node_expr*> ($1);}
	;

bin_op
	: arith_op          
	| rel_op
	| eq_op
	| cond_op
	;

arith_op
	: '+'                       {$$ = $<token>1;}
    | '-'                       {$$ = $<token>1;}
    | '*'                       {$$ = $<token>1;}
    | '/'                       {$$ = $<token>1;}
    | '%'                       {$$ = $<token>1;}
	;

rel_op      
	: '<'                       {$$ = $<token>1;}
    | '>'                       {$$ = $<token>1;}
    | LESS_EQUAL                {$$ = $<token>1;}
    | GREAT_EQUAL               {$$ = $<token>1;}
	;

eq_op
	: EQUAL                     {$$ = $<token>1;}
    | DISTINCT                  {$$ = $<token>1;}
	;

cond_op
	: AND                       {$$ = $<token>1;}
    | OR                        {$$ = $<token>1;}
	;

literal
	: L_INT                     {$$ = $<token>1;}
	| L_FLOAT                   {$$ = $<token>1;}
	| L_BOOL                    {$$ = $<token>1;}
	| L_STR                     {$$ = $<token>1;}
	;

%%

int main(int argc, char **argv) {
	// Check if the file name was given
	if (argc != 2) {
		std::cout << "Usage: ./parser file" << std::endl;
		exit(EXIT_FAILURE);
	}

	// open a file handle to a particular file:
	FILE *file = fopen(argv[1], "r");

	// make sure it is valid:
	if (!file) {
		std::cout << "I can't open a.snazzle.file!" << std::endl;
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
	std::cout << "Parse error!  Message: " << s << std::endl;

	// might as well halt now:
	exit(EXIT_FAILURE);
}
