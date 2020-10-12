%{
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "../src/node.h"

extern int yylex();
extern FILE *yyin;
extern int line_num;
void yyerror(const char *s);

program_pointer ast; // Pointer to the AST
%}

%union {
    node_program* program;
    node_class_decl* class_decl;
    class_block_list* class_block;
    node_field_decl* field_decl;
    node_method_decl* method_decl;
    method_call* method_call_data;
    parameter_list* params;
    id_list* ids;
    node_body* body;
    node_block* block;
    node_statement* statement;
    statement_list* statements;
    reference_list* ids_reference;
    expression_list* expr_params;
    node_expr* expr;
    node_location* location;
    bool l_bool;
    int l_int;
    float l_float;
    std::string* l_str;
    Type* type;
    AssignOper* assign;
    Oper* oper;
    std::string* id;
    int token; // Type of token identifier
}

%token <l_bool> L_BOOL
%token <l_int> L_INT
%token <l_float> L_FLOAT
%token <l_str> L_STR
%token <id> ID

%token <token> CLASS VOID EXTERN
%token <token> IF ELSE FOR WHILE RETURN BREAK CONTINUE
%token <type> INT FLOAT BOOLEAN STRING
%token <oper> LESS_EQUAL GREATER_EQUAL EQUAL DISTINCT AND OR
%token <assign> '=' PLUS_ASSIGN MINUS_ASSIGN

%type <program> program
%type <class_decl> class_decl
%type <class_block> class_block
%type <field_decl> field_decl
%type <method_decl> method_decl
%type <ids> ids
%type <type> type void
%type <assign> assign_op
%type <params> params
%type <body> body
%type <block> block
%type <statements> statements
%type <statement> statement
%type <ids_reference> ids_reference
%type <expr_params> expr_params
%type <expr> expr literal bin_op
%type <location> location
%type <method_call_data> method_call

%nonassoc IFX
%nonassoc ELSE
%left OR
%left AND
%nonassoc EQUAL DISTINCT
%nonassoc LESS_EQUAL GREATER_EQUAL
%left '<' '>'
%left '+' '-'
%left '*' '/' '%'
%right '!'
%right NEGATIVE

%start program

%%
program
    : program class_decl    {$1->classes.push_back(class_pointer($2));
                              $$ = $1;}
    | class_decl            {$$ = new node_program(class_pointer($1));
                            ast = program_pointer($$);}
    ;

class_decl
    : CLASS ID '{' class_block '}'    {$$ = new node_class_decl(*$2, *$4); delete $2; delete $4;}
    ;

class_block
    :                                 {$$ = new class_block_list();}
    | class_block field_decl          {$1->push_back(field_pointer($2)); $$ = $1;}
    | class_block method_decl         {$1->push_back(method_pointer($2)); $$ = $1;}
    ;

field_decl
    : type ids ';'                    {$$ = new node_field_decl(*$1, *$2); delete $1; delete $2;}
    ;

ids
    : ids ',' ID                       {$1->push_back(id_pointer(new node_id(*$3))); delete $3; $$ = $1;}
    | ID                               {$$ = new id_list();
                                        $$->push_back(id_pointer(new node_id(*$1))); delete $1;}
    | ids ',' ID '[' L_INT ']'         {$1->push_back(id_pointer(new node_id(*$3, $5))); delete $3; $$ = $1;}
    | ID '[' L_INT ']'                 {$$ = new id_list();
                                        $$->push_back(id_pointer(new node_id(*$1, $3))); delete $1;}
    ;

method_decl
    : type ID '(' params ')' body    {$$ = new node_method_decl(*$1, *$2, *$4, body_pointer($6));
                                       delete $1; delete $2; delete $4;}
    | void ID '(' params ')' body    {$$ = new node_method_decl(*$1, *$2, *$4, body_pointer($6));
                                       delete $1; delete $2; delete $4;}
    | type ID '(' ')' body           {$$ = new node_method_decl(*$1, *$2, parameter_list(), body_pointer($5));
                                       delete $1; delete $2;}
    | void ID '(' ')' body           {$$ = new node_method_decl(*$1, *$2, parameter_list(), body_pointer($5));
                                       delete $1; delete $2;}
    ;

void
    : VOID                             {$$ = new Type(Type::TVOID);}
    ;

type
    : INT                              {$$ = new Type(Type::TINTEGER);}
    | FLOAT                            {$$ = new Type(Type::TFLOAT);}
    | BOOLEAN                          {$$ = new Type(Type::TBOOLEAN);}
    | STRING                           {$$ = new Type(Type::TSTRING);}
    | ID                               {$$ = new Type(Type::TID, *$1); delete $1;}
    ;

params
    : params ',' type ID            {$1->push_back(parameter_pointer(new node_parameter_identifier(*$3, *$4)));
                                      $$ = $1; delete $3; delete $4;}
    | type ID                       {$$ = new parameter_list();
                                      $$->push_back(parameter_pointer(new node_parameter_identifier(*$1, *$2)));
                                      delete $1; delete $2;}
    ;

body
    : block                         {$$ = new node_body(block_pointer($1));}
    | EXTERN ';'                    {$$ = new node_body();}
    ;

block
    : '{' statements '}'            {$$ = new node_block(*($2)); delete $2;}
    ;

statements
    :                           {$$ = new statement_list();}
    | statements statement      {$1->push_back(statement_pointer($2)); $$ = $1;}
    ;

statement
    : field_decl                                 {$$ = $1;}
    | location assign_op expr ';'                {$$ = new node_assignment_statement(location_pointer($1),
                                                   *$2, expr_pointer($3)); delete $2;}
    | method_call ';'                            {$$ = new node_method_call_statement($1);}
    | IF '(' expr ')' statement %prec IFX        {$$ = new node_if_statement(expr_pointer($3), statement_pointer($5));}
    | IF '(' expr ')' statement ELSE statement   {$$ = new node_if_statement(expr_pointer($3), statement_pointer($5),
                                                   statement_pointer($7));}
    | FOR ID '=' expr ',' expr statement         {$$ = new node_for_statement(*$2, expr_pointer($4), expr_pointer($6),
                                                   statement_pointer($7)); delete $2;}
    | WHILE expr statement                       {$$ = new node_while_statement(expr_pointer($2), statement_pointer($3));}
    | RETURN expr ';'                            {$$ = new node_return_statement(expr_pointer($2));}
    | RETURN ';'                                 {$$ = new node_return_statement();}
    | BREAK ';'                                  {$$ = new node_break_statement();}
    | CONTINUE ';'                               {$$ = new node_continue_statement();}
    | ';'                                        {$$ = new node_skip_statement();}
    | block                                      {$$ = $1;}
    ;

assign_op
    : '='                                   {$$ = new AssignOper(AssignOper::AASSIGN);}
    | PLUS_ASSIGN                           {$$ = new AssignOper(AssignOper::APLUS_ASSIGN);}
    | MINUS_ASSIGN                          {$$ = new AssignOper(AssignOper::AMINUS_ASSIGN);}
    ;

method_call
    : ID ids_reference '(' expr_params ')'  {$2->insert($2->begin(), *$1); $$ = new method_call(*$2, *$4);
                                              delete $1; delete $2; delete $4;}
    | ID ids_reference '(' ')'              {$2->insert($2->begin(), *$1); $$ = new method_call(*$2);
                                              delete $1; delete $2;}
    ;

location
    : ID ids_reference                      {$2->insert($2->begin(), *$1); $$ = new node_location(*$2);
                                              delete $1; delete $2;}
    | ID ids_reference '[' expr ']'         {$2->insert($2->begin(), *$1); $$ = new node_location(*$2, expr_pointer($4));
                                              delete $1; delete $2;}
    ;

ids_reference
    :                                       {$$ = new reference_list();}
    | ids_reference '.' ID                  {$1->push_back(*$3); $$ = $1; delete $3;}
    ;

expr
    : location                              {$$ = $1;}
    | method_call                           {$$ = new node_method_call_expr($1);}
    | literal                               {$$ = $1;}
    | bin_op                                {$$ = $1;}
    | '-' expr %prec NEGATIVE               {$$ = new node_negative_expr(expr_pointer($2));}
    | '!' expr                              {$$ = new node_negate_expr(expr_pointer($2));}
    | '(' expr ')'                          {$$ = new node_parentheses_expr(expr_pointer($2));}
    ;

expr_params
    : expr_params ',' expr                  {$1->push_back(expr_pointer($3)); $$ = $1;}
    | expr                                  {$$ = new expression_list(); $$->push_back(expr_pointer($1));}
    ;

bin_op
    : expr '+' expr             {$$ = new node_binary_operation_expr(Oper::OPLUS,
                                  expr_pointer($1), expr_pointer($3));}
    | expr '-' expr             {$$ = new node_binary_operation_expr(Oper::OMINUS,
                                  expr_pointer($1), expr_pointer($3));}
    | expr '*' expr             {$$ = new node_binary_operation_expr(Oper::OTIMES, 
                                  expr_pointer($1), expr_pointer($3));}
    | expr '/' expr             {$$ = new node_binary_operation_expr(Oper::ODIVIDE, 
                                  expr_pointer($1), expr_pointer($3));}
    | expr '%' expr             {$$ = new node_binary_operation_expr(Oper::OMOD,
                                  expr_pointer($1), expr_pointer($3));}
    | expr '<' expr             {$$ = new node_binary_operation_expr(Oper::OLESS,
                                  expr_pointer($1), expr_pointer($3));}
    | expr '>' expr             {$$ = new node_binary_operation_expr(Oper::OGREATER,
                                  expr_pointer($1), expr_pointer($3));}
    | expr LESS_EQUAL expr      {$$ = new node_binary_operation_expr(Oper::OLESS_EQUAL,
                                  expr_pointer($1), expr_pointer($3));}
    | expr GREATER_EQUAL expr   {$$ = new node_binary_operation_expr(Oper::OGREATER_EQUAL, 
                                  expr_pointer($1), expr_pointer($3));}
    | expr EQUAL expr           {$$ = new node_binary_operation_expr(Oper::OEQUAL,
                                  expr_pointer($1), expr_pointer($3));}
    | expr DISTINCT expr        {$$ = new node_binary_operation_expr(Oper::ODISTINCT,
                                  expr_pointer($1), expr_pointer($3));}
    | expr AND expr             {$$ = new node_binary_operation_expr(Oper::OAND,
                                  expr_pointer($1), expr_pointer($3));}
    | expr OR expr              {$$ = new node_binary_operation_expr(Oper::OOR,
                                  expr_pointer($1), expr_pointer($3));}
    ;

literal
    : L_INT                     {$$ = new node_int_literal($1);}
    | L_FLOAT                   {$$ = new node_float_literal($1);}
    | L_BOOL                    {$$ = new node_bool_literal($1);}
    | L_STR                     {$$ = new node_string_literal(*$1); delete $1;}
    ;

%%

void yyerror(const char *s) {
    std::cout << "Parse error on line " << line_num << "! Message: " << s << std::endl;

    // might as well halt now:
    exit(EXIT_FAILURE);
}
