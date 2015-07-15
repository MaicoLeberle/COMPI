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

void test_class_decl(){
	std::cout << "1) Translation of a class declaration:";

	// One class declaration...
	std::string test_program = "class Program {}\0\0";

	inter_code_gen_visitor v1;
	translate(v1, test_program);

	const instructions_list *translation = v1.get_inst_list();
	// One instruction...
	assert(translation->size() == 1);
	// which is: label "Program".
	std::string aux("Program");
	assert(is_label((*translation)[0], aux));

	// Two declarations of classes...
	test_program = "class Program {}\n "
					"class main {}\0\0";

	inter_code_gen_visitor v2;
	translate(v2, test_program);

	translation = v2.get_inst_list();
	assert(translation->size() == 2);
	aux = std::string("Program");
	assert(is_label((*translation)[0], aux));
	aux = std::string("main");
	assert(is_label((*translation)[1], aux));

	std::cout << "OK. " << std::endl;
}

void test_method_decl(){
	std::cout << "2) Translation of a method declaration:";

	std::string test_program = "class Program { void method() {} }\0\0";
	inter_code_gen_visitor v1;

	translate(v1, test_program);

	const instructions_list *translation = v1.get_inst_list();
	assert(translation->size() == 2);
	std::string aux("Program");
	assert(is_label((*translation)[0], aux));
	aux = std::string("Program.method");
	assert(is_label((*translation)[1], aux));

	std::cout << "OK. " << std::endl;
}

void test_inter_code_gen_visitor(){
	std::cout << "\nTesting intermediate code generation:" << std::endl;
	test_class_decl();
	test_method_decl();
}
