#include "test_asm_instruction.h"

void test_print_intel_syntax(){
	std::cout << "1) Printing with intel syntax: ";

	// add
	// addw -0(%rbp) , %rdi
	operand_pointer reg_rdi = new_register_operand(register_id::RDI);
	operand_pointer mem_0 = new_memory_operand(1,
												register_id::RBP,
												register_id::NONE,
												1);
	asm_instruction_pointer inst = new_add_instruction(mem_0, reg_rdi, data_type::W);
	// TODO: borrar contenido del heap!
	assert(print_intel_syntax(inst) == std::string("\taddw 1(%rbp, 0, 1) , %rdi\n"));

	// mul
	// imull -0(%rbp) , %rdi
	inst = new_mul_instruction(mem_0, reg_rdi, data_type::L, true);
	assert(print_intel_syntax(inst) == std::string("\timull 1(%rbp, 0, 1) , %rdi\n"));

	// div
	// idivb %rdi
	operand_pointer imm_1 = new_immediate_integer_operand(1);
	inst = new_div_instruction(imm_1, data_type::B, true);
	assert(print_intel_syntax(inst) == std::string("\tidivb $1\n"));

	// sub
	// subq $1 , %rdi
	inst = new_sub_instruction(imm_1, reg_rdi, data_type::Q);
	assert(print_intel_syntax(inst) == std::string("\tsubq $1 , %rdi\n"));

	// neg
	// negl %rdi
	inst = new_neg_instruction(reg_rdi, data_type::L);
	assert(print_intel_syntax(inst) == std::string("\tnegl %rdi\n"));

	// TODO: sar y shr?

	// not
	// notq %rdi
	inst = new_not_instruction(reg_rdi, data_type::L);
	assert(print_intel_syntax(inst) == std::string("\tnotl %rdi\n"));

	// mov
	// movw -0(%rbp) , %rdi
	inst = new_mov_instruction(mem_0, reg_rdi, data_type::W);
	assert(print_intel_syntax(inst) == std::string("\tmovw 1(%rbp, 0, 1) , %rdi\n"));

	// jmp
	// jmp .label
	std::string label("label");
	inst = new_jmp_instruction(label);
	assert(print_intel_syntax(inst) == std::string("\tjmp label\n"));

	// je
	// je .label
	inst = new_je_instruction(label);
	assert(print_intel_syntax(inst) == std::string("\tje label\n"));

	// jne
	// jne .label
	inst = new_jne_instruction(label);
	assert(print_intel_syntax(inst) == std::string("\tjne label\n"));

	// jl
	// jl .label
	inst = new_jl_instruction(label);
	assert(print_intel_syntax(inst) == std::string("\tjl label\n"));

	// jle
	// jle .label
	inst = new_jle_instruction(label);
	assert(print_intel_syntax(inst) == std::string("\tjle label\n"));

	// jg
	// jg .label
	inst = new_jg_instruction(label);
	assert(print_intel_syntax(inst) == std::string("\tjg label\n"));

	// jge
	// jge .label
	inst = new_jge_instruction(label);
	assert(print_intel_syntax(inst) == std::string("\tjge label\n"));

	// call
	// call .label
	// TODO: la etiqueta en este caso también comienza con .?
	inst = new_call_instruction(label);
	assert(print_intel_syntax(inst) == std::string("\tcall label\n"));

	// leave
	inst = new_leave_instruction();
	assert(print_intel_syntax(inst) == std::string("\tleave\n"));

	// ret
	inst = new_ret_instruction();
	assert(print_intel_syntax(inst) == std::string("\tret\n"));

	// enter
	// enter $1 , $1
	inst = new_enter_instruction(imm_1, imm_1);
	assert(print_intel_syntax(inst) == std::string("\tenter $1 , $1\n"));

	// label
	// label
	inst = new_label_instruction(label);
	assert(print_intel_syntax(inst) == std::string(" label\n"));

	// cmp
	// cmpq $1 , %rdi
	inst = new_cmp_instruction(imm_1, reg_rdi, data_type::Q);
	assert(print_intel_syntax(inst) == std::string("\tcmpq $1 , %rdi\n"));

	std::cout << "OK. " << std::endl;
}

void test_asm_instruction(){
	std::cout << "\nTesting assembly code representation:" << std::endl;
	test_print_intel_syntax();
}

