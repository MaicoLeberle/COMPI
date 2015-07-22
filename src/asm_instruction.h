#ifndef ASM_INSTRUCTION_H
#define ASM_INSTRUCTION_H

enum class register_id {
	// 64-bits registers
	RAX,
	RBX,
	RCX,
	RDX,
	RSI,
	RDI,
	RBP,
	RSP,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	// 32-bits registers
	EAX,
	EBX,
	ECX,
	EDX
}

enum class operand_addressing {
	IMMEDIATE, // Immediate value
	REGISTER, // Register
	MEMORY,   // Memory value
	NONE	  // For labels or other kind of data
};

// Data types
enum class data_type {
	B, // Byte
	W, // Word
	L, // Double (or long) word
	Q, // Quad word
	S, // Single precision float
	D,  // Double precision float
	NONE // For labels or other kind of data
};

struct operand {
	operand_addressing op_addr;

	union {
		// Memory address representation. The effective address is
		// base + index × scale + offset.
		struct {
			unsigned int offset;
			unsigned int base;
			unsigned int index;
			unsigned int scale;
		} mem;

		// Register representation.
		register_id reg;

		// Immediate value representation.
		union {
			// TODO: coloco sólo estos campos, ya que son los únicos tipos
			// de constantes que tenemos en nuestro lenguaje,pero no tienen relación
			// directa con el contenido de type...
			int ival;
			float fval;
			bool bval;
		} imm;

		// Label representation
		std::string label;
	} value;
};

typedef std::shared_ptr<operand> operand_pointer;

enum class operation {
	// Arithmetic
	ADD,
	MUL,
	DIV,
	SUB,
	NEG,
	SAR, // sar[b|w|l|q] imm,d -> d = d>>imm (arithmetic right shift: the
		 // spaces are filled in such a way to preserve the sign of the number
		 // being slid)
	// Logic
	NOT,
	SHR, // shr[b|w|l|q] imm,d -> d = d>>imm (logical right shift: the spaces
		 // are always filled with zeros)
	// Data transfer
	MOV,
	// Control transfer
	JMP, // Unconditional jump
	JE,	 // Jump if equal
	JNE, // Jump if not equal
	JL,  // Jump if less
	JLE, // Jump if less or equal
	JG,  // Jump if greater
	JGE, // Jump if greater of equal
	CALL,
	LEAVE,
	RET,
	// Data comparison
	CMP, // CMP s1, s2 set flags based on s 1 − s 2
	// Misc
	ENTER,
	LABEL
};


// TODO: NOTA!, conviene dejarlo como struct para poder identificar más
// rápidamente el tipo de operacion, con un simple switch sobre op
struct asm_instruction {
	operation op;
	// TODO: asumo que a estas alturas, los operandos siempre tienen el
	// mismo tipo, entonces defino esa información en un único lugar.
	data_type ops_type;
	operand_pointer source = nullptr;
	operand_pointer destination = nullptr;
};

typedef std::shared_ptr<asm_instruction> asm_instruction_pointer;

class asm_instructions_list : public std::vector<asm_instruction_pointer> {};

// Constructors of special kinds of operands.
operand_pointer new_register_operand(register_id);
operand_pointer new_immediate_int_operand(int);
operand_pointer new_memory_operand(int offset, int base, int index, int scale);

// Constructors of instructions.
asm_instruction_pointer new_mov_instruction(address_pointer source,
											address_pointer destination,
											data_type ops_type);
asm_instruction_pointer new_shr_instruction(address_pointer imm,
											address_pointer destination,
											data_type ops_type);
asm_instruction new_div_instruction(address_pointer, data_type);
asm_instruction new_mul_instruction(address_pointer source,
								address_pointer destination,
								data_type ops_type);
asm_instruction_pointer new_add_instruction(address_pointer source,
								address_pointer destination,
								data_type ops_type);
asm_instruction_pointer new_sub_instruction(address_pointer source,
								address_pointer destination,
								data_type ops_type);
asm_instruction_pointer new_neg_instruction(address_pointer destination,
											data_type ops_type);
asm_instruction_pointer new_cmp_instruction(address_pointer, address_pointer,
											data_type);
asm_instruction_pointer new_jmp_instruction(std::string);
asm_instruction_pointer new_je_instruction(std::string);
asm_instruction_pointer new_jne_instruction(std::string);
asm_instruction_pointer new_jl_instruction(std::string);
asm_instruction_pointer new_jle_instruction(std::string);
asm_instruction_pointer new_jg_instruction(std::string);
asm_instruction_pointer new_jge_instruction(std::string);
asm_instruction_pointer new_call_instruction(std::string);
asm_instruction_pointer new_leave_instruction();
asm_instruction_pointer new_ret_instruction();
asm_instruction_pointer new_label_instruction(std::string);
asm_instruction_pointer new_enter_instruction(int);
#endif
