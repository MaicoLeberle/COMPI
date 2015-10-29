// An Intel's syntax ACM's parser
%{
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "../src/asm_instruction.h"

extern int asmlex();
extern FILE *asmin;
extern int asm_line_num;
void asmerror(const char *s);

asm_instructions_list *asm_code; // Pointer to the instructions' list.
%}

%define api.prefix {asm}
 
%union {
    asm_instructions_list* asm_inst_list;
    asm_instruction_pointer *instruction;
    operand_pointer *arg;
    int l_int;
    float l_float;
    std::string* l_str;
    std::string* label_id;
    std::string* label;
    register_id register_val;
    operation op;
}

%token <l_int> L_INT
%token <l_float> L_FLOAT
%token <l_str> L_STR
%token <register_val> REGISTER
%token <label_id> LABEL_ID LABEL
%token <op> OPERATION
%token <token> ADD IMUL MUL IDIV DIV SUB NEG SAR NOT SHR MOV JMP JE JNE JL JLE 
               JG JGE CALL LEAVE RET CMP ENTER

%type <asm_inst_list> inst_list
%type <instruction> instruction arithmetic logic data_transfer control_transfer
                    data_comparison misc
%type <arg> source destination register memory immediate
%type <l_int> int_operand

%start inst_list

%%
inst_list
    : inst_list instruction    {$1->push_back(*$2);
                                $$ = $1;
                                asm_code = $$;}

    | instruction              {$$ = new asm_instructions_list();
                                $$->push_back(*$1);
                                asm_code = $$;}
    ;
    
instruction
    : arithmetic                {$$ = $1;}
    
    | logic                     {$$ = $1;}
    
    | data_transfer             {$$ = $1;}
    
    | control_transfer          {$$ = $1;}
    
    | data_comparison             {$$ = $1;}
    
    | misc             {$$ = $1;}
    
// TODO: no haria falta mantener ',' a estas alturas...
// TODO: podriamos definir una categoria sintactico "origen" y "destino",
// que abstraiga el tipo posible de operandos...
// TODO: qué pasa con el tipo de los operandos?
arithmetic
    : ADD source ',' destination   {$$ = new asm_instruction_pointer(
                                    new_add_instruction(*$2, *$4, data_type::L));}
                                    
    | IMUL source ',' destination   {$$ = new asm_instruction_pointer(
                                    new_mul_instruction(*$2, 
                                                        *$4, 
                                                        data_type::L,
                                                        true));}
    
    | MUL source ',' destination   {$$ = new asm_instruction_pointer(
                                    new_mul_instruction(*$2, 
                                                        *$4, 
                                                        data_type::L,
                                                        false));}
                                    
    | IDIV source                   {$$ = new asm_instruction_pointer(
                                    new_div_instruction(*$2, 
                                                        data_type::L,
                                                        true));}
    
    | DIV source                    {$$ = new asm_instruction_pointer(
                                    new_div_instruction(*$2, 
                                                        data_type::L,
                                                        false));}
                                    
    | NEG destination               {$$ = new asm_instruction_pointer(
                                    new_neg_instruction(*$2, data_type::L));}

logic
    : NOT destination                {$$ = new asm_instruction_pointer(
                                    new_not_instruction(*$2, data_type::L));}
                                    
    | SHR immediate ',' destination  {$$ = new asm_instruction_pointer(
                                    new_shr_instruction(*$2, *$4, data_type::L));}
                                    
data_transfer
    : MOV source ',' destination    {$$ = new asm_instruction_pointer(
                                    new_mov_instruction(*$2, *$4, data_type::L));}

// TODO: nos haran falta las versions de estas instrucciones en donde el lugar
// al que saltar se especifica mediante un registro?
control_transfer
    : JMP LABEL_ID                  {$$ = new asm_instruction_pointer(
                                    new_jmp_instruction(*$2));}
                                    
    | JE LABEL_ID                   {$$ = new asm_instruction_pointer(
                                    new_je_instruction(*$2));}
    
    | JNE LABEL_ID                  {$$ = new asm_instruction_pointer(
                                    new_jne_instruction(*$2));}
    
    | JL LABEL_ID                  {$$ = new asm_instruction_pointer(
                                    new_jl_instruction(*$2));}
    
    | JLE LABEL_ID                  {$$ = new asm_instruction_pointer(
                                    new_jle_instruction(*$2));}
    
    | JG LABEL_ID                  {$$ = new asm_instruction_pointer(
                                    new_jg_instruction(*$2));}
    
    | JGE LABEL_ID                  {$$ = new asm_instruction_pointer(
                                    new_jge_instruction(*$2));}
    
    | CALL LABEL_ID                  {$$ = new asm_instruction_pointer(
                                    new_call_instruction(*$2));}
    
    | LEAVE                         {$$ = new asm_instruction_pointer(
                                    new_leave_instruction());}
    
    | RET                           {$$ = new asm_instruction_pointer(
                                    new_ret_instruction());}
                                    
data_comparison
    : CMP source ',' source         {$$ = new asm_instruction_pointer(
                                    new_cmp_instruction(*$2, *$4, data_type::L));}
                                    
misc
    : ENTER immediate ',' immediate {$$ = new asm_instruction_pointer(
                                    new_enter_instruction(*$2, *$4));}
                                    
    | LABEL                         {$$ = new asm_instruction_pointer(
                                    new_label_instruction(*$1));}

source
    : immediate                 {$$ = $1;}
    | memory                    {$$ = $1;}
    | register                  {$$ = $1;}

destination
    : memory                    {$$ = $1;}
    | register                  {$$ = $1;}

register
    : '%' REGISTER             {$$ = new operand_pointer(
                                        new_register_operand($2));}
                                        
immediate
    : '$' int_operand           {$$ = new operand_pointer(
                                            new_immediate_integer_operand($2));}
                                            
    | '$' L_FLOAT               {$$ = new operand_pointer(
                                            new_immediate_float_operand($2));}

// TODO: cuando index es omitido, cual es el valor por defecto que debemos
// colocar?
memory
    : L_INT '(' REGISTER ',' REGISTER ',' L_INT ')' {$$ = new operand_pointer(
                                            new_memory_operand($1, $3, $5, $7));}
                                            
    | L_INT '(' REGISTER ',' REGISTER ')'           {$$ = new operand_pointer(
                                            new_memory_operand($1, $3, $5, 1));}
    
    | L_INT '(' REGISTER ')'                        {$$ = new operand_pointer(
                                            new_memory_operand($1, 
                                                                $3, 
                                                                register_id::RSP, 
                                                                1));}
                                            
    // TODO: asi?
    | L_INT '(' ')'                         {$$ = new operand_pointer(
                                            new_memory_operand($1, 
                                                                register_id::RSP, 
                                                                register_id::RSP, 
                                                                1));}
    
    | '(' REGISTER ',' REGISTER ',' L_INT ')'     {$$ = new operand_pointer(
                                            new_memory_operand(0, $2, $4, $6));}
                                           
    | '(' REGISTER ',' REGISTER ')'               {$$ = new operand_pointer(
                                            new_memory_operand(0, $2, $4, 1));}
    
    | '(' REGISTER ')'                         {$$ = new operand_pointer(
                                            new_memory_operand(0, 
                                                                $2, 
                                                                register_id::RSP, 
                                                                1));}

// To allow some static arithmetic expression in the place of an integer 
// operand.       
int_operand
    : L_INT                     {$$ = $1;}
    
    | L_INT '*' L_INT           {$$ = $1 * $3;}

%%

void asmerror(const char *s) {
    std::cout << "Parse error on line " << asm_line_num << "! Message: " << s << std::endl;

    // might as well halt now:
    exit(EXIT_FAILURE);
}