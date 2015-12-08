#include "test_inter_code_gen_visitor.h"

// COMPI's parser's interface and global variables.
extern program_pointer ast;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

void translate(inter_code_gen_visitor& v, std::string program){
	YY_BUFFER_STATE program_buffer = yy_scan_string(program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	ast->accept(v);

	yy_delete_buffer(program_buffer);
}

// COMPI's parser's interface and global variables.
extern void translate_ir_code(std::string program);
extern instructions_list *ir_code;

void test_three_address_code(){
	std::cout << "1) Three-address code's representation-related procedures: ";

	address_pointer add1 = new_boolean_constant(boolean_initial_value);
	address_pointer add2 = new_boolean_constant(boolean_initial_value);

	/* TODO : borrar add1 y add2 */
	assert(are_equal_address_pointers(add1, add2));

	add1 = new_integer_constant(integer_initial_value);
	add2 = new_integer_constant(integer_initial_value);

	assert(are_equal_address_pointers(add1, add2));

	add1 = new_float_constant(float_initial_value);
	add2 = new_float_constant(float_initial_value);

	assert(are_equal_address_pointers(add1, add2));

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

	std::string ir_program = "main.main:\n"
							 "enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	// In the intermediate_symtable, just data about the classes.
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

	// Check ir code.
	std::string ir_program = "main.main:\n"
							 "enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

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
	ir_program = "class2.method:\n"
				 "enter "+std::to_string(reference_width +
						 	 	 	 	 integer_width*2 + float_width*2 +
						 	 	 	 	 boolean_width*2)+"\n"
				 "n = "+std::to_string(integer_initial_value)+"\n"
				 "x = "+std::to_string(float_initial_value)+"\n"
				 "b = "+BOOL_STR(boolean_initial_value)+"\n"
				 "obj.b2 = "+BOOL_STR(boolean_initial_value)+"\n"
				 "obj.y = "+std::to_string(float_initial_value)+"\n"
				 "obj.m = "+std::to_string(integer_initial_value)+"\n"
				 "main.main:\n"
				 "enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

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

	std::string ir_program = "class2.method:\n"
				"enter " + std::to_string(reference_width + integer_width +
									integer_width +
									float_width) + "\n" +
				"n = " + std::to_string(integer_initial_value) +
				"x = " + std::to_string(float_initial_value) +
				"main.main:\n" +
				"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

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

	// Parameter "this"
	param++;
	assert((*param).get_class() == symtable_element::T_OBJ);
	assert((*param).get_key() == std::string("this"));
	assert((*param).get_type() == symtable_element::ID);
	assert(*(*param).get_class_type() == std::string("class2"));

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

	std::string ir_program = "class1.method:\n"
							"enter "+std::to_string(reference_width + integer_width) + "\n"
							"n = " + std::to_string(integer_initial_value) + "\n"
							"n = 1\n"
							"n = n + 1\n"
							"n = n - 1"
							"main.main:"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

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
	std::string ir_program = "class1.method1:\n"
							"enter "+std::to_string(reference_width) + "\n"
							"class1.method2:\n"
							"enter " + std::to_string(reference_width + 2*integer_width) + "\n"
							"param this\n"
							"call class1.method1,1\n"
							"class2.method3:\n"
							"enter " + std::to_string(reference_width + 2*integer_width) + "\n"
							"x = "+std::to_string(integer_initial_value) + "\n"
							"y = "+std::to_string(integer_initial_value) + "\n"
							"param obj\n"
							"param x\n"
							"param y\n"
							"call class1.method2,3\n"
							"main.main:\n"
							"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

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

	std::string ir_program = "class1.method1:\n"
							"enter " +
								std::to_string(reference_width + integer_width) + "\n"
							"x = " + std::to_string(integer_initial_value) + "\n"
							"ifFalse true goto L1\n"
							"x = 1\n"
							"goto L2\n"
							"L1:\n"
							"x = 2\n"
							"L2:\n"
							"main.main:\n"
							"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_for_statement(){
	std::cout << "8) Loop \"for\":";

	std::string test_program = "class class1 { void method1(int y) {"
													"y = 1;"
													"for x = 1 , 2 y += 1;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter " +
							std::to_string(reference_width + integer_width) + "\n"
							"y = 1\n"
							"x = 1\n"
							"L1:\n"
							"if 2 < x goto L2\n"
							"y = y + 1\n"
							"x = x + 1\n"
							"goto L1\n"
							"L2:\n"
							"main.main:\n"
							"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_while_statement(){
	std::cout << "9) Loop \"while\":";

	std::string test_program = "class class1 { void method1(int y) {"
													"y = 1;"
													"while true y += 1;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter " +
								std::to_string(reference_width + integer_width) + "\n"
							"y = 1\n"
							"L1:\n"
							"ifFalse true goto L2\n"
							"y = y + 1\n"
							"goto L1\n"
							"L2:\n"
							"main.main:\n"
							"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_return_statement(){
	std::cout << "10) Statement \"return\":";

	std::string test_program = "class class1 { void method1() {"
													"return 1;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
								"enter " + std::to_string(reference_width) + "\n"
								"return 1\n"
								"main.main:\n"
								"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_break_statement(){
	std::cout << "11) Statement \"break\":";

	std::string test_program = "class class1 { void method1() {"
													"while true break;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter " + std::to_string(reference_width) + "\n"
							"L1:\n"
							"ifFalse true goto L2\n"
							"goto L2\n"
							"goto L1\n"
							"L2:\n"
							"main.main:\n"
							"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_continue_statement(){
	std::cout << "12) Statement \"continue\":";

	std::string test_program = "class class1 { void method1() {"
													"while true continue;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter " + std::to_string(reference_width) + "\n"
							"L1:\n"
							"ifFalse true goto L2\n"
							"goto L1\n"
							"goto L1\n"
							"L2:\n"
							"main.main:\n"
							"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_skip_statement(){
	std::cout << "13) Statement \"skip\":";

	std::string test_program = "class class1 { void method1() {"
													"while true {; continue;}"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter " + std::to_string(reference_width) + "\n"
							"L1:\n"
							"ifFalse true goto L2\n"
							"goto L1\n"
							"goto L1\n"
							"L2:\n"
							"main.main:\n"
							"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_binary_operation_expr(){
	std::cout << "14) Binary operations:";

	/////////////////////////////////
	// PLUS
	/////////////////////////////////
	std::string test_program = "class class1 { void method1(int x) {"
													"x = 1 + 2;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	// TODO: hay que pedir que el nombre de las temporales sea otro, ya que
	// t-0 se presta a confusión. Como tenemos que tener un espacio de nombres
	// privado, quizás podríamos emplear algún símbolo especial para identificarlas
	// como $?
	std::string ir_program = "class1.method1:\n"
							"enter " + std::to_string(reference_width + integer_width) + "\n"
							"t-0 = 1 + 2\n"
							"x = t-0\n"
							"main.main:\n"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	/*assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	instructions_list::iterator it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	address_pointer left_operand = new_integer_constant(1);
	address_pointer right_operand = new_integer_constant(2);
	address_pointer temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::PLUS));

	// Copy.
	it++;
	address_pointer dest = new_name_address(std::string("x"));
	assert(is_copy(*it, dest, temporal));*/

	/////////////////////////////////
	// MINUS
	/////////////////////////////////
	/*test_program = "class class1 { void method1(int x) {"
										"x = 1 - 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v2;

	translate(v2, test_program);

	translation = v2.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::MINUS));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// TIMES
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 * 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v3;

	translate(v3, test_program);

	translation = v3.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::TIMES));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// DIVIDE
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 / 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v4;

	translate(v4, test_program);

	translation = v4.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::DIVIDE));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// MOD
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 % 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v5;

	translate(v5, test_program);

	translation = v5.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::MOD));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// LESS
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 < 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v6;

	translate(v6, test_program);

	translation = v6.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::LESS));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// LESS EQUAL
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 <= 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v7;

	translate(v7, test_program);

	translation = v7.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::LESS_EQUAL));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// GREATER
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 > 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v8;

	translate(v8, test_program);

	translation = v8.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::GREATER));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// GREATER OR EQUAL
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 >= 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v9;

	translate(v9, test_program);

	translation = v9.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::GREATER_EQUAL));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// EQUAL
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 == 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v10;

	translate(v10, test_program);

	translation = v10.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::EQUAL));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));

	/////////////////////////////////
	// DISTINCT
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 != 2;"
									"}\n"
					"}"
					"class main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v11;

	translate(v11, test_program);

	translation = v11.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	it = translation->begin();

	// Enter procedure.
	it++;

	// Binary assignment: because we don't know the temporal variable created to
	// hold the result, we refer to the one in *it.
	it++;
	temporal = (*it)->result;
	assert(is_binary_assignment(*it, temporal,
								left_operand,
								right_operand,
								quad_oper::DISTINCT));

	// Copy.
	it++;
	assert(is_copy(*it, dest, temporal));*/

	std::cout << "OK. " << std::endl;
}

void test_negate_expr(){
	std::cout << "15) Logic negation:";

	std::string test_program = "class class1 { void method1(bool x) {"
													"x = ! true;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	instructions_list::iterator it = translation->begin();

	// Enter procedure.
	it++;

	// Negate expr.
	it++;
	address_pointer boolean = new_boolean_constant(true);
	address_pointer temporal = (*it)->result;
	assert(is_unary_assignment(*it, temporal, boolean, quad_oper::NEGATION));

	// Copy.
	it++;
	address_pointer dest = new_name_address(std::string("x"));
	assert(is_copy(*it, dest, temporal));

	std::cout << "OK. " << std::endl;
}

void test_negative_expr(){
	std::cout << "16) Negative:";

	std::string test_program = "class class1 { void method1(int x) {"
													"x = -1;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	assert(translation->size() == 6);

	// Class 1.

	// Method1's label.
	instructions_list::iterator it = translation->begin();

	// Enter procedure.
	it++;

	// Negate expr.
	it++;
	address_pointer constant = new_integer_constant(1);
	address_pointer temporal = (*it)->result;
	assert(is_unary_assignment(*it, temporal, constant, quad_oper::NEGATIVE));

	// Copy.
	it++;
	address_pointer dest = new_name_address(std::string("x"));
	assert(is_copy(*it, dest, temporal));

	std::cout << "OK. " << std::endl;
}

void test_parentheses_expr(){
	std::cout << "17) Expressions between parentheses:";

	std::string test_program = "class class1 { void method1(int x) {"
													"x = (1 + 2) + 3;"
												"}\n"
								"}"
								"class main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	assert(translation->size() == 7);

	// Class 1.

	// Method1's label.
	instructions_list::iterator it = translation->begin();

	// Enter procedure.
	it++;

	// Expression between parentheses.
	it++;
	address_pointer first_operand = new_integer_constant(1);
	address_pointer second_operand = new_integer_constant(2);
	address_pointer first_temporal = (*it)->result;
	assert(is_binary_assignment(*it, first_temporal,
								first_operand,
								second_operand,
								quad_oper::PLUS));

	// Second addition.
	it++;
	address_pointer third_operand = new_integer_constant(3);
	address_pointer second_temporal = (*it)->result;
	assert(is_binary_assignment(*it, second_temporal,
								first_temporal,
								third_operand,
								quad_oper::PLUS));

	// Copy.
	it++;
	address_pointer dest = new_name_address(std::string("x"));
	assert(is_copy(*it, dest, second_temporal));

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
	test_for_statement();
	test_while_statement();
	test_return_statement();
	test_break_statement();
	test_continue_statement();
	test_skip_statement();
	/*test_binary_operation_expr();
	test_negate_expr();
	test_negative_expr();
	test_parentheses_expr();*/
}
