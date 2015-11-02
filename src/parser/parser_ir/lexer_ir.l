%{
#include <iostream>
#include <string>
#include <cstdlib>
#include "../src/three_address_code.h"
#include "parser_ir.hpp"

int ir_line_num = 1;
%}

/* To avoid name conflicts with the COMPI's parser:
    http://www.gnu.org/software/bison/manual/html_node/Multiple-Parsers.html.
*/
%option prefix="ir"

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
{digit}+                    irlval.l_int = atoi(irtext); return L_INT;
{digit}+.{digit}+           irlval.l_float = atof(irtext); return L_FLOAT;
true                        irlval.l_bool = true; return L_BOOL;
false                       irlval.l_bool = false; return L_BOOL;
goto                        return GOTO;
if                          return IF;
ifTrue                      return IFTRUE;
ifFalse                     return IFFALSE;
param                       return PARAM;
call                        return CALL;
enter                       return ENTER;
return                      return RETURN;
=                           return '=';
\+=                         return PLUS_ASSIGN;
\-=                         return MINUS_ASSIGN;
\+                          return '+';
\-                          return '-';
\*                          return '*';
\/                          return '/';
%                           return '%';
\<                          return LESS;
\>                          return GREATER;
==                          return EQUAL;
!=                          return DISTINCT;
\<=                         return LESS_EQUAL;
\>=                         return GREATER_EQUAL;
&&                          return AND;
\|\|                        return OR;
not                         return NOT;
\[                          return '[';
\]                          return ']';
\,                          return ',';
\(                          return '(';
\)                          return ')';
\.                          return '.';
\:                          return ':';
{alpha}({alpha_num}|\.)*    {irlval.id = new std::string(irtext, irleng); 
                             return ID;}
\"(.|\\n)*\"                {irlval.l_str = new std::string(irtext, irleng); 
                             return L_STR;}
\n                          { ir_line_num++; }
.                           std::cout << "Unknown character!\n"; yyterminate();

%%