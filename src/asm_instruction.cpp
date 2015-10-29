#include "asm_instruction.h"

/******************************************************************
 * Constructors of special kinds of operands.
 ******************************************************************/

operand_pointer new_register_operand(register_id reg){
	operand_pointer op = operand_pointer(new operand);
	op->op_addr = operand_addressing::REGISTER;
	op->value.reg = reg;

	return op;
}

operand_pointer new_immediate_integer_operand(int imm_int){
	operand_pointer op = operand_pointer(new operand);
	op->op_addr = operand_addressing::IMMEDIATE;
	op->value.imm.val.ival = imm_int;
	op->value.imm.imm_op_type = immediate_op_type::INTEGER;

	return op;
}

operand_pointer new_immediate_float_operand(float imm_float){
	operand_pointer op = operand_pointer(new operand);
	op->op_addr = operand_addressing::IMMEDIATE;
	op->value.imm.val.fval = imm_float;
	op->value.imm.imm_op_type = immediate_op_type::FLOAT;

	return op;
}

operand_pointer new_immediate_boolean_operand(bool imm_boolean){
	operand_pointer op = operand_pointer(new operand);
	op->op_addr = operand_addressing::IMMEDIATE;
	op->value.imm.val.bval = imm_boolean;
	op->value.imm.imm_op_type = immediate_op_type::BOOLEAN;

	return op;
}

operand_pointer new_memory_operand(unsigned int offset,
									register_id base,
									register_id index,
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

/******************************************************************
 * Constructors of instructions.
 ******************************************************************/
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
	inst->is_signed = false;

	return inst;

}

asm_instruction_pointer new_mul_instruction(operand_pointer source,
											operand_pointer destination,
											data_type ops_type,
											bool is_signed){

	#ifdef __DEBUG
		assert(destination->op_addr == operand_addressing::REGISTER ||
			   destination->op_addr == operand_addressing::MEMORY);
	#endif

	// TODO: precondición: source y destination se refieren a valores del
	// mismo tipo
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);

	if(is_signed){
		inst->op = operation::IMUL;
	}
	else{
		// {not is_signed}
		inst->op = operation::MUL;
	}
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->source = source;
	inst->destination = destination;
	inst->is_signed = is_signed;

	return inst;
}

asm_instruction_pointer new_add_instruction(operand_pointer source,
								operand_pointer destination,
								data_type ops_type){

	#ifdef __DEBUG
		std::cout << "New add instruction." << std::endl;
		assert(destination->op_addr == operand_addressing::REGISTER ||
			   destination->op_addr == operand_addressing::MEMORY);
	#endif

	// TODO: precondición: source y destination se refieren a valores del
	// mismo tipo
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::ADD;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->source = source;
	inst->destination = destination;
	inst->is_signed = true; // TODO: no hay distinción entre add signed y unsigned?

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
	inst->is_signed = true; // TODO: no hay distinción entre add signed y unsigned?

	return inst;
}

asm_instruction_pointer new_div_instruction(operand_pointer dividend,
								data_type ops_type, bool is_signed){

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);

	if(is_signed){
		inst->op = operation::IDIV;
	}
	else{
		// {not is_signed}
		inst->op = operation::DIV;
	}
	inst->ops_type = ops_type;
	inst->destination = dividend;
	inst->is_signed = is_signed;

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
	inst->is_signed = false;

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
	inst->is_signed = true;

	return inst;
}

asm_instruction_pointer new_not_instruction(operand_pointer destination,
											data_type ops_type){

	#ifdef __DEBUG
		assert(destination->op_addr == operand_addressing::REGISTER ||
			   destination->op_addr == operand_addressing::MEMORY);
	#endif

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::NOT;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = ops_type;
	inst->destination = destination;
	inst->is_signed = false;

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
	inst->is_signed = false;

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
	inst->is_signed = false;

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
	inst->is_signed = false;

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
	inst->is_signed = false;

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
	inst->is_signed = false;

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
	inst->is_signed = false;

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
	inst->is_signed = false;

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
	inst->is_signed = false;

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
	inst->is_signed = false;

	return inst;
}

asm_instruction_pointer new_leave_instruction(){

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::LEAVE;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	inst->is_signed = false;

	return inst;
}

asm_instruction_pointer new_ret_instruction(){

	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::RET;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	inst->is_signed = false;

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
	inst->is_signed = false;

	return inst;
}

// TODO: nesting_level? ver vol. 1 del manual de prog. de x86_64.
asm_instruction_pointer new_enter_instruction(operand_pointer stack_space,
											operand_pointer nesting_level){
	asm_instruction_pointer inst = asm_instruction_pointer(new asm_instruction);
	inst->op = operation::ENTER;
	// TODO: cómo determino el tipo de información de source?
	inst->ops_type = data_type::NONE;
	// TODO: notar que está más piola esta forma de definir el constructor
	// de instrucciones, sin exigirle al usuario que llame a
	// new_++++_operand para crear el tipo correcto de operando...
	inst->source = stack_space;
	inst->destination = nesting_level;
	inst->is_signed = false;

	return inst;
}

/******************************************************************
 * Printing
 ******************************************************************/

std::string print_register(register_id reg){
	std::string ret;

	switch(reg){
		// TODO: que hacemos en este caso?
		case register_id::NONE:
			ret = std::string("0");
			break;

		case register_id::RAX:
			ret = std::string("%rax");
			break;

		case register_id::RBX:
			ret = std::string("%rbx");
			break;

		case register_id::RCX:
			ret = std::string("%rcx");
			break;

		case register_id::RDX:
			ret = std::string("%rdx");
			break;

		case register_id::RSI:
			ret = std::string("%rsi");
			break;

		case register_id::RDI:
			ret = std::string("%rdi");
			break;

		case register_id::RBP:
			ret = std::string("%rbp");
			break;

		case register_id::RSP:
			ret = std::string("%rsp");
			break;

		case register_id::R8:
			ret = std::string("%r8");
			break;

		case register_id::R9:
			ret = std::string("%r9");
			break;

		case register_id::R10:
			ret = std::string("%r10");
			break;

		case register_id::R11:
			ret = std::string("%r11");
			break;

		case register_id::R12:
			ret = std::string("%r12");
			break;

		case register_id::R13:
			ret = std::string("%r13");
			break;

		case register_id::R14:
			ret = std::string("%r14");
			break;

		case register_id::R15:
			ret = std::string("%r15");
			break;

		case register_id::EAX:
			ret = std::string("%eax");
			break;

		case register_id::EBX:
			ret = std::string("%ebx");
			break;

		case register_id::ECX:
			ret = std::string("%ecx");
			break;

		case register_id::EDX:
			ret = std::string("%edx");
			break;

		case register_id::R8D:
			ret = std::string("%r8d");
			break;

		case register_id::R9D:
			ret = std::string("%r9d");
			break;

		case register_id::R10D:
			ret = std::string("%r10d");
			break;

		case register_id::R11D:
			ret = std::string("%r11d");
			break;

		case register_id::R12D:
			ret = std::string("%r12d");
			break;

		case register_id::R13D:
			ret = std::string("%r13d");
			break;

		case register_id::R14D:
			ret = std::string("%r14d");
			break;

		case register_id::R15D:
			ret = std::string("%r15d");
			break;

		case register_id::R8B:
			ret = std::string("%r8b");
			break;

		case register_id::R9B:
			ret = std::string("%r9b");
			break;

		case register_id::R10B:
			ret = std::string("%r10b");
			break;

		case register_id::R11B:
			ret = std::string("%r11b");
			break;

		case register_id::R12B:
			ret = std::string("%r12b");
			break;

		case register_id::R13B:
			ret = std::string("%r13b");
			break;

		case register_id::R14B:
			ret = std::string("%r14b");
			break;

		case register_id::R15B:
			ret = std::string("%r15b");
			break;
	}

	return ret;
}
// TODO: hay diferencie entre la sintaxis de intel y de las demás, en el caso
// de los operandos?
std::string print_operand_intel_syntax(operand_pointer operand){

	#ifdef __DEBUG
		std::cout << "print_operand_intel_syntax: ";
	#endif

	std::string ret;

	switch(operand->op_addr){
		case operand_addressing::IMMEDIATE:{
			switch(operand->value.imm.imm_op_type){
				case immediate_op_type::FLOAT:
					// Single-precision float
					// TODO:?
					break;

				case immediate_op_type::INTEGER:
					ret =  std::string("$" + std::to_string(operand->value.imm.val.ival));
					break;

				case immediate_op_type::BOOLEAN:{
					if(operand->value.imm.val.bval){
						ret = std::string("$1");
					}
					else{
						// {not operand->value.imm.bval}
						ret = std::string("$0");
					}
					break;
				}
			}

			break;
		}

		case operand_addressing::REGISTER:{
			ret = print_register(operand->value.reg);
			break;
		}

		case operand_addressing::MEMORY:{
			// TODO: qué pasa cuando alguno de los campos no esta presente?
			ret = std::to_string(operand->value.mem.offset)
				+ std::string("(") + print_register(operand->value.mem.base)
				+ std::string(", ") + print_register(operand->value.mem.index)
				+ std::string(", ") + std::to_string(operand->value.mem.scale)
				+ std::string(")");
			break;
		}

		default:
			// {operand->op_addr == None}
			// It's a label.
			ret = std::string("." + *operand->value.label);
	}

	#ifdef __DEBUG
		std::cout << ret << std::endl;
	#endif

	return ret;
}

std::string obtain_data_type(data_type d_type){
	std::string ret;

	switch(d_type){
		case data_type::B:
			ret = "b";
			break;

		case data_type::W:
			ret = "w";
			break;

		case data_type::L:
			ret = "l";
			break;

		case data_type::Q:
			ret = "q";
			break;

		case data_type::S:
			ret = "s";
			break;

		case data_type::D:
			ret = "d";
			break;

		default:
			// {d_type == data_type::NONE}
			ret = "";
	}

	return ret;
}

// TODO: parte de la semántica de este procedimiento consiste en no lidiar
// con cuestiones de indentación, cosa que sí va a hacer print_intel_syntax
std::string print_binary_op_intel_syntax(asm_instruction_pointer instruction){
	std::string ret, prefix;

	#ifdef __DEBUG
		std::cout << "print_binary_op_intel_syntax: ";
	#endif

	switch(instruction->op){
		case operation::ADD:
			prefix = std::string("add" + obtain_data_type(instruction->ops_type));
			break;

		case operation::IMUL:
			// TODO: sólo hay imul?
			prefix = std::string("imul" + obtain_data_type(instruction->ops_type));
			break;

		case operation::SUB:
			prefix = std::string("sub" + obtain_data_type(instruction->ops_type));
			break;

		case operation::MOV:
			prefix = std::string("mov" + obtain_data_type(instruction->ops_type));
			break;

		case operation::ENTER:
			prefix = "enter";
			break;

		case operation::CMP:
			prefix = std::string("cmp" + obtain_data_type(instruction->ops_type));
			break;
	}

	ret =  prefix + " " + print_operand_intel_syntax(instruction->source) + " , "
			+ print_operand_intel_syntax(instruction->destination);

	#ifdef __DEBUG
		std::cout << ret << std::endl;
	#endif

	return ret;
}

std::string print_unary_op_intel_syntax(asm_instruction_pointer instruction){
	std::string ret, prefix;

	#ifdef __DEBUG
		std::cout << "print_unary_op_intel_syntax: ";
	#endif

	switch(instruction->op){
		case operation::NOT:
			prefix = std::string("not" + obtain_data_type(instruction->ops_type));
			break;

		case operation::NEG:
			prefix = std::string("neg" + obtain_data_type(instruction->ops_type));
			break;

		case operation::IDIV:
			if(instruction->is_signed){
				prefix = std::string("idiv" + obtain_data_type(instruction->ops_type));;
			}
			else{
				// {not instruction->is_signed}
				prefix = std::string("div" + obtain_data_type(instruction->ops_type));;
			}
			break;

		case operation::JMP:
			prefix = "jmp";
			break;

		case operation::JE:
			prefix = "je";
			break;

		case operation::JNE:
			prefix = "jne";
			break;

		case operation::JL:
			prefix = "jl";
			break;

		case operation::JLE:
			prefix = "jle";
			break;

		case operation::JG:
			prefix = "jg";
			break;

		case operation::JGE:
			prefix = "jge";
			break;

		case operation::CALL:
			prefix = "call";
			break;

		case operation::LABEL:
			prefix = "";
			// TODO: esto me agrega un espacio al comienzo del label...
			break;
	}

	ret = prefix + " " + print_operand_intel_syntax(instruction->destination);

	#ifdef __DEBUG
		std::cout << ret << std::endl;
	#endif

	return ret;
}

std::string print_intel_syntax(asm_instruction_pointer instruction){
	std::string ret("");

	switch(instruction->op){
		// TODO: podriamos pasarle el tipo de instruccion a estos metodos
		// que son llamados para realizar efectivamente la traduccion?
		case operation::ADD:
			ret += "\t" + print_binary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::SUB:
			ret += "\t" + print_binary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::IMUL:
			ret += "\t" + print_binary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::IDIV:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::DIV:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::NEG:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::SAR:
			break;

		case operation::NOT:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::MOV:
			ret += "\t" + print_binary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::JMP:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::JE:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::JNE:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::JL:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::JLE:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::JG:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::JGE:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::CALL:
			ret += "\t" + print_unary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::LEAVE:
			ret += "\tleave\n";
			break;

		case operation::RET:
			ret += "\tret\n";
			break;

		case operation::ENTER:
			ret += "\t" + print_binary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::CMP:
			ret += "\t" + print_binary_op_intel_syntax(instruction) + "\n";
			break;

		case operation::LABEL:
			ret += print_unary_op_intel_syntax(instruction) + "\n";
			break;
	}

	return ret;
}


/******************************************************************
 * Debugging procedures.
 ******************************************************************/
bool are_equal_operands(operand_pointer op1, operand_pointer op2){
	bool ret = false;

	if (op1->op_addr == op2->op_addr){
		switch(op1->op_addr){
			case operand_addressing::IMMEDIATE:{
				switch(op1->value.imm.imm_op_type){
					case immediate_op_type::INTEGER:
						ret = op2->value.imm.imm_op_type == immediate_op_type::INTEGER &&
							op1->value.imm.val.ival == op2->value.imm.val.ival;
						break;

					case immediate_op_type::FLOAT:
						ret = op2->value.imm.imm_op_type == immediate_op_type::FLOAT &&
							op1->value.imm.val.fval == op2->value.imm.val.fval;
						break;

					case immediate_op_type::BOOLEAN:
						ret = op2->value.imm.imm_op_type == immediate_op_type::BOOLEAN &&
							op1->value.imm.val.bval == op2->value.imm.val.bval;
						break;
				}
				break;
			}

			case operand_addressing::REGISTER:{
				ret = op1->value.reg == op2->value.reg;
				break;
			}

			case operand_addressing::MEMORY:{
				ret = op1->value.mem.offset == op2->value.mem.offset &&
					op1->value.mem.base == op2->value.mem.base &&
					op1->value.mem.scale == op2->value.mem.scale &&
					op1->value.mem.index == op2->value.mem.index;
				break;
			}

			default:
				// {op1->op_addr == operand_addressing::NONE}
				// TODO: estos sí o sí implica que es una etiqueta?
				ret = *op1->value.label == *op2->value.label;
		}
	}

	return ret;
}

bool is_mov_instruction(asm_instruction_pointer inst, operand_pointer source,
						operand_pointer destination, data_type ops_type){

	return	inst->op == operation::MOV &&
			inst->ops_type == ops_type &&
			are_equal_operands(inst->source, source) &&
			are_equal_operands(inst->destination, destination);
}

bool is_add_instruction(asm_instruction_pointer inst, operand_pointer source,
						operand_pointer destination, data_type ops_type){

	return	inst->op == operation::ADD &&
			inst->ops_type == ops_type &&
			are_equal_operands(inst->source, source) &&
			are_equal_operands(inst->destination, destination);
}

bool is_mul_instruction(asm_instruction_pointer inst, operand_pointer source,
						operand_pointer destination, data_type ops_type,
						bool is_signed){
	bool cmp_sign;

	if(is_signed){
		cmp_sign = inst->op == operation::IMUL;
	}
	else{
		// {not is_signed}
		cmp_sign = inst->op == operation::MUL;
	}

	return	cmp_sign &&
			inst->ops_type == ops_type &&
			are_equal_operands(inst->source, source) &&
			are_equal_operands(inst->destination, destination);
}

bool is_div_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						data_type ops_type,
						bool is_signed){
	bool cmp_sign;

	if(is_signed){
		cmp_sign = inst->op == operation::IDIV;
	}
	else{
		// {not is_signed}
		cmp_sign = inst->op == operation::DIV;
	}

	return	cmp_sign &&
			inst->ops_type == ops_type &&
			are_equal_operands(inst->destination, source);
}

bool is_neg_instruction(asm_instruction_pointer inst,
						operand_pointer destination,
						data_type ops_type){
	return	inst->op == operation::NEG &&
			inst->ops_type == ops_type &&
			are_equal_operands(inst->destination, destination);
}

bool is_not_instruction(asm_instruction_pointer inst,
						operand_pointer destination,
						data_type ops_type){
	return	inst->op == operation::NOT &&
			inst->ops_type == ops_type &&
			are_equal_operands(inst->destination, destination);
}

bool is_shr_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination,
						data_type ops_type){
	return	inst->op == operation::SHR &&
			inst->ops_type == ops_type &&
			are_equal_operands(inst->source, source) &&
			are_equal_operands(inst->destination, destination);
}

bool is_jmp_instruction(asm_instruction_pointer inst,
						std::string label){
	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::JMP &&
			are_equal_operands(inst->destination, aux);

	return	ret;
}

bool is_je_instruction(asm_instruction_pointer inst,
						std::string label){
	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::JE &&
			are_equal_operands(inst->destination, aux);

	return	ret;
}

bool is_jne_instruction(asm_instruction_pointer inst,
						std::string label){
	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::JNE &&
			are_equal_operands(inst->destination, aux);

	return	ret;
}

bool is_jl_instruction(asm_instruction_pointer inst,
						std::string label){
	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::JL &&
			are_equal_operands(inst->destination, aux);

	return	ret;
}

bool is_jle_instruction(asm_instruction_pointer inst,
						std::string label){
	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::JLE &&
			are_equal_operands(inst->destination, aux);

	return	ret;
}

bool is_jg_instruction(asm_instruction_pointer inst,
						std::string label){
	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::JG &&
			are_equal_operands(inst->destination, aux);

	return	ret;
}

bool is_jge_instruction(asm_instruction_pointer inst,
						std::string label){
	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::JGE &&
			are_equal_operands(inst->destination, aux);

	return	ret;
}

bool is_call_instruction(asm_instruction_pointer inst,
						std::string label){
	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::CALL &&
			are_equal_operands(inst->destination, aux);

	return	ret;
}

bool is_leave_instruction(asm_instruction_pointer inst){
	return	inst->op == operation::LEAVE;
}

bool is_ret_instruction(asm_instruction_pointer inst){
	return	inst->op == operation::RET;
}

bool is_cmp_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination,
						data_type ops_type){

	return	inst->op == operation::CMP &&
			inst->ops_type == ops_type &&
			are_equal_operands(inst->source, source) &&
			are_equal_operands(inst->destination, destination);
}

bool is_enter_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination){

	return	inst->op == operation::ENTER &&
			are_equal_operands(inst->source, source) &&
			are_equal_operands(inst->destination, destination);
}

bool is_label_instruction(asm_instruction_pointer inst,
						std::string label){

	operand_pointer aux = new_label_operand(label);
	bool ret = inst->op == operation::LABEL &&
			are_equal_operands(inst->destination, aux);

	return ret;
}
