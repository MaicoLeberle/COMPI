#include "test_asm_parser.h"


extern asm_instructions_list *asm_code;
/*typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int asmparse();
extern YY_BUFFER_STATE asm_scan_string(const char * str);
extern void asm_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void asm_delete_buffer(YY_BUFFER_STATE buffer);

void translate_asm_code(std::string program){
	YY_BUFFER_STATE program_buffer = asm_scan_string(program.c_str());
	asm_switch_to_buffer(program_buffer);

	asmparse();

	asm_delete_buffer(program_buffer);
}*/
void translate_asm_code(std::string program);

extern void translate_asm_code(std::string program);

void test_arithmetic_inst_translation(){
	std::cout << "1) Translation of arithmetic instructions:";

	// ADD.
	translate_asm_code(std::string("add $3, %r10"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	asm_instructions_list::iterator it = asm_code->begin();
	assert(is_add_instruction(*it,
			new_immediate_integer_operand(3),
			new_register_operand(register_id::R10),
			data_type::L));

	// MUL.
	// Signed mul.
	translate_asm_code(std::string("imul $3, %r10"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_mul_instruction(*it,
			new_immediate_integer_operand(3),
			new_register_operand(register_id::R10),
			data_type::L,
			true));

	// Unsigned mul.
	translate_asm_code(std::string("mul $3, %r10"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_mul_instruction(*it,
			new_immediate_integer_operand(3),
			new_register_operand(register_id::R10),
			data_type::L,
			false));

	// DIV
	// Signed div.
	translate_asm_code(std::string("idiv $3"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_div_instruction(*it,
			new_immediate_integer_operand(3),
			data_type::L,
			true));

	// Unsigned div.
	translate_asm_code(std::string("div $3"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_div_instruction(*it,
			new_immediate_integer_operand(3),
			data_type::L,
			false));


	// Neg.
	translate_asm_code(std::string("neg %r10"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_neg_instruction(*it,
							new_register_operand(register_id::R10),
							data_type::L));
	std::cout << "OK. " << std::endl;
}

void test_logic_inst_translation(){
	std::cout << "2) Translation of logic instructions:";

	// NOT
	translate_asm_code(std::string("not %r10"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	asm_instructions_list::iterator it = asm_code->begin();
	assert(is_not_instruction(*it,
			new_register_operand(register_id::R10),
			data_type::L));

	// SHR
	translate_asm_code(std::string("shr $3, %r10"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_shr_instruction(*it,
							new_immediate_integer_operand(3),
							new_register_operand(register_id::R10),
							data_type::L));

	std::cout << "OK. " << std::endl;
}

void test_data_transfer_inst_translation(){
	std::cout << "3) Translation of data transfer instructions:";

	// NOT
	translate_asm_code(std::string("mov $3, %r10"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	asm_instructions_list::iterator it = asm_code->begin();
	assert(is_mov_instruction(*it,
			new_immediate_integer_operand(3),
			new_register_operand(register_id::R10),
			data_type::L));

	std::cout << "OK. " << std::endl;
}

void test_control_transfer_inst_translation(){
	std::cout << "4) Translation of control transfer instructions:";

	// JMP.
	translate_asm_code(std::string("jmp label"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	asm_instructions_list::iterator it = asm_code->begin();
	assert(is_jmp_instruction(*it,
							std::string("label")));

	// JE.
	translate_asm_code(std::string("je label"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_je_instruction(*it,
							std::string("label")));

	// JNE.
	translate_asm_code(std::string("jne label"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_jne_instruction(*it,
							std::string("label")));

	// JL.
	translate_asm_code(std::string("jl label"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_jl_instruction(*it,
							std::string("label")));

	// JLE.
	translate_asm_code(std::string("jle label"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_jle_instruction(*it,
							std::string("label")));

	// JG.
	translate_asm_code(std::string("jg label"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_jg_instruction(*it,
							std::string("label")));

	// JGE.
	translate_asm_code(std::string("jge label"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_jge_instruction(*it,
							std::string("label")));

	// CALL.
	translate_asm_code(std::string("call label"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_call_instruction(*it,
							std::string("label")));

	// LEAVE.
	translate_asm_code(std::string("leave"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_leave_instruction(*it));

	// RET.
	translate_asm_code(std::string("ret"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_ret_instruction(*it));

	std::cout << "OK. " << std::endl;
}

void test_data_comparison_inst_translation(){
	std::cout << "5) Translation of data comparison instructions:";

	// CMP.
	translate_asm_code(std::string("cmp %r9, %r10"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	asm_instructions_list::iterator it = asm_code->begin();
	assert(is_cmp_instruction(*it,
							new_register_operand(register_id::R9),
							new_register_operand(register_id::R10),
							data_type::L));

	std::cout << "OK. " << std::endl;
}

void test_misc_inst_translation(){
	std::cout << "6) Translation of misc. instructions:";

	// ENTER.
	translate_asm_code(std::string("enter $1, $1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	asm_instructions_list::iterator it = asm_code->begin();
	assert(is_enter_instruction(*it,
							new_immediate_integer_operand(1),
							new_immediate_integer_operand(1)));

	// LABEL.
	translate_asm_code(std::string("label:"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(asm_code->size() == 1);
	it = asm_code->begin();
	assert(is_label_instruction(*it,
								std::string("label")));

	std::cout << "OK. " << std::endl;
}

void test_asm_parser(){
	std::cout << "\nTesting asm code's parser:" << std::endl;
	test_arithmetic_inst_translation();
	test_logic_inst_translation();
	test_data_transfer_inst_translation();
	test_control_transfer_inst_translation();
	test_data_comparison_inst_translation();
	test_misc_inst_translation();
}

