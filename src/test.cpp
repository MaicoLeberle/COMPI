#include <iostream>
//#include "node.h"

//extern node_program* ast;
//extern int yyparse();

int main(int argc, const char* argv[]) {
	std::string full_test_program =
		"class Program {\n\tint inc(int x){\n\t\treturn x + 1;\n\t}\n\n"
		"\tint read_int() extern;\n\n\tvoid print(string s) extern;\n\n"
		"\tvoid main() {\n\t\tint y;\n\t\ty = read_int();\n\t\ty = inc(y);\n\n"
		"\t\tif (y == 1)\n\t\t\tprintf(\"y==1\");\n\t\telse\n\t\t\tprintf(\"y!=1\");\n"
		"\t}\n}";

	std::cout << full_test_program << std::endl;
}