#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "inter_code_gen_visitor.h"
#include "asm_instruction.h"

/* Bibliography adopted:
 * [1] "Notes on x86-64 programming",
 * 		available at https://www.lri.fr/~filliatr/ens/compil/x86-64.pdf.
 *
 * [2] "AMD64 Architecture Programmer’s Manual Volume 1: Application
 * 		Programming", available at
 * 		http://developer.amd.com/wordpress/media/2012/10/24592_APM_v11.pdf.
 *
 * [3] "X86-64 Architecture Guide", available at
 * 		http://www.myoops.org/cocw/mit/NR/
 * 		rdonlyres/Electrical-Engineering-and-Computer-Science/
 * 		6-035Fall-2005/
 * 		A5797AF0-7976-4F22-90C3-AF15A2688988/0/x86_64_guide.pdf
 *
 * [4] "System V Application Binary Interface AMD64 Architecture Processor
 * 		Supplement", available at http://www.x86-64.org/documentation/abi.pdf
 * */

/* BINARY_ASSIGN x = y + z:
 * 		addw y,z (z = y+z) Los operandos son de tipo word?
 * 		mov[b|w|l|q] z,x (tendremos que ver el tipo de los operandos
 *		para determinar el tipo de instrucción?)
 * BINARY_ASSIGN x = y * z:
 * 		imulw y,z (z = z ∗ y (throws away high-order half of result; d must be a register))
 * 		mov[b|w|l|q] z,x
 * BINARY_ASSIGN x = y/z:
 * 		mov[b|w|l|q] y,%edx::%eax (puede ir así?)
 * 		idivl z (signed divide of %edx::%eax by z; quotient in %eax, remainder in %edx)
 * 		mov[b|w|l|q] %eax,x
 *
 * BINARY_ASSIGN x = y % z:
 * 		mov[b|w|l|q] y,%edx::%eax (puede ir así?)
 * 		idivl z (signed divide of %edx::%eax by z; quotient in %eax, remainder in %edx)
 * 		Podemos hacer lo mismo que con la división, y tomar el resto de %edx)
 *
 * 		mov[b|w|l|q] %edx,x
 *
 * BINARY_ASSIGN x = y - z:
 * 		subw y,z (z = z − y)
 * 		mov[b|w|l|q] z,x
 * BINARY_ASSIGN x = y and z:
 * 		and[b|w|l|q] y,z (z = z&y)
 * 		mov[b|w|l|q] z,x
 * UNARY_ASSIGN x = -y
 * 		negw y (y = −y)
 * 		mov[b|w|l|q] y,x
 * UNARY_ASSIGN x = not y
 * 		notw y (y =∼ y (bitwise complement))
 * 		mov[b|w|l|q] y,x
 *
 * COPY x = y
 *		mov[b|w|l|q] y,x (move s to d; tendremos que ver el tipo de los operandos
 *		para determinar el tipo de instrucción?)
 * INDEXED_COPY_TO,	// x[i] = y
 *		mov[b|w|l|q] y,x[i] (igual al anterior, solo que para representar x[i]
 *		tenemos que recordar la notación offset(base,index,scale) que representa
 *		la dirección (base + index × scale + offset) TODO: ver qué significa cada
 *		componente)
 * INDEXED_COPY_FROM,	// x = y[i]
 * 		mov[b|w|l|q] y[i], x (idem caso anterior)
 * UNCONDITIONAL_JUMP,	// goto L
 *		jmp L
 * CONDITIONAL_JUMP,	// if x goto L
 * 		TODO: primero debemos transformar el testeo de x en la comparación
 * 		con el valor 0 o signo del mismo. Usaremos estas instrucciones
 * 		cmp[b|w|l|q] s 2 ,s 1       set flags based on s 1 − s 2
 *		test[b|w|l|q] s 2 ,s 1      set flags based on s 1 & s 2 (logical and)
 *		Luego usar:
 *			jcc L  donde cc es la condición adecuada, para que salte si x es verdadero
 *
 * CONDITIONAL_JUMP,	// ifFalse x goto L
 * 		Idem, solo que saltamos si x es falso
 *
 * RELATIONAL_JUMP,	// if x < y goto L
 *		cmp[b|w|l|q] x , y (set flags based on y − x)
 *		jl L 				(salta si la comparación resultó less (signed <))
 *
 * RELATIONAL_JUMP,	// if x <= y goto L
 *		cmp[b|w|l|q] x , y (set flags based on y − x)
 *		jle L 				(salta si la comparación resultó less or equal (signed <=))
 * RELATIONAL_JUMP,	// if x > y goto L
 * 		cmp[b|w|l|q] x , y (set flags based on y − x)
 *		jg L 				(salta si la comparación resultó greater (signed >))
 * RELATIONAL_JUMP,	// if x >= y goto L
 * 		cmp[b|w|l|q] x , y (set flags based on y − x)
 *		jge L 				(salta si la comparación resultó greater or equal (signed >=)
 * RELATIONAL_JUMP,	// if x == y goto L
 * 		cmp[b|w|l|q] x , y (set flags based on y − x)
 *		je L 				(salta si la comparación resultó equal to zero)
 * RELATIONAL_JUMP,	// if x != y goto L
 * 		cmp[b|w|l|q] x , y (set flags based on y − x)
 *		jne L 				(salta si la comparación resultó not equal to zero)
 * PARAMETER,			// param x
 * 		Integer arguments (up to the first six) are passed in registers,
 * 		namely: %rdi, %rsi, %rdx, %rcx, %r8, %r9. Así es que la traducción
 * 		debería ser algo como:
 * 			mov[b|w|l|q] x,%rdi
 * 		para el primer parámetro, y así...
 * 		Observación: Floating arguments (up to 8) are passed in SSE registers
 * 		%xmm0, %xmm1, ..., %xmm7
 * 		Observación: Any arguments passed on the stack are pushed in reverse
 * 		(right-to-left) order.
 *
 * PROCEDURE_CALL,		// call p, n
 * 		call p				(n no hace falta?)
 * 		TODO: qué deberíamos hacer con esto:
 * 		Registers %rbx,%rbp,%r12,%r13,%r14,%r15 are callee-save; that is, the
 * 		callee is responsible for making sure that the values in these registers
 * 		are the same at exit as they were on entry.
 * FUNCTION_CALL,		// y = call p, n
 *		TODO: hace falta esto?
 * RETURN,				// return [y]
 * 		Observación: An integer-valued function returns its result in %rax. En
 * 		ese caso, la traducción de return debería ser:
 * 			mov y, %rax
 * 			leave				(The leave instruction sets %rsp to %rbp and then
 * 								pops the stack into %rbp, effectively popping the
 * 								entire current stack frame. It is nominally
 * 								intended to reverse the action of a previous
 * 								enter instruction.)
 * 			ret					(ret pops the top of stack into %rip, thus resuming
 * 								execution in the calling routine)
 *	LABEL,				// L: skip
 *		L:
 *	BEGIN_PROCEDURE n
 *		enter n
 * */

class asm_code_generator {
public:
	asm_code_generator(instructions_list*, ids_info*);

	void translate_ir(void);
	void print_translation_intel_syntax();
	asm_instructions_list* get_translation();

private:
	instructions_list *ir;
	// Auxiliary list of pushq instructions, that put parameters passed to a
	// procedure, into the stack, in the corresponding order.
	asm_instructions_list *stack_params;
	asm_instructions_list *translation;
	ids_info *s_table;
	register_id last_reg_used; // Last register used for integer parameters.
	bool using_registers;
	int params_in_registers; // Quantity of integer parameters put into registers.
	int offset;
    int nmbr_parameter; // Number of the method's parameter being analyzed.s 
	std::string actual_method_name; // Name of the method being translated, and
	std::string actual_class_name; // the class it belongs to.
	t_attributes actual_class_attributes;
	std::string last_label;
    bool contains_main_method; // Does the file compiled has a "main" method defined?
                                // Used to know when to add the corresponding 
                                // assembler's directives, to make the method 
                                // globally accesible.


    /* Translates a thress-address code's operand into an assembly's operand's.
     * PRE : {the offset of address, as indicated in this->s_table->get_offset
     *          is updated.}
     */ 
	operand_pointer convert_to_asm_operand(address_pointer address);
	void translate_binary_op(const quad_pointer&);
	void translate_unary_op(const quad_pointer&);
	void translate_copy(const quad_pointer&);
	void translate_indexed_copy_to(const quad_pointer&);
	void translate_indexed_copy_from(const quad_pointer&);
	void translate_unconditional_jump(const quad_pointer&);
	void translate_conditional_jump(const quad_pointer&);
	void translate_relational_jump(const quad_pointer&);

	/* Translates the 3-address "call" instruction. Also, if there are
	 * parameters to put into the stack, the procedure introduces the
	 * corresponding instructions, to save them in reverse order (the
	 * parameters that go into the stack, are managed internally).
	 *
	 * Calling sequence:
	 * 			_ Evaluate and pass parameters (according to the convention
	 * 			described in translate_parameter).
	 *
	 * 			_ Set %rax to the total number of floating point parameters
	 * 			passed to the function in vector registers ([4]).
	 *
	 * 			_ Save the return address (value of EIP, to point to the next
	 * 			instruction, after the calling sequence).
	 *
	 * 			_ Jump to the position where the prodedure's definition begins.
	 * 			(this last 2 steps are performed by the instruction
	 * 			"call procedure_label").
	 *
	 * PARAMETER: 3-address code instruction: call label, n
	 * */
	void translate_procedure_call(const quad_pointer&);
	void translate_function_call(const quad_pointer&);

	/* Translates a param instruction, following the C calling convention,
	 * adopted by linux and GNU tools for the x86-64 ([1] and [4]):
	 * 		_ Integer arguments (up to the first six) are passed in registers,
	 * 		namely: %rdi, %rsi, %rdx, %rcx, %r8, %r9.
	 *
	 * 		_ Floating arguments (up to 8) are passed in SSE registers
	 * 		%xmm0, %xmm1, ..., %xmm7
	 *
	 * 		_ Any arguments passed on the stack are pushed in reverse
	 * 		(right-to-left) order.
	 *
	 * PARAMETER: 3-address code instruction: param x
	 * */
	void translate_parameter(const quad_pointer&);

	/* Translates the 3-address code "return" statement, following the C
	 * calling convention, adopted by linux and GNU tools for the x86-64
	 * ([1] and [4]):
	 *		_ An integer value is returned into register rax or rdx. As we
	 *		do not use register rax for other purpose, we use it to return
	 *		integer values from procedures.
	 *
	 *		_ Sets %rsp to %rbp and then pops the stack into %rbp, effectively
	 *		popping the entire current stack frame (from [1], which is done by
	 *		a single "leave" instruction).
	 *
	 *		_ Pops the top of the stack into %rip (done by a single "ret"
	 *		instruction), thus resuming execution in the calling routine
	 *		(from [1]).
	 * */
	void translate_return(const quad_pointer&);

	void translate_label(const quad_pointer&);

	/*	Translates an "enter" instruction, whose purpose is to set the
	 *  stack frame of the called procedure. This involves [2]:
	 *  	_ Save, into the stack, the current value of rBP register.
	 *
	 *  	_ Store the value of the frame pointer into rBP:
	 *  	"The value of the rSP register at that moment is a frame pointer
	 *  	for the current procedure: positive
	 *  	offsets from this pointer give access to the parameters passed to
	 *  	the procedure, and negative offsets
	 *  	give access to the local variables which will be allocated later"
	 *  	from [2].
	 *
	 *  	_ Decrement the value of the rSP pointer, to allocate space
	 *  	for local variables used in the procedure.
	 *
	 *		_ If there are integer parameters into registers, take them into
	 *		the stack frame.
	 *		TODO: la razón, aparente, para realizar esto:
	 *		"Parameters send to function in registers rdi-r9 aren't preserved
	 *		in stack area ? What happens when child function is called ?", de
	 *		http://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64/
	 *
	 *  Notes:
	 *  	_ "the callee is responsible for preserving the value of registers
	 *  %rbp %rbx, and %r12-r15, as these registers are owned by the caller.",
	 *  from [3]. As we only use rBP, into every procedure, we just make sure
	 *  to save only the value of it.
	 *
	 *  	_ All the steps needed to set the stack frame, are performed by
	 *  the assembly's enter instruction [2].
	 *
	 *  	_ In COMPI, we don't have nested procedures. Thus, the second
	 *  parameter of assembly's enter instruction, is always 0.
	 *
	 *  PARAMETER: 3-address code instruction: enter x, where x is the number
	 *  of bytes to allocate into the stack frame, for the local variables
	 *  used in the procedure. */
	void translate_enter_procedure(const quad_pointer&);

	/* Returns the next register available, for integer parameters, following
	 * the order: %rdi, %rsi, %rdx, %rcx, %r8, %r9 or none (when there are no
	 * more registers available).
	 * Sets the attribute act_reg_av to the register selected*/
	register_id get_next_reg_av(register_id);

	bool allocate_integer_param(const operand_pointer& val, bool pass_address);

	bool is_attribute(std::string var_name);

    // Returns the width in bytes for an indicated three-address value's type. 
    int get_value_width(value_type);
};

#endif
