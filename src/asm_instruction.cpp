#include "asm_instruction.h"

// Constructors of special kinds of operands.
operand_pointer new_register_operand(register_id reg){
	operand_pointer op = operand_pointer(new operand);
	op->op_addr = operand_addressing::REGISTER;
	op->value.reg = reg;

	return op;
}

operand_pointer new_immediate_int_operand(int imm_int){
	operand_pointer op = operand_pointer(new operand);
	op->op_addr = operand_addressing::IMMEDIATE;
	op->value.imm.ival = imm_int;

	return op;
}

operand_pointer new_memory_operand(unsigned int offset, unsigned int base,
								   unsigned int index,
								   unsigned int scale){

	operand_pointer op = operand_pointer(new operand);
	op->op_addr = operand_addressing::MEMORY;
	op->value.mem.offset = offset;
	op->value.mem.base = base;
	op->value.mem.index = index;
	op->value.mem.scale = scale;

	return op;
}

operand_pointer new_label_operand(std::string label){

	operand_pointer op = operand_pointer(new operand);
	op->op_addr = operand_addressing::NONE;
	op->value.label = new std::string(label); // TODO: liberar!

	return op;
}

// Constructors of instructions.
asm_instruction_pointer new_mov_instruction(operand_pointer source,
								operand_pointer destination,
								data_type ops_type){

	#ifdef __DEBUG
		assert(destination->op_addr == operand_addressing::REGISTER ||
			   destination->op_addr == operand_addressing::MEMORY);
	#endif

	// TODO: precondición: source y destination se refieren a valores del
	// mismo tipo.
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::MOV;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->source = source;
	inst->destination = destination;

	return inst;

}

asm_instruction_pointer new_mul_instruction(operand_pointer source,
											operand_pointer destination,
											data_type ops_type){

	#ifdef __DEBUG
		assert(destination->op_addr == operand_addressing::REGISTER ||
			   destination->op_addr == operand_addressing::MEMORY);
	#endif

	// TODO: precondición: source y destination se refieren a valores del
	// mismo tipo
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::MUL;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->source = source;
	inst->destination = destination;

	return inst;
}

asm_instruction_pointer new_add_instruction(operand_pointer source,
								operand_pointer destination,
								data_type ops_type){

	#ifdef __DEBUG
		std::cout << "New add instruction." << std::endl;
		//assert(destination->op_addr == operand_addressing::REGISTER ||
		//	   destination->op_addr == operand_addressing::MEMORY);
	#endif

	// TODO: precondición: source y destination se refieren a valores del
	// mismo tipo
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::ADD;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->source = source;
	inst->destination = destination;

	return inst;
}

asm_instruction_pointer new_sub_instruction(operand_pointer source,
								operand_pointer destination,
								data_type ops_type){

	#ifdef __DEBUG
		assert(destination->op_addr == operand_addressing::REGISTER ||
			   destination->op_addr == operand_addressing::MEMORY);
	#endif

	// TODO: precondición: source y destination se refieren a valores del
	// mismo tipo
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::SUB;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->source = source;
	inst->destination = destination;

	return inst;
}

asm_instruction_pointer new_div_instruction(operand_pointer dividend,
								data_type ops_type){

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::DIV;
	inst->ops_type = ops_type;
	inst->source = dividend;

	return inst;
}

asm_instruction_pointer new_shr_instruction(operand_pointer imm,
											operand_pointer destination,
											data_type ops_type){

	#ifdef __DEBUG
		assert(imm->op_addr == operand_addressing::IMMEDIATE &&
			   (destination->op_addr == operand_addressing::REGISTER ||
			    destination->op_addr == operand_addressing::MEMORY));
	#endif

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::SHR;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->source = imm;
	inst->destination = destination;

	return inst;
}

asm_instruction_pointer new_neg_instruction(operand_pointer destination,
											data_type ops_type){

	#ifdef __DEBUG
		assert(destination->op_addr == operand_addressing::REGISTER ||
			   destination->op_addr == operand_addressing::MEMORY);
	#endif

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::NEG;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->destination = destination;

	return inst;
}

asm_instruction_pointer new_jmp_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::JMP;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_je_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::JE;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_jne_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::JNE;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_jl_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::JL;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_jle_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::JLE;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_jg_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::JG;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_jge_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::JGE;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_call_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::CALL;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_cmp_instruction(operand_pointer op_1,
											operand_pointer op_2,
											data_type ops_type){

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::CMP;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->source = op_1;
	inst->destination = op_2;

	return inst;
}

asm_instruction_pointer new_leave_instruction(){

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::LEAVE;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;

	return inst;
}

asm_instruction_pointer new_ret_instruction(){

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::RET;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;

	return inst;
}

asm_instruction_pointer new_label_instruction(std::string label){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::LABEL;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_label_operand(label);

	return inst;
}

asm_instruction_pointer new_enter_instruction(int n){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::ENTER;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->destination = new_immediate_int_operand(n);

	return inst;
}
