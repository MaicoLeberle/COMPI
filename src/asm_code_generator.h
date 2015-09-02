#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "inter_code_gen_visitor.h"
#include "asm_instruction.h"


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

class code_generator {
public:
	code_generator(instructions_list*, ids_info*);

	void translate_ir(void);

private:
	instructions_list *ir;
	asm_instructions_list *translation;
	ids_info *s_table;

	operand_pointer get_address(address_pointer address);
	void translate_binary_op(const quad_pointer&);
	void translate_unary_op(const quad_pointer&);
	void translate_copy(const quad_pointer&);
	void translate_indexed_copy_to(const quad_pointer&);
	void translate_indexed_copy_from(const quad_pointer&);
	void translate_unconditional_jump(const quad_pointer&);
	void translate_conditional_jump(const quad_pointer&);
	void translate_relational_jump(const quad_pointer&);
	void translate_parameter(const quad_pointer&);
	void translate_procedure_call(const quad_pointer&);
	void translate_function_call(const quad_pointer&);
	void translate_return(const quad_pointer&);
	void translate_label(const quad_pointer&);
	void translate_enter_procedure(const quad_pointer&);
};

#endif
