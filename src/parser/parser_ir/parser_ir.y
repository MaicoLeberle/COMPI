%{
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "../src/three_address_code.h"

extern int irlex();
extern FILE *irin;
extern int ir_line_num;
void irerror(const char *s);

instructions_list *ir_code; // Pointer to the instructions' list.
%}

%define api.prefix {ir}
 
%union {
    instructions_list* inst_list;
    quad_pointer *instruction;
    address_pointer *arg;
    bool l_bool;
    int l_int;
    float l_float;
    std::string* l_str;
    std::string* id;
    quad_oper relop;
}

%token <l_bool> L_BOOL
%token <l_int> L_INT
%token <l_float> L_FLOAT
%token <l_str> L_STR
%token <id> ID
%token <token> EQUAL DISTINCT LESS LESS_EQUAL GREATER GREATER_EQUAL AND OR NOT
               IF IFTRUE IFFALSE PLUS_ASSIGN MINUS_ASSIGN GOTO CALL ENTER PARAM 
               RETURN

%type <inst_list> inst_list
%type <instruction> instruction binary_assign unary_assign copy jump param call
                    return label_inst enter
%type <l_str> name
%type <arg> address label
%type <relop> relop

/* TODO: 4 shift/reduce conflicts, 27 reduce/reduce conflicts. */

%start inst_list

%%
inst_list
    : inst_list instruction    {$1->push_back(*$2);
                                $$ = $1;
                                ir_code = $$;}
    | instruction              {$$ = new instructions_list();
                                $$->push_back(*$1);
                                ir_code = $$;}
    ;

instruction
    : binary_assign            {$$ = $1;}
    
    | unary_assign             {$$ = $1;}
    
    | copy                     {$$ = $1;}
    
    | jump                     {$$ = $1;}
    
    | param                    {$$ = $1;}
    
    | call                     {$$ = $1;}
    
    | return                    {$$ = $1;}
    
    | label_inst                    {$$ = $1;}
    
    | enter                     {$$ = $1;}
    ;
    
/* TODO: abstraer los operadores en otras producciones, de otro no terminal,
        cuyo tipo sea quad_oper */
/* TODO: src/parser/parser_ir/parser_ir.y: warning: 4 shift/reduce conflicts [-Wconflicts-sr]
 */
binary_assign
    : name '=' address '+' address              {$$ = new quad_pointer(
                                                        new_binary_assign(address_pointer(new_name_address(*$1)), 
                                                                          *$3, 
                                                                          *$5, 
                                                        quad_oper::PLUS));}
    | name '=' address '-' address              {$$ = new quad_pointer(
                                                        new_binary_assign(address_pointer(new_name_address(*$1)), 
                                                        *$3, 
                                                        *$5, 
                                                        quad_oper::MINUS));}
                                                        
    | name '=' address '*' address              {$$ = new quad_pointer(
                                                        new_binary_assign(address_pointer(new_name_address(*$1)), 
                                                        *$3, 
                                                        *$5, 
                                                        quad_oper::TIMES));}
                                                       
    | name '=' address '/' address              {$$ = new quad_pointer(
                                                        new_binary_assign(address_pointer(new_name_address(*$1)), 
                                                        *$3, 
                                                        *$5, 
                                                        quad_oper::DIVIDE));}
                                                        
    | name '=' address '%' address              {$$ = new quad_pointer(
                                                        new_binary_assign(address_pointer(new_name_address(*$1)), 
                                                        *$3, 
                                                        *$5, 
                                                    quad_oper::MOD));}
    ;
    
unary_assign
    : name '=' '-' address              {$$ = new quad_pointer(
                                                new_unary_assign(
                                                address_pointer(new_name_address(*$1)), 
                                                *$4,
                                                quad_oper::NEGATIVE));}

    | name '=' NOT address              {$$ = new quad_pointer(
                                                new_unary_assign(
                                                address_pointer(
                                                new_name_address(*$1)), 
                                                *$4,
                                                quad_oper::NEGATION));}
                                                
    | name '=' '*' address              {$$ = new quad_pointer(
                                                new_unary_assign(
                                                address_pointer(
                                                new_name_address(*$1)), 
                                                *$4,
                                                quad_oper::ADDRESS_OF));}
    ;
    
copy
    : name '=' address              {$$ = new quad_pointer(new_copy(address_pointer(new_name_address(*$1)), *$3));}
    
    | name '[' address ']' '=' address   {$$ = new quad_pointer(
                                            new_indexed_copy_to(address_pointer(new_name_address(*$1)), *$3, *$6));}
    
    | name '=' address '[' address ']'   {$$ = new quad_pointer(
                                            new_indexed_copy_from(address_pointer(new_name_address(*$1)), *$3, *$5));}
    ;
    
jump
    : GOTO label                               {$$ = new quad_pointer(
                                            new_unconditional_jump_inst(*$2));} 
    
    | IFTRUE address GOTO label                {$$ = new quad_pointer(
                                            new_conditional_jump_inst(*$2, 
                                                                    *$4,
                                                                    quad_oper::IFTRUE));}
                                        
    | IFFALSE address GOTO label               {$$ = new quad_pointer(
                                            new_conditional_jump_inst(*$2, 
                                                                    *$4,
                                                                    quad_oper::IFFALSE));}
                                                                    
    | IF address relop address GOTO label      {$$ = new quad_pointer(
                                                    new_relational_jump_inst(*$2, 
                                                                    *$4,
                                                                    $3,
                                                                    *$6));}
                                                                    
param
    : PARAM address                         {$$ = new quad_pointer(
                                            new_parameter_inst(*$2));}
                                            
call
    : CALL label ',' address              {$$ = new quad_pointer(
                                            new_procedure_call_inst(
                                                                    *$2, 
                                                                    *$4));}
                                            
    | name '=' CALL label ',' address  {$$ = new quad_pointer(
                                             new_function_call_inst(address_pointer(new_name_address(*$1)), 
                                                                    *$4, 
                                                                    *$6));}

return
    : RETURN                            {$$ = new quad_pointer(
                                            new_return_inst(nullptr));}
                                            
    | RETURN address                    {$$ = new quad_pointer(
                                            new_return_inst(*$2));}

label_inst
    : label ':'                         {$$ = new quad_pointer(
                                            new_label_inst(*$1));}
    
enter
    : ENTER L_INT                     {$$ = new quad_pointer(new_enter_procedure($2));}

address
    : name                {$$ = new address_pointer(new_name_address(*$1));}
    
    | L_INT             {$$ = new address_pointer(new_integer_constant($1));}
    
    | L_FLOAT             {$$ = new address_pointer(new_float_constant($1));}
    
    | L_BOOL             {$$ = new address_pointer(new_boolean_constant($1));}

name
    : ID                {$$ = $1;}
    
    | ID '.' name       {$$ = new std::string(*$1 + '.' + *$3);}
    
    
label
    : ID '.' ID                  {$$ = new address_pointer(
                                            new_method_label_address(*$3, *$1));}
                                             
    | ID                         {$$ = new address_pointer(
                                            new_label_address(*$1));}

relop
    : LESS                    {$$ = quad_oper::LESS;}
    
    | LESS_EQUAL              {$$ = quad_oper::LESS_EQUAL;}
    
    | GREATER                 {$$ = quad_oper::GREATER;}
    
    | GREATER_EQUAL           {$$ = quad_oper::GREATER_EQUAL;}
    
    | EQUAL                   {$$ = quad_oper::EQUAL;}
    
    | DISTINCT                {$$ = quad_oper::DISTINCT;}
%%

void irerror(const char *s) {
    std::cout << "IR parser: Parse error on line " << ir_line_num << 
                "! Message: " << s << std::endl;

    // might as well halt now:
    exit(EXIT_FAILURE);
}

// TODO: ordenar esto...
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE ir_scan_string(const char * str);
extern void ir_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void ir_delete_buffer(YY_BUFFER_STATE buffer);

void translate_ir_code(std::string program){
    YY_BUFFER_STATE program_buffer = ir_scan_string(program.c_str());
    ir_switch_to_buffer(program_buffer);

    irparse();

    ir_delete_buffer(program_buffer);
}