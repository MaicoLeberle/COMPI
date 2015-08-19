#include "test_inter_code_gen_visitor.h"

extern program_pointer ast;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

instructions_list& translate(inter_code_gen_visitor& v, std::string program){
	YY_BUFFER_STATE program_buffer = yy_scan_string(program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	ast->accept(v);

	yy_delete_buffer(program_buffer);
}

void test_three_address_code(){
	std::cout << "1) Three-address-code representation related procedures: ";

	address_pointer add1 = new_boolean_constant(boolean_initial_value);
	address_pointer add2 = new_boolean_constant(boolean_initial_value);

	// TODO: borrar add1 y add2
	assert(are_equal_pointers(add1, add2));

	add1 = new_integer_constant(integer_initial_value);
	add2 = new_integer_constant(integer_initial_value);

	assert(are_equal_pointers(add1, add2));

	add1 = new_float_constant(float_initial_value);
	add2 = new_float_constant(float_initial_value);

	assert(are_equal_pointers(add1, add2));

	std::cout << "OK. " << std::endl;
}
void test_class_decl(){
	std::cout << "2) Translation of a class declaration: ";

	// One empty class declaration...
	std::string test_program = "class Program {}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	inter_code_gen_visitor v1;
	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	// No instruction...
	// TODO: testear esto!
	//assert(translation->size() == 2);

	// Just data about the classes...
	intermediate_symtable *symtable = v1.get_symtable();

	assert(symtable->get(std::string("Program"))->get_class() ==
		   symtable_element::T_CLASS);

	assert(symtable->get(std::string("main"))->get_class() ==
			   symtable_element::T_CLASS);

	std::cout << "OK. " << std::endl;
}

void test_field_decl(){
	std::cout << "3) Translation of a field declaration: ";

	// Attributes of an object.
	std::string test_program = "class class1 { int n,m,o[1];"
								"				float x;"
								"				boolean b;}"
								"class class2 { class1 obj; }"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	// TODO: chequear esto!
	//assert(translation->size() == 0);

	// Data about the fields...
	intermediate_symtable *symtable = v1.get_symtable();

	// In class1...
	symtable_element *class_program = symtable->get(std::string("class1"));
	std::list<symtable_element> *fields =  class_program->get_class_fields();

	assert(fields->size() == 5);
	// TODO: pareciera ser que los identificadores no se están guardando,
	// en la tabla de símbolos, en el orden en que el se los agrega...
	// TODO: testeo offsets?
	// Field b
	std::list<symtable_element>::iterator it = fields->begin();
	assert((*it).get_key() == std::string("b"));
	assert((*it).get_class() == symtable_element::T_VAR);
	assert((*it).get_type() == symtable_element::BOOLEAN);

	// Field x
	it++;
	assert(it->get_key() == std::string("x"));
	assert(it->get_class() == symtable_element::T_VAR);
	assert(it->get_type() == symtable_element::FLOAT);

	// Field o
	it++;
	assert(it->get_key() == std::string("o"));
	assert(it->get_class() == symtable_element::T_ARRAY);
	assert(it->get_type() == symtable_element::INTEGER);
	assert(it->get_dimension() == 1);

	// Field m
	it++;
	assert(it->get_key() == std::string("m"));
	assert(it->get_class() == symtable_element::T_VAR);
	assert(it->get_type() == symtable_element::INTEGER);

	// Field n
	it++;
	assert(it->get_key() == std::string("n"));
	assert(it->get_class() == symtable_element::T_VAR);
	assert(it->get_type() == symtable_element::INTEGER);

	// In class2...
	class_program = symtable->get(std::string("class2"));
	fields =  class_program->get_class_fields();

	assert(fields->size() == 1);
	// TODO: testeo offsets?
	// Field obj
	it = fields->begin();
	assert((*it).get_key() == std::string("obj"));
	assert((*it).get_class() == symtable_element::T_OBJ);
	assert((*it).get_type() == symtable_element::ID);
	assert(*(*it).get_class_type() == std::string("class1"));

	// Local variables
	// TODO: testear declaración de arreglos
	test_program = 	"class class1 { int m;"
									"float y;"
									"boolean b2;"
					"}"
					"class class2 { void method() {"
										"int n;"
										"float x;"
										"boolean b;"
										"class1 obj;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v2;

	translate(v2, test_program);

	translation = v2.get_inst_list();
	//assert(translation->size() == 2);

	// Method's label
	instructions_list::iterator inst = translation->begin();

	// Enter procedure
	inst++;

	// Initialization of the variable n.
	inst++;

	address_pointer dest = new_name_address(std::string("n"));
	address_pointer orig = new_integer_constant(integer_initial_value);

	assert(is_copy(*inst, dest, orig));
	// TODO: borrar dest y orig

	// Initialization of the variable x.
	inst++;

	dest = new_name_address(std::string("x"));
	orig = new_float_constant(float_initial_value);

	assert(is_copy(*inst, dest, orig));

	// Initialization of the variable b.
	inst++;

	dest = new_name_address(std::string("b"));
	orig = new_boolean_constant(boolean_initial_value);

	assert(is_copy(*inst, dest, orig));

	// Initialization of the variable obj.
	// First field: b2
	// TODO: los atributos de la clase se almacenan en orden inverso.
	// TODO: actualmente estamos deinifiendo como addresses a cosas de la forma
	// id_instance "." nombre de atributo
	inst++;

	dest = new_name_address(std::string("obj.b2"));
	orig = new_boolean_constant(boolean_initial_value);

	assert(is_copy(*inst, dest, orig));

	// Second field: y
	inst++;

	dest = new_name_address(std::string("obj.y"));
	orig = new_float_constant(float_initial_value);

	assert(is_copy(*inst, dest, orig));

	// Third field: m
	inst++;

	dest = new_name_address(std::string("obj.m"));
	orig = new_integer_constant(integer_initial_value);

	assert(is_copy(*inst, dest, orig));


	std::cout << "OK. " << std::endl;
}

/* To test only a method's signature and enter procedure.
 * */
void test_method_decl(){
	std::cout << "4) Translation of a method declaration:";

	std::string test_program = "class class1 {"
								"}"
								"class class2 { void method(int m, class1 obj) {"
													"int n;"
													"float x;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	// TODO: testear size
	//assert(translation->size() == 2);

	// Method's label
	instructions_list::iterator it = translation->begin();
	std::string aux("class2.method");
	assert(is_label(*it, aux));

	// Enter instruction
	it++;
	assert(is_enter_procedure(*it, integer_width+float_width));

	// Test data collected.
	intermediate_symtable *symtable = v1.get_symtable();
	symtable_element *class1 = symtable->get(std::string("class2"));
	std::list<symtable_element> *fields =  class1->get_class_fields();

	assert(fields->size() == 1);
	// TODO: testeo offsets?
	// Method "method"
	std::list<symtable_element>::iterator method = fields->begin();

	assert((*method).get_class() == symtable_element::T_FUNCTION);

	std::list<symtable_element> *params = (*method).get_func_params();
	std::list<symtable_element>::iterator param =  params->begin();

	// TODO: los parámetros también me los está guardando, en la tabla, en el
	// orden inverso en el que son guardados.
	// Parameter obj
	assert((*param).get_class() == symtable_element::T_OBJ);
	assert((*param).get_key() == std::string("obj"));
	assert((*param).get_type() == symtable_element::ID);
	assert(*(*param).get_class_type() == std::string("class1"));

	// Parameter m
	param++;
	assert((*param).get_class() == symtable_element::T_VAR);
	assert((*param).get_key() == std::string("m"));
	assert((*param).get_type() == symtable_element::INTEGER);

	std::cout << "OK. " << std::endl;
}

void test_assignment_stm(){
	std::cout << "5) Translation of assignment statement:";

	std::string test_program = "class class1 { void method() {"
													"int n;"
													"n = 1;"
													"n += 1;"
													"n -= 1;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	// Method's label
	instructions_list::iterator it = translation->begin();

	// Enter procedure
	it++;

	// Variable initialization
	it++;

	// Assignment statement
	it++;
	address_pointer dest = new_name_address(std::string("n"));
	address_pointer constant = new_integer_constant(1);
	assert(is_copy(*it, dest, constant));

	// Plus assignment statement
	it++;
	assert(is_binary_assignment(*it,
							 	dest,
							 	dest,
							 	constant,
							 	quad_oper::PLUS));

	// Minus assignment statement
	it++;
	assert(is_binary_assignment(*it,
								dest,
								dest,
								constant,
								quad_oper::MINUS));

	std::cout << "OK. " << std::endl;
}

void test_method_call_statement(){
	std::cout << "6) Method call as statement:";

	std::string test_program = "class class1 { void method1() {"
												"}\n"
												"void method2(int x, int y) {"
													"method1();"
												"}\n"
								"}"
								"class class2 { void method3() {"
													"class1 obj;"
													"int x,y;"
													"obj.method2(x, y);"
												"}\n"
											"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	// Class 1

	// Method1's label
	instructions_list::iterator it = translation->begin();

	// Enter procedure
	it++;

	// Method2's label
	it++;

	// Enter procedure
	it++;

	// Param this
	it++;
	address_pointer this_param = new_name_address(std::string("this")); // TODO: borrar...
	assert(is_parameter_inst(*it, this_param));

	// Method call
	it++;
	address_pointer label = new_label_address(std::string("class1.method1"));
	assert(is_procedure_call(*it, label, 1));

	// Class 2

	// Method3's label
	it++;

	// Enter procedure
	it++;

	// Variable x
	it++;

	// Variable y
	it++;

	// Param this
	it++;
	this_param = new_name_address(std::string("obj"));
	assert(is_parameter_inst(*it, this_param));

	// Param x
	it++;
	this_param = new_name_address(std::string("x"));
	assert(is_parameter_inst(*it, this_param));

	// Param y
	it++;
	this_param = new_name_address(std::string("y"));
	assert(is_parameter_inst(*it, this_param));

	// Method call
	it++;
	label = new_label_address(std::string("class1.method2"));
	assert(is_procedure_call(*it, label, 3));

	std::cout << "OK. " << std::endl;
}

void test_if_statement(){
	std::cout << "7) Conditional statement:";

	std::string test_program = "class class1 { void method1() {"
													"int x;"
													"if (true) x = 1; else x = 2;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	// Class 1.

	// Method1's label.
	instructions_list::iterator it = translation->begin();

	// Enter procedure.
	it++;

	// Variable x.
	it++;


	// Conditional jump to the else branch.
	it++;
	address_pointer guard = new_boolean_constant(true);
	std::string label_else("L1");
	assert(is_conditional_jump_inst(*it, guard,label_else,quad_oper::IFFALSE));

	// If-then body.
	it++;
	address_pointer x = new_name_address("x");
	address_pointer constant = new_integer_constant(1);
	assert(is_copy(*it, x, constant));

	// Unconditional jump to the last part of the translation.
	it++;
	std::string label_end("L2");
	assert(is_unconditional_jump_inst(*it, label_end));

	// Label for the "else" branch.
	it++;
	assert(is_label(*it, label_else));

	// "else" body.
	it++;
	address_pointer constant2 = new_integer_constant(2);
	assert(is_copy(*it, x, constant2));

	// Label for the ending.
	it++;
	assert(is_label(*it, label_end));

	std::cout << "OK. " << std::endl;
}

void test_inter_code_gen_visitor(){
	std::cout << "\nTesting intermediate code generation:" << std::endl;
	test_three_address_code();
	test_class_decl();
	test_field_decl();
	test_method_decl();
	test_assignment_stm();
	test_method_call_statement();
	test_if_statement();
}
