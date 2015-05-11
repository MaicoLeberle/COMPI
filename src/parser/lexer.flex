
digit [0-9]
comments \/\/.*|\/\*(.|\\n)*\*\/
spaces [ \t|\n]|{comments}

%%

{spaces}                    printf("Eliminando espacios en blanco: %s\n", yytext );
{digit}+                    printf("Un entero: %s\n", yytext );
{digit}+.{digit}+           printf("Un real: %s\n", yytext );
true|false                  printf("Un booleano: %s\n", yytext );
class                       printf("Palabra clave: %s\n", yytext );
void                        printf("Palabra clave: %s\n", yytext );
extern                      printf("Palabra clave: %s\n", yytext );
int                         printf("Palabra clave: %s\n", yytext );
float                       printf("Palabra clave: %s\n", yytext );
boolean                     printf("Palabra clave: %s\n", yytext );
if                          printf("Palabra clave: %s\n", yytext );
else                        printf("Palabra clave: %s\n", yytext );
for                         printf("Palabra clave: %s\n", yytext );
while                       printf("Palabra clave: %s\n", yytext );
return                      printf("Palabra clave: %s\n", yytext );
break                       printf("Palabra clave: %s\n", yytext );
continue                    printf("Palabra clave: %s\n", yytext );
\"(.|\\n)*\"                printf("Una cadena de caracteres: %s\n", yytext );
"="                         printf("Símbolo de asignación: %s\n", yytext );
"+="                        printf("Símbolo de asignación: %s\n", yytext );
"-="                        printf("Símbolo de asignación: %s\n", yytext );
\+                          printf("Símbolo de asignación: %s\n", yytext );
-                           printf("Operador: %s\n", yytext );
\*                          printf("Operador: %s\n", yytext );
\/                          printf("Operador: %s\n", yytext );
%                           printf("Operador: %s\n", yytext );
==                          printf("Operador: %s\n", yytext );
!=                          printf("Operador: %s\n", yytext );
&&                          printf("Operador: %s\n", yytext );
\|\|                        printf("Operador: %s\n", yytext );
\{                          printf("Delimitador: %s\n", yytext );
\}                          printf("Delimitador: %s\n", yytext );
\[                          printf("Delimitador: %s\n", yytext );
\]                          printf("Delimitador: %s\n", yytext );
,                           printf("Delimitador: %s\n", yytext );
;                           printf("Delimitador: %s\n", yytext );
\(                          printf("Delimitador: %s\n", yytext );
\)                          printf("Delimitador: %s\n", yytext );
!                           printf("Delimitador: %s\n", yytext );
.                           printf("Caracter desconocido: %s\n", yytext );
%%

main(int argc, char** argv) {
    ++argv, --argc;
    if (argc > 0)
        yyin = fopen(argv[0], "r");
    else
        yyin = stdin;
    
    yylex();
}
