#include <iostream>
#include <cassert>
#include "node.h"
#include "semantic_analysis.h"

extern program_pointer ast;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

void test_parser(){
	std::cout << "Testing parsing: " << std::endl;

	std::string full_test_program =
	        "class Program {\n\tint inc(int x){\n\t\treturn x + 1;\n\t}\n\n"
	        "\tint read_int() extern;\n\n\tvoid print(string s) extern;\n\n"
	        "\tvoid main() {\n\t\tint y;\n\t\ty = read_int();\n\t\ty = inc(y);\n\n"
	        "\t\tif (y < 1)\n\t\t\tprintf(\"y<1\");\n\t\telse\n\t\t\tprintf(\"y>=1\");\n"
	        "\t}\n}\0\0"; // Double NULL termination (needed for flex)

	YY_BUFFER_STATE program_buffer = yy_scan_string(full_test_program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	// Test 1
	std::cout << "1) Number of classes should be 1: ";

	assert(ast->classes.size() == 1);

	std::cout << "OK" << std::endl;

	// Test 2
	std::cout << "2) Main class id should be Program: ";
	class_pointer main_class = ast->classes[0];

	assert(main_class->id.compare("Program") == 0);

	std::cout << "OK" << std::endl;

	// Test 3
	std::cout << "3) First statement should be inc declaration: ";
	method_pointer method = std::dynamic_pointer_cast<node_method_decl>(main_class->class_block[0]);

	assert(method->type.type == Type::INTEGER);  // Method type should be int
	assert(method->id.compare("inc") == 0); // Method id should be inc
	assert(method->parameters.size() == 1); // Method params count should 1
	assert(method->parameters[0]->type.type == Type::INTEGER); // The only param should be of type 'int' ...
	assert(method->parameters[0]->id.compare("x") == 0); // ... and id 'x'

	body_pointer method_body = method->body;
	statement_list method_statements = method_body->block->content;

	assert(method_body->is_extern == false); // The body should not be extern
	assert(method_statements.size() == 1); // The statement quantity should be 1

	return_pointer return_statement = std::dynamic_pointer_cast<node_return_statement>(method_statements[0]);
	binary_operation_pointer returned_expr = std::dynamic_pointer_cast<node_binary_operation_expr>(return_statement->expression);
	std::string returned_x = std::dynamic_pointer_cast<node_location>(returned_expr->left)->ids[0];
	long long returned_1 = std::dynamic_pointer_cast<node_int_literal>(returned_expr->right)->value;

	assert(returned_expr->oper == Oper::PLUS); // The operation should be a plus
	assert(returned_x.compare("x") == 0); // The left expr should be a location of name 'x'
	assert(returned_1 == 1); // The right expr should be a integer of value 1
	std::cout << "OK" << std::endl;

	// Test 4
	std::cout << "4) Second statement should be extern read_int declaration: ";
	method.reset();
	method_body.reset();
	method = std::dynamic_pointer_cast<node_method_decl>(main_class->class_block[1]);
	method_body = method->body;

	assert(method->type.type == Type::INTEGER);  // Method type should be int
	assert(method->id.compare("read_int") == 0); // Method id should be read_int
	assert(method->parameters.size() == 0); // Method params count should 0
	assert(method_body->is_extern == true); // Method should have an external body

	std::cout << "OK" << std::endl;

	// Test 5
	std::cout << "5) Third statement should be extern print declaration: ";
	method.reset();
	method_body.reset();
	method = std::dynamic_pointer_cast<node_method_decl>(main_class->class_block[2]);
	method_body = method->body;

	assert(method->type.type == Type::VOID);  // Method type should be void
	assert(method->id.compare("print") == 0); // Method id should be print
	assert(method->parameters.size() == 1); // Method params count should 0
	assert(method->parameters[0]->type.type == Type::ID); // The only param should be of user defined type ...
	assert(method->parameters[0]->type.id.compare("string") == 0); // ... 'string' ...
	assert(method->parameters[0]->id.compare("s") == 0); // ... and id 's'
	assert(method_body->is_extern == true); // Method should have an external body

	std::cout << "OK" << std::endl;

	// Test 6
	std::cout << "6) Final statement should be the main function: ";
	method.reset();
	method_body.reset();
	method = std::dynamic_pointer_cast<node_method_decl>(main_class->class_block[3]);
	method_body = method->body;
	method_statements = method_body->block->content;

	assert(method->type.type == Type::VOID);  // Method type should be void
	assert(method->id.compare("main") == 0); // Method id should be main
	assert(method->parameters.size() == 0); // Method params count should 0
	assert(method_body->is_extern == false); // Method should not have an external body
	assert(method_statements.size() == 4); // Method should have 4 statements

	field_pointer y_decl = std::dynamic_pointer_cast<node_field_decl>(method_statements[0]);

	assert(y_decl->type.type == Type::INTEGER); // The declared y should be of type int
	assert(y_decl->ids.size() == 1); // Should have only 1 var declared
	assert(y_decl->ids[0]->id.compare("y") == 0); // The name of the var is 'y'

	std::cout << "OK" << std::endl;

	yy_delete_buffer(program_buffer);

	ast.reset();
}

/**********************************************
 * SEMANTIC ANALYSIS
 **********************************************/
void test_rule_1(){
	std::cout << "1) An identifier is declared at most once, into a given scope:";
	semantic_analysis v1;

	std::string full_test_program = "class Program {int x,x;} class main {void main(){}}\0\0";

	YY_BUFFER_STATE program_buffer = yy_scan_string(full_test_program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	ast->accept(v1);

	assert(v1.get_last_error() == semantic_analysis::ERROR_1);

	std::cout << "OK. " << std::endl;

}

void test_rule_3(){
	std::cout << "3) Every program has one class with name \"main\", and a \"main\" method: ";

	// No main class declared
	semantic_analysis v1;

	std::string full_test_program = "class Program {}\0\0";

	YY_BUFFER_STATE program_buffer = yy_scan_string(full_test_program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	ast->accept(v1);

	assert(v1.get_last_error() == semantic_analysis::ERROR_3);

	// Main class declared, without a \"main\" method
	semantic_analysis v2;

	full_test_program = "class main {}\0\0";

	program_buffer = yy_scan_string(full_test_program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	ast->accept(v2);

	assert(v2.get_last_error() == semantic_analysis::ERROR_3);

	std::cout << "OK. " << std::endl;

}

void test_semantic_analysis(){
	std::cout << "Testing semantic analysis: " << std::endl;

	test_rule_1();
	test_rule_3();
}

int main(int argc, const char* argv[]) {
	test_parser();
	test_semantic_analysis();

    return 0;
}
