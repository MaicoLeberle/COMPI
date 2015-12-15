#include "test_inter_code_gen_visitor.h"

// COMPI's parser's interface and global variables.
extern program_pointer ast;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

void translate(inter_code_gen_visitor& v, std::string program){
	semantic_analysis sem_an_v;
	YY_BUFFER_STATE program_buffer = yy_scan_string(program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	// Semantic analysis.
	ast->accept(sem_an_v);

	// IR code generation.
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";

	inter_code_gen_visitor v1;
	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "Main.main:\n"
							 "enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	// In the intermediate_symtable, just data about the classes.
	intermediate_symtable *symtable = v1.get_symtable();

	assert(symtable->get(std::string("Program"))->get_class() ==
		   symtable_element::T_CLASS);

	assert(symtable->get(std::string("Main"))->get_class() ==
			   symtable_element::T_CLASS);

	// One empty main class declaration...
	test_program = "class Main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v2;
	translate(v2, test_program);

	translation = v2.get_inst_list();

	ir_program = "Main.main:\n"
						"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));
	std::cout << "OK. " << std::endl;
}

void test_field_decl(){
	std::cout << "3) Translation of a field declaration: ";

	// Attributes of an object.
	std::string test_program = "class class1 { int n,m,o[1];"
								"				float x;"
								"				boolean b;}"
								"class class2 { class1 obj; }"
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	// Check ir code.
	std::string ir_program = "Main.main:\n"
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
					"class Main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v2;

	translate(v2, test_program);

	// TODO: estoy hay que cambiarlo: la tabla de símbolos me tiene que
	// devolver los atributos en el orden que corresponden!
	translation = v2.get_inst_list();
	ir_program = "class2.method:\n"
				 "enter " + std::to_string(reference_width +
						 	 	 	 	 	 integer_width*2 +
						 	 	 	 	 	 float_width*2 +
						 	 	 	 	 	 boolean_width*2) + "\n"
				 "n = "+std::to_string(integer_initial_value)+"\n"
				 "x = "+std::to_string(float_initial_value)+"\n"
				 "b = "+BOOL_STR(boolean_initial_value)+"\n"
				 "obj[0] = "+BOOL_STR(boolean_initial_value)+"\n"
				 "obj[" + std::to_string(boolean_width) + "] = " +
				 	 	 	 	 	 	std::to_string(float_initial_value)+"\n"
				 "obj[" + std::to_string(float_width +
	 	 	 	 	 	 boolean_width) + "] = " +
	 	 	 	 	 	 	 	 	 std::to_string(integer_initial_value)+"\n"
				 "obj = &obj.b2\n"
				 "Main.main:\n"
				 "enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	// Referencing attributes.
	test_program = 	"class class1 { int n;"

									"void method() {"
										"n += 1;"
									"}\n"
					"}"
					"class Main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";

	inter_code_gen_visitor v3;

	translate(v3, test_program);

	translation = v3.get_inst_list();
	ir_program = "class1.method:\n"
				 "enter 0\n"
				 "n = n + 1\n"
				 "Main.main:\n"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class2.method:\n"
				"enter " + std::to_string(integer_width +
										float_width) + "\n" +
				"n = " + std::to_string(integer_initial_value) +
				"x = " + std::to_string(float_initial_value) +
				"Main.main:\n" +
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method:\n"
							"enter "+std::to_string(integer_width) + "\n"
							"n = " + std::to_string(integer_initial_value) + "\n"
							"n = 1\n"
							"n = n + 1\n"
							"n = n - 1"
							"Main.main:"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	std::string ir_program = "class1.method1:\n"
							"enter 0\n"
							"class1.method2:\n"
							"enter 0\n"
							"param this\n"
							"call class1.method1,1\n"
							"class2.method3:\n"
							"enter " + std::to_string(reference_width +
														2*integer_width) + "\n"
							"obj = &obj\n"
							"x = "+std::to_string(integer_initial_value) + "\n"
							"y = "+std::to_string(integer_initial_value) + "\n"
							"param obj\n"
							"param x\n"
							"param y\n"
							"call class1.method2, 3\n"
							"Main.main:\n"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter " +
								std::to_string(integer_width) + "\n"
							"x = " + std::to_string(integer_initial_value) + "\n"
							"ifFalse true goto L1\n"
							"x = 1\n"
							"goto L2\n"
							"L1:\n"
							"x = 2\n"
							"L2:\n"
							"Main.main:\n"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter 0\n"
							"y = 1\n"
							"x = 1\n"
							"L1:\n"
							"if 2 < x goto L2\n"
							"y = y + 1\n"
							"x = x + 1\n"
							"goto L1\n"
							"L2:\n"
							"Main.main:\n"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter 0\n"
							"y = 1\n"
							"L1:\n"
							"ifFalse true goto L2\n"
							"y = y + 1\n"
							"goto L1\n"
							"L2:\n"
							"Main.main:\n"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
								"enter 0\n"
								"return 1\n"
								"Main.main:\n"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter 0\n"
							"L1:\n"
							"ifFalse true goto L2\n"
							"goto L2\n"
							"goto L1\n"
							"L2:\n"
							"Main.main:\n"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
							"enter 0\n"
							"L1:\n"
							"ifFalse true goto L2\n"
							"goto L1\n"
							"goto L1\n"
							"L2:\n"
							"Main.main:\n"
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
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	// TODO: esto no está mal? no debería guardar en una temporal la guarda
	// del while?
	std::string ir_program = "class1.method1:\n"
							"enter 0\n"
							"L1:\n"
							"ifFalse true goto L2\n"
							"goto L1\n"
							"goto L1\n"
							"L2:\n"
							"Main.main:\n"
							"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_binary_operation(){
	std::cout << "14) Binary operations:";

	/////////////////////////////////
	// PLUS
	/////////////////////////////////
	std::string test_program = "class class1 { void method1(int x) {"
													"x = 1 + 2;"
												"}\n"
								"}"
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	std::string ir_program = "class1.method1:\n"
							"enter 4\n"
							"@t0 = 1 + 2\n"
							"x = @t0\n"
							"Main.main:\n"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	/////////////////////////////////
	// MINUS
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 - 2;"
									"}\n"
								"}"
					"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v2;

	translate(v2, test_program);

	translation = v2.get_inst_list();
	ir_program = "class1.method1:\n"
							"enter 4\n"
							"@t0 = 1 - 2\n"
							"x = @t0\n"
							"Main.main:\n"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));
	/////////////////////////////////
	// TIMES
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 * 2;"
									"}\n"
								"}"
					"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v3;

	translate(v3, test_program);

	translation = v3.get_inst_list();
	ir_program = "class1.method1:\n"
							"enter 4\n"
							"@t0 = 1 * 2\n"
							"x = @t0\n"
							"Main.main:\n"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	/////////////////////////////////
	// DIVIDE
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 / 2;"
									"}\n"
								"}"
					"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v4;

	translate(v4, test_program);

	translation = v4.get_inst_list();
	ir_program = "class1.method1:\n"
							"enter 4\n"
							"@t0 = 1 / 2\n"
							"x = @t0\n"
							"Main.main:\n"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	/////////////////////////////////
	// MOD
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = 1 % 2;"
									"}\n"
								"}"
					"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v5;

	translate(v5, test_program);

	translation = v5.get_inst_list();
	ir_program = "class1.method1:\n"
							"enter 4\n"
							"@t0 = 1 % 2\n"
							"x = @t0\n"
							"Main.main:\n"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_unary_operation(){
	std::cout << "15) Unary operations:";

	/////////////////////////////////
	// NEGATION
	/////////////////////////////////
	std::string test_program = "class class1 { void method1(bool x) {"
													"x = ! true;"
												"}\n"
								"}"
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	std::string ir_program = "class1.method1:\n"
							"enter 4\n"
							"@t0 = not true\n"
							"x = @t0\n"
							"Main.main:\n"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	/////////////////////////////////
	// NEGATIVE
	/////////////////////////////////
	test_program = "class class1 { void method1(int x) {"
										"x = -1;"
									"}\n"
					"}"
					"class Main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";
	inter_code_gen_visitor v2;

	translate(v2, test_program);

	translation = v2.get_inst_list();
	ir_program = "class1.method1:\n"
							"enter 4\n"
							"@t0 = -1\n"
							"x = @t0\n"
							"Main.main:\n"
							"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_parentheses_expr(){
	std::cout << "16) Expressions between parentheses:";

	// Two sub-expressions.
	std::string test_program = "class class1 { void method1(int x) {"
													"x = (1 + 2) + 3;"
												"}\n"
								"}"
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();
	std::string ir_program = "class1.method1:\n"
						"enter 8\n"
						"@t0 = 1 + 2\n"
						"@t1 = @t0 + 3\n"
						"x = @t1\n"
						"Main.main:\n"
						"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	// More than 2 sub-expressions
	test_program = "class class1 { void method1(int x) {"
										"x = ((((1 - 2) + 3) * 4) / 5);"
									"}\n"
					"}"
					"class Main {\n"
						"void main(){\n"
						"}\n"
					"}\0\0";
	inter_code_gen_visitor v2;

	translate(v2, test_program);

	translation = v2.get_inst_list();
	ir_program = "class1.method1:\n"
						"enter 16\n"
						"@t0 = 1 - 2\n"
						"@t1 = @t0 + 3\n"
						"@t2 = @t1 * 4\n"
						"@t3 = @t2 / 5\n"
						"x = @t3\n"
						"Main.main:\n"
						"enter 0";


	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

	std::cout << "OK. " << std::endl;
}

void test_array_decl(){
	std::cout << "17) Declaration of arrays:";

	std::string test_program = "class class1 { void method1() {"
													"int a[2];"
												"}\n"
								"}"
								"class Main {\n"
									"void main(){\n"
									"}\n"
								"}\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	instructions_list *translation = v1.get_inst_list();

	std::string ir_program = "class1.method1:\n"
								"enter 8\n"
								"a[0] = 0\n"
								"a[4] = 0\n"
								"Main.main:\n"
								"enter 0";

	translate_ir_code(ir_program);

	assert(are_equal_instructions_list(*translation, *ir_code));

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
	test_binary_operation();
	test_unary_operation();
	test_parentheses_expr();
	test_array_decl();
}

