#include "test_asm_code_generator.h"

extern instructions_list *ir_code;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int irparse();
extern YY_BUFFER_STATE ir_scan_string(const char * str);
extern void ir_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void ir_delete_buffer(YY_BUFFER_STATE buffer);

void translate(std::string ir_program){
	YY_BUFFER_STATE program_buffer = ir_scan_string(ir_program.c_str());
	ir_switch_to_buffer(program_buffer);

	irparse();

	ir_delete_buffer(program_buffer);
}

void test_binary_assign(){
	std::cout << "1) Translation of binary assign: ";

	////////////////////////
	// PLUS
	////////////////////////

	// x = 2 + 1
	std::string var("x");
	/*instructions_list *ir_code = new instructions_list();
	ir_code->push_back(new_binary_assign(new_name_address(var),
										new_integer_constant(2),
										new_integer_constant(1),
										quad_oper::PLUS));*/
	translate(std::string("x = 2 + 1"));
	ids_info *s_table = new ids_info();
	s_table->register_var(var, 4);

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	assert(translation->size() == 3);

	// mov instruction
	asm_instructions_list::iterator it = translation->begin();
	operand_pointer reg_r8d = new_register_operand(register_id::R8D);
	operand_pointer mem_0 = new_memory_operand(0,
												register_id::NONE,
												register_id::NONE,
												1);
	operand_pointer imm_1 = new_immediate_integer_operand(1);
	operand_pointer imm_2 = new_immediate_integer_operand(2);
	assert(is_mov_instruction(*it, imm_2, reg_r8d, data_type::L));

	// add instruction
	it++;
	assert(is_add_instruction(*it, imm_1, reg_r8d, data_type::L));

	////////////////////////
	// MINUS
	////////////////////////
	// x = y - z
	ir_code = new instructions_list();
	ir_code->push_back(new_binary_assign(new_name_address(var),
										new_integer_constant(2),
										new_integer_constant(1),
										quad_oper::MINUS));

	s_table = new ids_info();
	s_table->register_var(var, 4);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	////////////////////////
	// TIMES
	////////////////////////
	// x = y * z
	ir_code = new instructions_list();
	ir_code->push_back(new_binary_assign(new_name_address(var),
										new_integer_constant(2),
										new_integer_constant(1),
										quad_oper::TIMES));

	s_table = new ids_info();
	s_table->register_var(var, 4);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	////////////////////////
	// DIVIDE
	////////////////////////
	// x = y * z
	ir_code = new instructions_list();
	ir_code->push_back(new_binary_assign(new_name_address(var),
										new_integer_constant(2),
										new_integer_constant(1),
										quad_oper::DIVIDE));

	s_table = new ids_info();
	s_table->register_var(var, 4);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	////////////////////////
	// MOD
	////////////////////////
	// x = y * z
	ir_code = new instructions_list();
	ir_code->push_back(new_binary_assign(new_name_address(var),
										new_integer_constant(2),
										new_integer_constant(1),
										quad_oper::MOD));

	s_table = new ids_info();
	s_table->register_var(var, 4);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	std::cout << "OK. " << std::endl;
}

void test_asm_code_generator(){
	std::cout << "\nTesting assembly code generation:" << std::endl;
	test_binary_assign();
}
