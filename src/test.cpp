#include <iostream>
#include <assert.h>
#include "node2.h"

extern node_program* ast;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

int main(int argc, const char* argv[]) {
    std::string full_test_program =
		"class Program {\n\tint inc(int x){\n\t\treturn x + 1;\n\t}\n\n"
		"\tint read_int() extern;\n\n\tvoid print(string s) extern;\n\n"
		"\tvoid main() {\n\t\tint y;\n\t\ty = read_int();\n\t\ty = inc(y);\n\n"
		"\t\tif (y == 1)\n\t\t\tprintf(\"y==1\");\n\t\telse\n\t\t\tprintf(\"y!=1\");\n"
		"\t}\n}\0\0"; // Double NULL termination (needed for flex)

    YY_BUFFER_STATE program_buffer = yy_scan_string(full_test_program.c_str());
    yy_switch_to_buffer(program_buffer);

    yyparse();

    std::cout << "Number of classes should be 1: ";
    assert(ast->classes->size() == 1);
    std::cout << "OK" << std::endl;

    std::cout << "Main class id should be Program: ";
    assert(ast->classes->at(0)->id.compare("Program") == 0);
    std::cout << "OK" << std::endl;

    yy_delete_buffer(program_buffer);

    return 0;
}