#include "test_asm_code_generator.h"

extern program_pointer ast;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

// TODO: colocar este procedimiento en una librería de utilidades para todas
// las suites de tests...

void trans(inter_code_gen_visitor& v, std::string program){

	YY_BUFFER_STATE program_buffer = yy_scan_string(program.c_str());
	yy_switch_to_buffer(program_buffer);

	yyparse();

	ast->accept(v);

	yy_delete_buffer(program_buffer);
}

void test_binary_assign(){
	std::cout << "1) Basic test: ";

	// x = y + z
	address_pointer right_operand = new_integer_constant(1);
	address_pointer left_operand = new_integer_constant(2);
	std::string var("x");
	address_pointer dest = new_name_address(var);

	instructions_list *ir_code = new instructions_list();
	ir_code->push_back(new_binary_assign(dest,
			   left_operand, right_operand, quad_oper::PLUS));

	ids_info *s_table = new ids_info();
	s_table->register_var(var, 0);

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	std::cout << "OK. " << std::endl;
}

void test_asm_code_generator(){
	std::cout << "\nTesting assembly code generation:" << std::endl;
	test_binary_assign();
}
