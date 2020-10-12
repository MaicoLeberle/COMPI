#include <iostream>

#include <fstream>
// For execution of bash commands with std::system().
#include <cstdlib>
// Check if file or directory exists
#include <sys/stat.h>

#include <cassert>
#include <string>
#include "node.h"
#include "semantic_analysis.h"
#include "./tests/test_inter_code_gen_visitor.h"
#include "./tests/test_asm_code_generator.h"
#include "./tests/test_asm_instruction.h"
#include "./tests/test_ir_parser.h"
#include "./tests/test_asm_parser.h"

extern program_pointer ast;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

void test_parser_1(){
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
	std::cout << "2) main class id should be Program: ";
	class_pointer main_class = ast->classes[0];

	assert(main_class->id.compare("Program") == 0);

	std::cout << "OK" << std::endl;

	// Test 3
	std::cout << "3) First statement should be inc declaration: ";
	method_pointer method = std::dynamic_pointer_cast<node_method_decl>(main_class->class_block[0]);

	assert(method->type.type == Type::TINTEGER);  // Method type should be int
	assert(method->id.compare("inc") == 0); // Method id should be inc
	assert(method->parameters.size() == 1); // Method params count should 1
	assert(method->parameters[0]->type.type == Type::TINTEGER); // The only param should be of type 'int' ...
	assert(method->parameters[0]->id.compare("x") == 0); // ... and id 'x'

	body_pointer method_body = method->body;
	statement_list method_statements = method_body->block->content;

	assert(method_body->is_extern == false); // The body should not be extern
	assert(method_statements.size() == 1); // The statement quantity should be 1

	return_pointer return_statement = std::dynamic_pointer_cast<node_return_statement>(method_statements[0]);
	binary_operation_pointer returned_expr = std::dynamic_pointer_cast<node_binary_operation_expr>(return_statement->expression);
	std::string returned_x = std::dynamic_pointer_cast<node_location>(returned_expr->left)->ids[0];
	long long returned_1 = std::dynamic_pointer_cast<node_int_literal>(returned_expr->right)->value;

	assert(returned_expr->oper == Oper::OPLUS); // The operation should be a plus
	assert(returned_x.compare("x") == 0); // The left expr should be a location of name 'x'
	assert(returned_1 == 1); // The right expr should be a integer of value 1
	std::cout << "OK" << std::endl;

	// Test 4
	std::cout << "4) Second statement should be extern read_int declaration: ";
	method.reset();
	method_body.reset();
	method = std::dynamic_pointer_cast<node_method_decl>(main_class->class_block[1]);
	method_body = method->body;

	assert(method->type.type == Type::TINTEGER);  // Method type should be int
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

	assert(method->type.type == Type::TVOID);  // Method type should be void
	assert(method->id.compare("print") == 0); // Method id should be print
	assert(method->parameters.size() == 1); // Method params count should 0
	assert(method->parameters[0]->type.type == Type::TSTRING); // The only param should be an string...
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

	assert(method->type.type == Type::TVOID);  // Method type should be void
	assert(method->id.compare("main") == 0); // Method id should be main
	assert(method->parameters.size() == 0); // Method params count should 0
	assert(method_body->is_extern == false); // Method should not have an external body
	assert(method_statements.size() == 4); // Method should have 4 statements

	field_pointer y_decl = std::dynamic_pointer_cast<node_field_decl>(method_statements[0]);

	assert(y_decl->type.type == Type::TINTEGER); // The declared y should be of type int
	assert(y_decl->ids.size() == 1); // Should have only 1 var declared
	assert(y_decl->ids[0]->id.compare("y") == 0); // The name of the var is 'y'

	std::cout << "OK" << std::endl;

	yy_delete_buffer(program_buffer);

	ast.reset();
}

void test_parser_2(){
	std::string full_test_program =
			"class Program {int x;}\n"
			"class main {void main() {}}\0\0"; // Double NULL termination (needed for flex)

	YY_BUFFER_STATE program_buffer = yy_scan_string(full_test_program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	std::cout << "7) Declaration of multiple classes: ";

	assert(ast->classes.size() == 2);

	std::cout << "OK" << std::endl;

	yy_delete_buffer(program_buffer);

	ast.reset();
}
void test_parser(){
	test_parser_1();
	test_parser_2();
}

/**********************************************
 * SEMANTIC ANALYSIS
 **********************************************/
semantic_analysis::error_id execute_test(std::string program){
	semantic_analysis v;

	YY_BUFFER_STATE program_buffer = yy_scan_string(program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	ast->accept(v);

	yy_delete_buffer(program_buffer);

	return v.get_last_error();
}

void test_rule_1(){
	std::cout << "1) An identifier is declared at most once, into a "
			"given scope:\n";
	std::string test_program = "class Program {\n"
										"int x,x;\n"
									"}\n"
									"class main {\n"
										"void main(){\n"
										"}\n"
									"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_1);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_2(){
	std::cout << "2) An identifier is declared before it is used:\n";
	std::string test_program = "class Program {\n"
									"void method(){\n"
										"x = 1;"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_2);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_3(){
	std::cout << "3) Every program has one class with name \"main\", and a "
			"\"main\" method:\n";

	// No main class declared
	std::string test_program = "class Program {}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_3);

	// main class declared, without a \"main\" method
	test_program = "class main {}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_3);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_4(){
	std::cout << "4) In an array declaration, the length must be > 0:\n";

	// Array declaration with length == 0
	std::string test_program = "class Program {\n"
										"int x[0];\n"
									"}\n"
									"class main {\n"
										"void main(){\n"
										"}\n"
									"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_4);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_5(){
	std::cout << "5) The number and type of the actual parameters, must be the "
				"same of the formal parameters:\n";

	// Wrong quantity of actual parameters.
	std::string test_program = "class Program {\n"
									"void method1(){\n"
									"}\n"
									"void method2(){\n"
										"int x;\n"
										"method1(x);\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_5);

	test_program = "class Program {\n"
						"void method1(int x){\n"
						"}\n"
						"void method2(){\n"
							"int x;\n"
							"method1(x, x);\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_5);

	// Wrong type of actual parameters
	test_program = "class Program {\n"
						"void method1(int x){\n"
						"}\n"
						"void method2(){\n"
							"boolean y;\n"
							"method1(y);\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_5);

	test_program = "class Program {\n"
						"void method1(int x, int y){\n"
						"}\n"
						"void method2(){\n"
							"boolean z;\n"
							"int u;\n"
							"method1(z, u);\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_5);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_6(){
	std::cout << "6) If a method call used is used as expression, the"
			"called method must return a result:\n";

	// void method in return expression (same as rule 9)
	std::string test_program = "class Program {\n"
										"void method1(){\n"
										"}"
										"int method2(){\n"
											"return method1();"
										"}"
									"}\n"
									"class main {\n"
										"void main(){\n"
										"}\n"
									"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_9);

	// void method in a conditional's guard (same as rule 12)
	test_program = "class Program {\n"
						"void method1(){\n"
						"}"
						"void method2(){\n"
							"if (method1()) ; else ;\n"
						"}"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_12);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_7(){
	std::cout << "7) String literals only with extern methods:\n";

	std::string test_program = "class Program {\n"
									"boolean method1(string x){\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_7);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_8(){
	std::cout << "8) A return statement must have an associated expression only "
				"if the method returns a value:\n";

	std::string test_program = "class Program {\n"
									"void method(){\n"
										"return 1;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_8);

	test_program = "class Program {\n"
						"int method(){\n"
							"return;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_8);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_9(){
	std::cout << "9) The type of the value returned from a method must be the "
				"same than the type of the expression of the return statement:\n";

	std::string test_program = "class Program {\n"
									"boolean method(){\n"
										"return 1;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_9);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_11(){
	std::cout << "11) If the location is an array position, the corresponding "
				 "id must point to an array, and the index must be an integer:\n";

	// Wrong index
	std::string test_program = "class Program {\n"
									"int x[1];\n"
									"void method(){\n"
										"x[true] = 1;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_11);

	test_program = "class Program {\n"
						"int x[1];\n"
						"void method1(){\n"
						"}\n"
						"void method2(){\n"
							"x[method1()] = 1;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_11);

	// Wrong id type
	test_program = "class Program {\n"
						"int x;\n"
						"void method(){\n"
							"x[0] = 1;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_11);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_12(){
	std::cout << "12) The guard, of a conditional and a while loop, must be a "
				"boolean expression:\n";

	// If statement
	std::string test_program = "class Program {\n"
									"void method(){\n"
										"if (1) ; else ;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_12);

	test_program = "class Program {\n"
						"void method1(){\n"
						"}\n"
						"void method2(){\n"
							"if (method1()) ; else ;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_12);

	// While statement
	test_program = "class Program {\n"
						"void method(){\n"
							"while (1) ;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_12);

	test_program = "class Program {\n"
						"void method1(){\n"
						"}\n"
						"void method2(){\n"
							"while (method1()) ;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_12);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_13(){
	std::cout << "13) Operands of arithmetic and relational operations,"
				"must have type int or float:\n";

	std::string test_program = "class Program {\n"
									"boolean method(){\n"
										"return true+1;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_13);

	test_program = "class Program {\n"
						"boolean method(){\n"
							"return true*1;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_13);

	test_program = "class Program {\n"
						"boolean method(){\n"
							"return -true;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_13);

	test_program = "class Program {\n"
						"boolean method(){\n"
							"return true < 1;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_13);

	test_program = "class Program {\n"
						"boolean method(){\n"
							"return true <= 1;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_13);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_14(){
	std::cout << "14) eq_op operands must have the same type (int, float or "
			"boolean):\n";

	// Operands of different type
	std::string test_program = "class Program {\n"
									"boolean method(){\n"
										"return 1 == true;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_14);

	test_program = "class Program {\n"
						"boolean method(){\n"
							"return 1 != true;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_14);

	// Operands of the wrong type.
	test_program = "class Program {\n"
						"boolean method(){\n"
							"return \"asd\" != true;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_14);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_15(){
	std::cout << "15) cond_op and ! operands, must evaluate to a boolean:\n";

	// cond_op
	std::string test_program = "class Program {\n"
									"boolean method(){\n"
										"return 1 && true;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_15);

	test_program = "class Program {\n"
						"int method1(){\n"
							"return 1;\n"
						"}\n"
						"boolean method2(){\n"
							"return true && method1();\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_15);

	// !
	test_program = "class Program {\n"
						"boolean method1(){\n"
							"return !1;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_15);

	test_program = "class Program {\n"
						"int method1(){\n"
							"return 1;\n"
						"}\n"
						"boolean method2(){\n"
							"return !method1();\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_15);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_16(){
	std::cout << "16) Both, the type of the location and the expression to "
				"be assigned, must be the same:\n";

	// Wrong r-value's type, in attribute assignment.
	std::string test_program = "class Program {\n"
									"int x;"
									"void method(){\n"
										"x = true;"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_16);

	test_program = "class Program {\n"
						"int x[1];"
						"void method(){\n"
							"x[0] = true;"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_16);

	test_program = "class A {\n"
						"int x;\n"
					"}\n"
					"class B {\n"
						"A y;\n"
					"}\n"
					"class C {\n"
						"B z;\n"
						"void method(){\n"
							"z.y.x = true;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_16);

	// Wrong r-value's type, in method's parameter assignment.
	test_program = "class Program {\n"
						"void method(float x){\n"
							"x = true;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_16);

	// Wrong r-value's type, in method's local variable assignment.
	test_program = "class Program {\n"
						"void method(){\n"
							"float x;\n"
							"x = true;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_16);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_17(){
	std::cout << "17) In a -= or += assignment, the location and the expression "
				"assigned must evaluated to integer or float:\n";

	std::string test_program = "class Program {\n"
									"void method(){\n"
										"int x;"
										"x += true;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_17);

	test_program = "class Program {\n"
						"void method(){\n"
							"boolean x;"
							"x += 1;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_17);


	std::cout << "OK.\n" << std::endl;
}

void test_rule_18(){
	std::cout << "18) In a for loop, \"from\" and \"to\" expression must "
					"evaluate to integers:\n";

	std::string test_program = "class Program {\n"
									"void method(){\n"
										"int x;"
										"for x = true , 2 ;"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_18);

	test_program = "class Program {\n"
						"void method(){\n"
							"int x;"
							"for x = 1 , false ;"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_18);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_19(){
	std::cout << "19) break and continue, only into a loop's body:\n";

	std::string test_program = "class Program {\n"
									"void method(){\n"
										"break;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_19);

	test_program = "class Program {\n"
						"void method(){\n"
							"continue;\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_19);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_20(){
	std::cout << "20) Identifiers have known type:\n";

	// Attributes
	std::string test_program = "class Program {\n"
									"unkownClass x;\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_20);

	// Method's parameters
	test_program = "class Program {\n"
						"void method(unknownClass x){\n"
						"}\n"
					"}\n"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_20);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_21(){
	std::cout << "21) Access of attributes only over instances:\n";

	std::string test_program = "class Program {\n"
									"int x,y;\n"
									"void method(){\n"
										"x = y.z;\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_21);

	std::cout << "OK.\n" << std::endl;
}


void test_rule_22(){
	std::cout << "22) Method call operation over...methods only:\n";

	std::string test_program = "class Program {\n"
									"int x[1];\n"
									"void method(){\n"
										"x();\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_22);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_23(){
	std::cout << "23) Parameter's identifier and method's name must differ:\n";

	std::string test_program = "class Program {\n"
									"void method(int method){\n"
									"}\n"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_23);

	std::cout << "OK.\n" << std::endl;
}

void test_rule_24(){
	std::cout << "24) Attributes cannot have as type the class where they belong to: \n";

	std::string test_program = "class Program {\n"
									"Program x;"
								"}\n"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	assert(execute_test(test_program) == semantic_analysis::ERROR_24);

	std::cout << "OK.\n" << std::endl;
}

void test_semantic_analysis(){
	std::cout << "\nTesting semantic analysis: " << std::endl;

	test_rule_1();
	test_rule_2();
	test_rule_3();
	test_rule_4();
	test_rule_5();
	test_rule_6();
	test_rule_7();
	test_rule_8();
	test_rule_9();
	test_rule_11();
	test_rule_12();
	test_rule_13();
	test_rule_14();
	test_rule_15();
	test_rule_16();
	test_rule_17();
	test_rule_18();
	test_rule_19();
	test_rule_20();
	test_rule_21();
	test_rule_22();
	test_rule_23();
	test_rule_24();
}


bool exists(std::string path) {
	std::fstream f(path, std::ios::in | std::ios::ate);
	bool res = !f.fail();
	if (res)
		f.close();
	return (res);
}

bool files_exist(std::string path_temp, unsigned int t_index, unsigned int f_index) {
	std::string input = path_temp
		+ std::to_string(t_index)
		+ std::string("/input")
		+ std::to_string(f_index);
	std::string output = path_temp
		+ std::to_string(t_index)
		+ std::string("/output")
		+ std::to_string(f_index);

	return (exists(input) && exists(output));
}

/*	Test the "semantics" (with respect to what the outputs should be, 
	given certain input) of the test cases in ../test/*.				 

	This testing has the preconditions of having a text output file in 
	test/files/testX/execution/output (for X = test0, test1, ...) and a 
	executable binary in test/files/testX/execution/bin (for X = test0, 
	test1, ...).													     */
void test_semantics_of_test_cases() {

	std::cout << std::endl << std::endl << "Checking semantics of every test case provided:" << std::endl;

	/*	Test case index. */
	unsigned int t_index = 0;
	/* (input, output) pair index. */
	unsigned int f_index = 0;
	const std::string path_temp = ("../test/files/test");
	
	/*	Input file. */
	std::string input_path;
	
	/*	Output file of execution. */
	std::string output_path;

	/*	The expected output file. */
	std::string exp_output_path;

	while(exists(path_temp + std::to_string(t_index) + std::string("/execution/bin"))) {
		/*	The binary file to be checked on exists. */
		
		f_index = 0;
		std::string binary = path_temp
					+ std::to_string(t_index)
					+ std::string("/execution/bin");

		while(files_exist(path_temp, t_index, f_index)) {
			/*	There is a new (input, output) pair to be checked.			 */

			input_path = path_temp 
					+ std::to_string(t_index) 
					+ std::string("/input") 
					+ std::to_string(f_index);
			output_path = path_temp
					+ std::to_string(t_index)
					+ std::string("/execution/output");
			exp_output_path = path_temp 
					+ std::to_string(t_index) 
					+ std::string("/output") 
					+ std::to_string(f_index);

			std::string execution = binary
					+ (std::string(" < ") + input_path)
					+ (std::string(" > ") + output_path);
			std::system(execution.c_str());
			

			/*	------------------------------------------------------------ */
			/*	These instructions check that there was no difference between
				the output that resulted from execution, and the expected 
				output. 													 */

			// First, build diff command.
			std::string diff = std::string("diff ")
					+ output_path
					+ std::string(" ")
					+ exp_output_path
					+ std::string(" > extra.diff");
			// Execute diff command.
			std::system(diff.c_str());
			// Open file that contains results of diff execution.
			std::fstream extra;
			extra.open("extra.diff", std::fstream::in);
			assert(!extra.fail());
			// Check that the end of the file is at position 0 
			// (i.e., length is 0).
			extra.seekg(0, extra.end);
			if ((int)extra.tellg() != 0) {
				std::cout << "Differences have been found between ";
				std::cout << output_path << " and " << exp_output_path;
				std::cout << ":" << std::endl;
				std::system("cat extra.diff");
				exit(1);
			}

			/*	------------------------------------------------------------ */

			++f_index;
		}
	
		++t_index;
	}
	
	if (exists(std::string("extra.diff")))
		std::system("rm extra.diff");

	std::cout << "\tSemantics of the test cases has been proven correct." << std::endl;
}

int main(int argc, const char* argv[]) {

	std::cout << std::endl;

	test_parser();
	//test_semantic_analysis();
	test_inter_code_gen_visitor();
	test_asm_code_generator();
	test_asm_instruction();
	test_ir_parser();
	test_asm_parser();

	test_semantics_of_test_cases();

	std::cout << std::endl;

    return 0;
}
