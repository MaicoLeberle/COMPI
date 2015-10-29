#ifndef ASM_INSTRUCTION_H
#define ASM_INSTRUCTION_H

#include <memory> // std::shared_ptr
#include <string> // std::string
#include <vector> // std::vector
#include <cassert> // assert
#include <iostream> // cout

/******************************************************************
 * Types to represent ASM instructions' operands..
 ******************************************************************/

enum class register_id {
	// Special value to denote the absence of a register.
	NONE,
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
	EDX,
	R8D,
	R9D,
	R10D,
	R11D,
	R12D,
	R13D,
	R14D,
	R15D,
	// 8-bits registers
	R8B,
	R9B,
	R10B,
	R11B,
	R12B,
	R13B,
	R14B,
	R15B,
};

enum class operand_addressing {
	IMMEDIATE, // Immediate value
	REGISTER, // Register
	MEMORY,   // Memory value
	NONE	  // For labels or other kind of data
};

// To indicate which field of the union imm is actually valid.
enum class immediate_op_type {
	INTEGER,
	FLOAT,
	BOOLEAN
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
		// TODO: ver página 16 del Vol. 1 del manual de prog. de x86_64
		struct {
			unsigned int offset;
			register_id base;
			register_id index;
			unsigned int scale;
		} mem;

		// Register representation.
		register_id reg;

		// Immediate value representation.
		struct{
			immediate_op_type imm_op_type;
			union {
				// TODO: coloco sólo estos campos, ya que son los únicos tipos
				// de constantes que tenemos en nuestro lenguaje,pero no tienen relación
				// directa con el contenido de type...
				// TODO: en este campo imm no tengo información sobre el tipo del
				// operando. Por otro lado, el enum data_type no distingue entre
				// enteros y booleanos. Será que la representación de los mismos
				// no varía?
				int ival;
				float fval;
				bool bval;
			} val;
		}imm;

		// Label representation
		std::string *label;
	} value;
};

typedef std::shared_ptr<operand> operand_pointer;

/******************************************************************
 * Types to represent ASM instructions.
 ******************************************************************/

enum class operation {
	// Arithmetic
	ADD,
	// TODO: sólo utilizamos la versión con signo de los operadores?
	IMUL,
	MUL,
	// TODO: sólo utilizamos la versión con signo de los operadores?
	IDIV,
	DIV,
	SUB,
	NEG,
	// TODO: para que usamos esta instruccion?
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
	bool is_signed = false;
};

typedef std::shared_ptr<asm_instruction> asm_instruction_pointer;

class asm_instructions_list : public std::vector<asm_instruction_pointer> {};

/******************************************************************
 * Constructors of special kinds of operands.
 ******************************************************************/
operand_pointer new_register_operand(register_id);

operand_pointer new_immediate_integer_operand(int);
operand_pointer new_immediate_float_operand(float);
// TODO: donde usamos esto?
operand_pointer new_immediate_boolean_operand(bool);

operand_pointer new_memory_operand(unsigned int offset,
								   register_id base,
								   register_id index,
								   unsigned int scale);

operand_pointer new_label_operand(std::string label);

/******************************************************************
 * Constructors of instructions.
 ******************************************************************/
asm_instruction_pointer new_mov_instruction(operand_pointer source,
											operand_pointer destination,
											data_type ops_type);
asm_instruction_pointer new_shr_instruction(operand_pointer imm,
											operand_pointer destination,
											data_type ops_type);
asm_instruction_pointer new_div_instruction(operand_pointer, data_type, bool);

asm_instruction_pointer new_mul_instruction(operand_pointer source,
								operand_pointer destination,
								data_type ops_type,
								bool is_signed);

asm_instruction_pointer new_add_instruction(operand_pointer source,
								operand_pointer destination,
								data_type ops_type);

asm_instruction_pointer new_sub_instruction(operand_pointer source,
								operand_pointer destination,
								data_type ops_type);

asm_instruction_pointer new_neg_instruction(operand_pointer destination,
											data_type ops_type);

asm_instruction_pointer new_not_instruction(operand_pointer destination,
											data_type ops_type);

asm_instruction_pointer new_cmp_instruction(operand_pointer, operand_pointer,
											data_type);

// TODO: del documento x86-64-arqu.guide.pdf: Jump unconditionally to target,
// which is specified as a memory location (for example, a label).
// Aqui solo consideramos una etiqueta.
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
asm_instruction_pointer new_enter_instruction(operand_pointer stack_space,
												operand_pointer nesting_level);

/******************************************************************
 * Printing
 ******************************************************************/
std::string print_operand_intel_syntax(operand_pointer);
std::string print_binary_op_intel_syntax(asm_instruction_pointer);
std::string print_unary_op_intel_syntax(asm_instruction_pointer);
std::string print_intel_syntax(asm_instruction_pointer);

/******************************************************************
 * Debugging procedures.
 ******************************************************************/
bool is_mov_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination,
						data_type ops_type);

bool is_add_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination,
						data_type ops_type);

bool is_mul_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination,
						data_type ops_type,
						bool is_signed);

bool is_div_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						data_type ops_type,
						bool is_signed);

bool is_neg_instruction(asm_instruction_pointer inst,
						operand_pointer destination,
						data_type ops_type);

bool is_shr_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination,
						data_type ops_type);

bool is_not_instruction(asm_instruction_pointer inst,
						operand_pointer destination,
						data_type ops_type);

bool is_jmp_instruction(asm_instruction_pointer inst,
						std::string label);

bool is_je_instruction(asm_instruction_pointer inst,
						std::string label);

bool is_jne_instruction(asm_instruction_pointer inst,
						std::string label);

bool is_jl_instruction(asm_instruction_pointer inst,
						std::string label);

bool is_jle_instruction(asm_instruction_pointer inst,
						std::string label);

bool is_jg_instruction(asm_instruction_pointer inst,
						std::string label);

bool is_jge_instruction(asm_instruction_pointer inst,
						std::string label);

bool is_call_instruction(asm_instruction_pointer inst,
						std::string label);

bool is_leave_instruction(asm_instruction_pointer inst);

bool is_ret_instruction(asm_instruction_pointer inst);

bool is_cmp_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination,
						data_type ops_type);

bool is_enter_instruction(asm_instruction_pointer inst,
						operand_pointer source,
						operand_pointer destination);

bool is_label_instruction(asm_instruction_pointer inst,
						std::string label);
#endif
