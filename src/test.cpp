#include <iostream>
#include <cassert>
#include "node.h"

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

    // Test 1
    std::cout << "Number of classes should be 1: ";
    
    assert(ast->classes->size() == 1);
    
    std::cout << "OK" << std::endl;

    // Test 2
    std::cout << "Main class id should be Program: ";
    node_class_decl *main_class = ast->classes->at(0);
    
    assert(main_class->id.compare("Program") == 0);
    
    std::cout << "OK" << std::endl;

    // Test 3
    std::cout << "First statement should be inc declaration: ";
    node_method_decl *method = dynamic_cast<node_method_decl*>(main_class->class_block->at(0));

    assert(method->m_type->compare("int") == 0);  // Method type should be int
    assert(method->id->compare("inc") == 0); // Method id should be inc
    assert(method->formal_params->size() == 1); // Method params count should 1
    assert(method->formal_params->at(0)->type->compare("int") == 0); // The only param should be of type 'int' ...
    assert(method->formal_params->at(0)->id->compare("x") == 0); // ... and id 'x'
    
    std::cout << "OK" << std::endl;
    
    // Test 4
    std::cout << "Body of 'inc' should be 'return x + 1': ";
    node_block *method_statements = method->m_body->b_block;
    
    assert(method_statements->block_content->size() == 1); // The statement quantity should be 1
    
    node_return_statement *return_statement = dynamic_cast<node_return_statement*>(method_statements->block_content->at(0));
    node_binary_operation_expr *returned_expr = dynamic_cast<node_binary_operation_expr*>(return_statement->e);
    std::string *returned_x = dynamic_cast<node_location*>(returned_expr->lhs_expr)->ids->at(0);
    long long returned_1 = dynamic_cast<node_int_literal*>(returned_expr->rhs_expr)->value;
    
    assert(returned_expr->op.compare("+") == 0); // The operation should be a plus
    assert(returned_x->compare("x") == 0); // The left expr should be a location of name 'x'
    assert(returned_1 == 1); // The right expr should be a integer of value 1
    std::cout << "OK" << std::endl;

    yy_delete_buffer(program_buffer);

    return 0;
}
