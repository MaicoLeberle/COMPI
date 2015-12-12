/* Lexer for the sub-set of assembly code needed for translate a program
   in COMPI.
*/
%{
#include <iostream>
#include <string>
#include <cstdlib>
#include "../src/asm_instruction.h"
#include "parser_asm.hpp"

int asm_line_num = 1;
%}

/* To avoid name conflicts with the COMPI's parser:
    http://www.gnu.org/software/bison/manual/html_node/Multiple-Parsers.html.
*/
%option prefix="asm"

/* To avoid the error "undefined reference to...":
   http://stackoverflow.com/questions/1811125/undefined-reference-to-yywrap. 
*/
%option noyywrap

digit [0-9]
comments \/\/.*|\/\*(.|\\n)*\*\/
spaces [ \t]|{comments}
alpha [a-zA-Z]
alpha_num {alpha}|{digit}|_
%%

{spaces}                    ;
-?{digit}+                  {asmlval.l_int = atoi(asmtext); 
                            return L_INT;}
-?{digit}+.{digit}+         {asmlval.l_float = atof(asmtext); 
                            return L_FLOAT;}
addl                         return ADDL;
imull                        return IMULL;
idivl                        return IDIVL;
subl                         return SUBL;
negl                         return NEGL;
sarl                         return SARL;
notl                         return NOTL;
shrl                         return SHRL;
movl                         return MOVL;
leal                         return LEAL;
jmp                          return JMP;
je                           return JE;
jne                          return JNE;
jl                           return JL;
jle                          return JLE;
jg                           return JG;
jge                          return JGE;
call                         return CALL;
leave                        return LEAVE;
ret                          return RET;
cmpl                         return CMPL;
enter                        return ENTER;
pushq                       return PUSHQ;
rax                         {asmlval.register_val = register_id::RAX; 
                            return REGISTER;}
rbx                         {asmlval.register_val = register_id::RBX;
                            return REGISTER;}
rcx                         {asmlval.register_val = register_id::RCX;
                            return REGISTER;}
rdx                         {asmlval.register_val  = register_id::RDX;
                            return REGISTER;}
rsi                         {asmlval.register_val = register_id::RSI;
                            return REGISTER;}
rdi                         {asmlval.register_val = register_id::RDI;
                            return REGISTER;}
rbp                         {asmlval.register_val = register_id::RBP;
                            return REGISTER;}
rsp                         {asmlval.register_val = register_id::RSP;
                            return REGISTER;}
r8                          {asmlval.register_val = register_id::R8;
                            return REGISTER;}
r9                          {asmlval.register_val = register_id::R9;
                            return REGISTER;}
r10                         {asmlval.register_val = register_id::R10;
                            return REGISTER;}
r11                         {asmlval.register_val = register_id::R11;
                            return REGISTER;}
r12                         {asmlval.register_val = register_id::R12;
                            return REGISTER;}
r13                         {asmlval.register_val = register_id::R13;
                            return REGISTER;}
r14                         {asmlval.register_val = register_id::R14;
                            return REGISTER;}
r15                         {asmlval.register_val = register_id::R15;
                            return REGISTER;}
eax                         {asmlval.register_val = register_id::EAX;
                            return REGISTER;}
ebx                         {asmlval.register_val = register_id::EBX;
                            return REGISTER;}
ecx                         {asmlval.register_val = register_id::ECX;
                            return REGISTER;}
edx                         {asmlval.register_val = register_id::EDX;
                            return REGISTER;}
r8d                         {asmlval.register_val = register_id::R8D;
                            return REGISTER;}
r9d                         {asmlval.register_val = register_id::R9D;
                            return REGISTER;}
r10d                        {asmlval.register_val = register_id::R10D;
                            return REGISTER;}
r11d                        {asmlval.register_val = register_id::R11D;
                            return REGISTER;}
r12d                        {asmlval.register_val = register_id::R12D;
                            return REGISTER;}
r13d                        {asmlval.register_val = register_id::R13D;
                            return REGISTER;}
r14d                        {asmlval.register_val = register_id::R14D;
                            return REGISTER;}
r15d                        {asmlval.register_val = register_id::R15D;
                            return REGISTER;}
r8b                         {asmlval.register_val = register_id::R8B;
                            return REGISTER;}
r9b                         {asmlval.register_val = register_id::R9B;
                            return REGISTER;}
r10b                        {asmlval.register_val = register_id::R10B;
                            return REGISTER;}
r11b                        {asmlval.register_val = register_id::R11B;
                            return REGISTER;}
r12b                        {asmlval.register_val = register_id::R12B;
                            return REGISTER;}
r13b                        {asmlval.register_val = register_id::R13B;
                            return REGISTER;}
r14b                        {asmlval.register_val = register_id::R14B;
                            return REGISTER;}
r15b                        {asmlval.register_val = register_id::R15B;
                            return REGISTER;}
{alpha}+({alpha_num}|\.|\_|\$)*\: {asmlval.label_id = new std::string(asmtext, asmleng-1);
                                return LABEL;}
\$                          return '$';
\*                          return '*';
%                           return '%';
\,                          return ',';
\(                          return '(';
\)                          return ')';
\.                          return '.';
\:                          return ':';
{alpha}+({alpha_num}|\.|\_|\$)* {asmlval.label_id = new std::string(asmtext, asmleng);
                            return LABEL_ID;}
\"(.|\\n)*\"                {asmlval.l_str = new std::string(asmtext, asmleng); 
                            return L_STR;}
\n                          { asm_line_num++; }
.                           { std::cout << "Unknown character:" << asmtext << "\n";
                                yyterminate();}

%%