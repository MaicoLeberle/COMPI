#include "code_generator.h"

code_generator::code_generator(instructions_list *_ir) : ir(_ir) {
	translation = new asm_instructions_list();
}

void code_generator::translate_binary_op(const quad_pointer& instruction){

	switch(instruction->op){
		case TIMES:
			// BINARY_ASSIGN x = y * z:
			//		mov[b|w|l|q] z,register
			// 		imulw y,z (z = z ∗ y, z debe ser un registro)
			// 		mov[b|w|l|q] z,x (x = z)
			operand_pointer new_register; // TODO: cómo lo defino?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			data_type ops_type; // TODO: cómo determino el tipo de los operandos?
			// We use a new register to ensure that the second operand can be
			// the destination of the product.
			// TODO: en la generación de código intermedio, no nos aseguramos de
			// esto?
			translation->push_back(new_mov_instruction(z, new_register, ops_type));
			translation->push_back(new_mul_instruction(y, new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));

		case DIVIDE:
			// BINARY_ASSIGN x = y / z:
			//		mov[b|w|l|q] y,%edx::%eax (puede ir así?)
			// 		idivl z (signed divide of %edx::%eax by z; quotient in %eax, remainder in %edx)
			// 		mov[b|w|l|q] %eax,x
			operand_pointer edx_reg = new_register_operand(register_id::EDX);
			operand_pointer eax_reg = new_register_operand(register_id::EAX);
			data_type ops_type; // TODO: cómo determino el tipo de los operandos?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			// TODO: estoy moviendo los primeros 32 bits de y en eax?
			translation->push_back(new_mov_instruction(y, eax_reg, ops_type));
			// TODO: quizás deberíamos guardar "y" en algún registro, hacer corrimiento
			// de 32 posiciones hacia la derecha (para quedarnos con los 32 bits
			// de mayor peso), y guardar el resultado en EDX
			// TODO: definir aux_reg
			operand_pointer zero = new_immediate_int_operand(0);
			translation->push_back(new_mov_instruction(zero, edx_reg, ops_type));
			translation->push_back(new_div_instruction(z, ops_type));
			translation->push_back(new_mov_instruction(eax_reg, x, ops_type));

		case MOD:
			// BINARY_ASSIGN x = y / z:
			//		mov[b|w|l|q] y,%edx::%eax (puede ir así?)
			// 		idivl z (signed divide of %edx::%eax by z; quotient in %eax, remainder in %edx)
			// 		Podemos hacer lo mismo que con la división, y tomar el resto de %edx)
			//
			// 		mov[b|w|l|q] %edx,x
			operand_pointer edx_reg = new_register_operand(register_id::EDX);
			operand_pointer eax_reg = new_register_operand(register_id::EAX);
			data_type ops_type; // TODO: cómo determino el tipo de los operandos?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			// TODO: estoy moviendo los primeros 32 bits de y en eax?
			translation->push_back(new_mov_instruction(y, eax_reg, ops_type));
			// TODO: quizás deberíamos guardar "y" en algún registro, hacer corrimiento
			// de 32 posiciones hacia la derecha (para quedarnos con los 32 bits
			// de mayor peso), y guardar el resultado en EDX
			// TODO: definir aux_reg
			operand_pointer zero = new_immediate_int_operand(0);
			translation->push_back(new_mov_instruction(zero, edx_reg, ops_type));
			translation->push_back(new_div_instruction(z, ops_type));
			translation->push_back(new_mov_instruction(edx_reg, x, ops_type));

		case PLUS:
			// BINARY_ASSIGN x = y + z:
			//		addw y,z (z = y+z) Los operandos son de tipo word
			// 		mov[b|w|l|q] z,x (tendremos que ver el tipo de los operandos
			//		para determinar el tipo de instrucción?)
			operand_pointer new_register; // TODO: cómo lo defino?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			data_type ops_type; // TODO: cómo determino el tipo de los operandos?
			translation->push_back(new_mov_instruction(z, new_register, ops_type));
			translation->push_back(new_add_instruction(y, new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));

		case MINUS:
			// BINARY_ASSIGN x = y - z:
			//		subw y,z (z = z − y)
			//		mov[b|w|l|q] z,x
			operand_pointer new_register; // TODO: cómo lo defino?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			data_type ops_type; // TODO: cómo determino el tipo de los operandos?
			translation->push_back(new_mov_instruction(z, new_register, ops_type));
			translation->push_back(new_sub_instruction(y, new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));

		// TODO: está bien poner less, less_equal, etc aquí?
		case LESS:

		case LESS_EQUAL:

		case GREATER:

		case GREATER_EQUAL:

		case EQUAL:

		case DISTINCT:

		case AND:

		case OR:
	}
}

void code_generator::translate_unary_op(const quad_pointer& instruction){

	switch(instruction->oper){
		case NEGATIVE:
			// UNARY_ASSIGN x = -y
			// 		negw y (y = −y)
			// 		mov[b|w|l|q] y,x
			operand_pointer new_register; // TODO: cómo lo defino?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			data_type ops_type; // TODO: cómo determino el tipo de los operandos?
			// We use a new register to ensure that the second operand can be
			// the destination of the product.
			// TODO: en la generación de código intermedio, no nos aseguramos de
			// esto?
			translation->push_back(new_mov_instruction(y, new_register, ops_type));
			translation->push_back(new_neg_instruction(new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));

		case NEGATION:
			// UNARY_ASSIGN x = not y
			// notw y (y =∼ y (bitwise complement))
			// mov[b|w|l|q] y,x
			operand_pointer new_register; // TODO: cómo lo defino?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			data_type ops_type; // TODO: cómo determino el tipo de los operandos?
			// We use a new register to ensure that the second operand can be
			// the destination of the product.
			// TODO: en la generación de código intermedio, no nos aseguramos de
			// esto?
			translation->push_back(new_mov_instruction(y, new_register, ops_type));
			translation->push_back(new_neg_instruction(new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));
	}
}

void code_generator::translate_copy(const quad_pointer& instruction){
	// COPY x = y:
	// 		mov[b|w|l|q] y,x (move s to d; tendremos que ver el tipo de los operandos
	// 		para determinar el tipo de instrucción?)
	operand_pointer x = get_address(instruction->result);
	operand_pointer y = get_address(instruction->arg1);
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	// We use a new register to ensure that the second operand can be
	// the destination of the product.
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_mov_instruction(y, x, ops_type));
}

void code_generator::translate_indexed_copy_to(const quad_pointer& instruction){
	// INDEXED_COPY_TO,	// x[i] = y
	//		mov[b|w|l|q] y,x[i] (igual al anterior, solo que para representar x[i]
	//		tenemos que recordar la notación offset(base,index,scale) que representa
	//		la dirección (base + index × scale + offset) TODO: ver qué significa cada
	//		componente)
	// TODO: de donde saco offset, base, etc?
	operand_pointer x = new_memory_operand(offset, base, index, scale);
	operand_pointer y = get_address(instruction->arg2);
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	// We use a new register to ensure that the second operand can be
	// the destination of the product.
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_mov_instruction(y, x, ops_type));
}

void code_generator::translate_indexed_copy_from(const quad_pointer& instruction){
	// INDEXED_COPY_FROM,	// x = y[i]
	// 		mov[b|w|l|q] y[i], x (idem caso anterior)
	// TODO: de donde saco offset, base, etc?
	operand_pointer y = new_memory_operand(offset, base, index, scale);
	operand_pointer x = get_address(instruction->result);
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	// We use a new register to ensure that the second operand can be
	// the destination of the product.
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_mov_instruction(y, x, ops_type));
}

void code_generator::translate_unconditional_jump(const quad_pointer& instruction){
	// UNCONDITIONAL_JUMP,	// goto L
	// jmp L
	// TODO: está en arg1 la etiqueta?
	translation->push_back(new_jmp_instruction(*instruction->arg1->value.label));
}

void code_generator::translate_conditional_jump(const quad_pointer& instruction){
	// CONDITIONAL_JUMP,	// if x goto L
	// 		TODO: primero debemos transformar el testeo de x en la comparación
	// 		con el valor 0 o signo del mismo. Usaremos estas instrucciones
	// 		cmp[b|w|l|q] s 2 ,s 1       set flags based on s 1 − s 2
	//		test[b|w|l|q] s 2 ,s 1      set flags based on s 1 & s 2 (logical and)
	//		Luego usar:
	//			jcc L  donde cc es la condición adecuada, para que salte si x es verdadero
	operand_pointer aux_reg; // TODO: cómo lo defino?
	// TODO: fijarse si estamos extrayendo correctamente las distintas componentes
	// de instruction
	operand_pointer x = get_address(instruction->arg1);
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_mov_instruction(new_immediate_int_operand(1),
												aux_reg, ops_type));
	translation->push_back(new_cmp_instruction(x, aux_reg, ops_type));

	switch(instruction->op){
		case quad_oper::IFTRUE:
			translation->push_back(new_je_instruction(*instruction->arg1->value.label));
			break;

		case quad_oper::IFFALSE:
			translation->push_back(new_jne_instruction(*instruction->arg1->value.label));
			break;

		default:
			assert(false);
	}
}

void code_generator::translate_relational_jump(const quad_pointer& instruction){
	operand_pointer x = get_address(instruction->arg1);
	operand_pointer y = get_address(instruction->arg2);
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_cmp_instruction(x, y, ops_type));

	switch(instruction->op){
		case operation::LESS:
			// RELATIONAL_JUMP: if x < y goto L
			//		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jl L 				(salta si la comparación resultó less (signed <))
			// TODO: fijarse si estamos extrayendo correctamente las distintas componentes
			// de instruction
			translation->push_back(new_jl_instruction(*instruction->result->value.label));
			break;

		case operation::LESS_EQUAL:
			// RELATIONAL_JUMP,	// if x <= y goto L
			//		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jle L 				(salta si la comparación resultó less or
			//		equal (signed <=))
			translation->push_back(new_jle_instruction(*instruction->result->value.label));
			break;

		case operation::GREATER:
			// RELATIONAL_JUMP,	// if x > y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jg L
			translation->push_back(new_jg_instruction(*instruction->result->value.label));
			break;

		case operation::GREATER_EQUAL:
			// RELATIONAL_JUMP,	// if x >= y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jge L 				(salta si la comparación resultó greater or equal (signed >=)
			translation->push_back(new_jge_instruction(*instruction->result->value.label));
			break;

		case operation::EQUAL:
			// RELATIONAL_JUMP,	// if x == y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		je L 				(salta si la comparación resultó equal to zero)
			translation->push_back(new_je_instruction(*instruction->result->value.label));
			break;

		case operation::DISTINCT:
			// RELATIONAL_JUMP,	// if x != y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jne L 				(salta si la comparación resultó not equal to zero)
			translation->push_back(new_jne_instruction(*instruction->result->value.label));
			break;

		default:
			// TODO: así fallamos?
			assert(false);
	}

}

void code_generator::translate_parameter(const quad_pointer& instruction){
	// PARAMETER: param x
	// 		Integer arguments (up to the first six) are passed in registers,
	// 		namely: %rdi, %rsi, %rdx, %rcx, %r8, %r9. Así es que la traducción
	// 		debería ser algo como:
	// 			mov[b|w|l|q] x,%rdi
	// 		para el primer parámetro, y así...
	// 		Observación: Floating arguments (up to 8) are passed in SSE registers
	// 		%xmm0, %xmm1, ..., %xmm7
	// 		Observación: Any arguments passed on the stack are pushed in reverse
	// 		(right-to-left) order.
	// TODO: hago el caso de los enteros...
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	operand_pointer reg; // TODO: cómo lo defino?
	operand_pointer x = get_address(instruction->arg1);
	translation->push_back(new_mov_instruction(x, reg, ops_type));
}

void code_generator::translate_procedure_call(const quad_pointer& instruction){
	// PROCEDURE_CALL: call p, n
	// 		call p				(n no hace falta?)
	// 		TODO: qué deberíamos hacer con esto:
	// 		Registers %rbx,%rbp,%r12,%r13,%r14,%r15 are callee-save; that is, the
	// 		callee is responsible for making sure that the values in these registers
	// 		are the same at exit as they were on entry.
	translation->push_back(new_call_instruction(*instruction->result->value.label));
}

void code_generator::translate_function_call(const quad_pointer& instruction){

}

void code_generator::translate_return(const quad_pointer& instruction){
	// Observación: An integer-valued function returns its result in %rax. En
	// 		ese caso, la traducción de return debería ser:
	// 			mov y, %rax
	// 			leave				(The leave instruction sets %rsp to %rbp and then
	// 								pops the stack into %rbp, effectively popping the
	// 								entire current stack frame. It is nominally
	// 								intended to reverse the action of a previous
	// 								enter instruction.)
	// 			ret					(ret pops the top of stack into %rip, thus resuming
	// 								execution in the calling routine)
	// TODO: por ahora, solo resuelvo el caso para enteros
	operand_pointer rax = new_register_operand(register_id::RAX);
	operand_pointer x = get_address(instruction->arg1);
	translation->push_back(new_mov_instruction(x, rax, ops_type));
	translation->push_back(new_leave_instruction());
	translation->push_back(new_ret_instruction());
}

void code_generator::translate_label(const quad_pointer& instruction){
	// LABEL: L :
	//		L:
	translation->push_back(new_label_instruction(*instruction->result->value.label));
}

void code_generator::translate_begin_procedure(const quad_pointer& instruction){
	// BEGIN_PROCEDURE n
	//		enter n
	translation->push_back(new_enter_instruction(*instruction->result->value.constant.ival));
}

void code_generator::translate_ir(void){
	// TODO: esta es la forma de iterar una instructions_list?

	// Translator with window size of 1
	for(instructions_list::iterator it = ir->begin();
	it != ir->end(); ++it){

		switch((*it)->type){
			case BINARY_ASSIGN:
				translate_binary_op(*it);
				break;

			case UNARY_ASSIGN:
				translate_unary_op(*it);
				break;

			case COPY:
				translate_copy(*it);
				break;


			case INDEXED_COPY_TO:
				translate_indexed_copy_to(*it);
				break;

			case INDEXED_COPY_FROM:
				translate_indexed_copy_from(*it);
				break;

			case UNCONDITIONAL_JUMP:
				translate_unconditional_jump(*it);
				break;

			case CONDITIONAL_JUMP:
				translate_conditional_jump(*it);
				break;

			case RELATIONAL_JUMP:
				translate_relational_jump(*it);
				break;

			case PARAMETER:
				translate_parameter(*it);
				break;

			case PROCEDURE_CALL:
				translate_procedure_call(*it);
				break;

			case FUNCTION_CALL:
				translate_function_call(*it);
				break;

			case RETURN:
				translate_return(*it);
				break;

			case LABEL:
				translate_label(*it);
				break;

			case BEGIN_PROCEDURE:
				translate_begin_procedure(*it);
		}

	}
}

