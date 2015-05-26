%{
#include <iostream>
#include <string>
#include <cstdlib>
#include "../src/node.h"
#include "parser.hpp"

int line_num = 1;
%}

digit [0-9]
comments \/\/.*|\/\*(.|\\n)*\*\/
spaces [ \t]|{comments}
alpha [a-zA-Z]
alpha_num {alpha}|{digit}|_

%%

{spaces}                    ;
{digit}+                    yylval.l_int = atoi(yytext); return L_INT;
{digit}+.{digit}+           yylval.l_float = atof(yytext); return L_FLOAT;
true                        yylval.l_bool = true; return L_BOOL;
false                       yylval.l_bool = false; return L_BOOL;
class                       return CLASS;
void                        return VOID;
extern                      return EXTERN;
int                         return INT;
float                       return FLOAT;
boolean                     return BOOLEAN;
if                          return IF;
else                        return ELSE;
for                         return FOR;
while                       return WHILE;
return                      return RETURN;
break                       return BREAK;
continue                    return CONTINUE;
\"(.|\\n)*\"                yylval.l_str = new std::string(yytext, yyleng); return L_STR;
=                           return '=';
\+=                         return PLUS_ASSIGN;
\-=                         return MINUS_ASSIGN;
\+                          return '+';
\-                          return '-';
\*                          return '*';
\/                          return '/';
%                           return '%';
\<                          return '<';
\>                          return '>';
==                          return EQUAL;
!=                          return DISTINCT;
\<=                         return LESS_EQUAL;
\>=                         return GREATER_EQUAL;
&&                          return AND;
\|\|                        return OR;
\{                          return '{';
\}                          return '}';
\[                          return '[';
\]                          return ']';
\,                          return ',';
;                           return ';';
\(                          return '(';
\)                          return ')';
!                           return '!';
\.                          return '.';
{alpha}{alpha_num}*         yylval.id = new std::string(yytext, yyleng); return ID;
\n                          { ++line_num; }
.                           std::cout << "Unknown character!\n"; yyterminate();

%%
