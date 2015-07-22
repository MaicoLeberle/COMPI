#include "code_generator.h"

code_generator::code_generator(instructions_list *_ir) : ir(_ir) {
	translation = new asm_instructions_list();
}

void code_generator::translate_binary_op(const quad_pointer& instruction){

	switch(instruction->oper){
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


}

void code_generator::translate_relational_jump(const quad_pointer& instruction){

}

void code_generator::translate_parameter(const quad_pointer& instruction){

}

void code_generator::translate_procedure_call(const quad_pointer& instruction){

}

void code_generator::translate_function_call(const quad_pointer& instruction){

}

void code_generator::translate_return(const quad_pointer& instruction){

}

void code_generator::translate_label(const quad_pointer& instruction){

}

void code_generator::translate_begin_procedure(const quad_pointer& instruction){

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

