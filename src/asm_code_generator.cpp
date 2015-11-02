#include "asm_code_generator.h"

asm_code_generator::asm_code_generator(instructions_list *_ir, ids_info *_s_table) :
ir(_ir), s_table(_s_table) {

	translation = new asm_instructions_list();
}

void asm_code_generator::print_translation_intel_syntax(){
	for(asm_instructions_list::iterator it = translation->begin();
		it != translation->end();
		++it){

		std::cout << print_intel_syntax(*it) << std::endl;
	}
}

asm_instructions_list* asm_code_generator::get_translation(){
	return translation;
}

operand_pointer asm_code_generator::get_address(address_pointer address){
	// TODO: cómo discriminamos los demás casos de operand?
	// TODO: esto no debería estar en asm_instruction?
	operand_pointer ret = nullptr;

	switch(address->type){
		case address_type::ADDRESS_NAME:{
			// TODO: cómo determinamos scale, base e index?
			ret = new_memory_operand(s_table->get_offset(*address->value.name),
									register_id::RSP,
									register_id::NONE,
									1);
			/*ret->op_addr = operand_addressing::MEMORY;
			ret->value.mem.offset = s_table->get_offset(*address->value.name);

			ret->value.mem.base = 0;
			ret->value.mem.scale = 0;
			ret->value.mem.index = 0;
			break;*/
			break;
		}

		case address_type::ADDRESS_CONSTANT:{
			switch(address->value.constant.type){
				case value_type::BOOLEAN:
					ret = new_immediate_boolean_operand(address->value.constant.val.bval);
					break;

				case value_type::INTEGER:
					ret = new_immediate_integer_operand(address->value.constant.val.ival);
					break;

				case value_type::FLOAT:
					ret = new_immediate_float_operand(address->value.constant.val.fval);
					break;
			}

			break;
		}
	}

	return ret;

}

void asm_code_generator::translate_binary_op(const quad_pointer& instruction){

	switch(instruction->op){
		case quad_oper::TIMES:{
			// BINARY_ASSIGN x = y * z:
			//		mov[b|w|l|q] z,register
			// 		imulw y,z (z = z ∗ y, z debe ser un registro)
			// 		mov[b|w|l|q] z,x (x = z)
			operand_pointer new_register = new_register_operand(register_id::R8D);
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			// We use a new register to ensure that the second operand can be
			// the destination of the product.
			// TODO: en la generación de código intermedio, no nos aseguramos de
			// esto?
			data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
			translation->push_back(new_mov_instruction(z, new_register, data_type::L));
			// TODO: signed?
			translation->push_back(new_mul_instruction(y, new_register, data_type::L, true));
			translation->push_back(new_mov_instruction(new_register, x, data_type::L));
			break;
		}

		case quad_oper::DIVIDE:{
			// BINARY_ASSIGN x = y / z:
			//		mov[b|w|l|q] y,%edx::%eax (puede ir así?)
			// 		idivl z (signed divide of %edx::%eax by z; quotient in %eax, remainder in %edx)
			// 		mov[b|w|l|q] %eax,x
			operand_pointer edx_reg = new_register_operand(register_id::EDX);
			operand_pointer aux_edx_reg = new_register_operand(register_id::R8D);
			operand_pointer eax_reg = new_register_operand(register_id::EAX);
			operand_pointer aux_eax_reg = new_register_operand(register_id::R9D);
			data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			// TODO: estoy moviendo los primeros 32 bits de y en eax?
			translation->push_back(new_mov_instruction(y, eax_reg, data_type::L));
			// TODO: quizás deberíamos guardar "y" en algún registro, hacer corrimiento
			// de 32 posiciones hacia la derecha (para quedarnos con los 32 bits
			// de mayor peso), y guardar el resultado en EDX
			// TODO: definir aux_reg
			operand_pointer zero = new_immediate_integer_operand(0);
			// TODO: como pasamos parámetros a través de los registros edx y eax
			// guardamos primero el contenido de los mismos, por las dudas,
			// en nuevos registros, para después recuperar el contenido de los
			// mismos.
			translation->push_back(new_mov_instruction(edx_reg, aux_edx_reg, ops_type));
			translation->push_back(new_mov_instruction(eax_reg, aux_eax_reg, ops_type));
			translation->push_back(new_mov_instruction(y, eax_reg, ops_type));
			translation->push_back(new_mov_instruction(zero, edx_reg, ops_type));
			// TODO: signed?
			translation->push_back(new_div_instruction(z, ops_type, true));
			translation->push_back(new_mov_instruction(eax_reg, x, ops_type));
			translation->push_back(new_mov_instruction(aux_edx_reg, edx_reg, ops_type));
			translation->push_back(new_mov_instruction(aux_eax_reg, eax_reg, ops_type));
			break;
		}

		case quad_oper::MOD:{
			// BINARY_ASSIGN x = y / z:
			//		mov[b|w|l|q] y,%edx::%eax (puede ir así?)
			// 		idivl z (signed divide of %edx::%eax by z; quotient in %eax, remainder in %edx)
			// 		Podemos hacer lo mismo que con la división, y tomar el resto de %edx)
			//
			// 		mov[b|w|l|q] %edx,x
			operand_pointer edx_reg = new_register_operand(register_id::EDX);
			operand_pointer aux_edx_reg = new_register_operand(register_id::R8D);
			operand_pointer eax_reg = new_register_operand(register_id::EAX);
			operand_pointer aux_eax_reg = new_register_operand(register_id::R9D);
			data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			// TODO: estoy moviendo los primeros 32 bits de y en eax?
			translation->push_back(new_mov_instruction(y, eax_reg, ops_type));
			// TODO: quizás deberíamos guardar "y" en algún registro, hacer corrimiento
			// de 32 posiciones hacia la derecha (para quedarnos con los 32 bits
			// de mayor peso), y guardar el resultado en EDX
			// TODO: definir aux_reg
			operand_pointer zero = new_immediate_integer_operand(0);
			translation->push_back(new_mov_instruction(edx_reg, aux_edx_reg, ops_type));
			translation->push_back(new_mov_instruction(eax_reg, aux_eax_reg, ops_type));
			translation->push_back(new_mov_instruction(zero, edx_reg, ops_type));
			// TODO: signed?
			translation->push_back(new_div_instruction(z, ops_type, true));
			translation->push_back(new_mov_instruction(edx_reg, x, ops_type));
			translation->push_back(new_mov_instruction(aux_edx_reg, edx_reg, ops_type));
			translation->push_back(new_mov_instruction(aux_eax_reg, eax_reg, ops_type));
			break;
		}

		case quad_oper::PLUS:{
			// BINARY_ASSIGN x = y + z:
			//		addw y,z (z = y+z) Los operandos son de tipo word
			// 		mov[b|w|l|q] z,x (tendremos que ver el tipo de los operandos
			//		para determinar el tipo de instrucción?)
			// TODO: cómo lo defino?
			operand_pointer new_register = new_register_operand(register_id::R8D);
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
			translation->push_back(new_mov_instruction(y, new_register, ops_type));
			translation->push_back(new_add_instruction(z, new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));
			break;
		}

		case quad_oper::MINUS:{
			// BINARY_ASSIGN x = y - z:
			//		subw y,z (z = z − y)
			//		mov[b|w|l|q] z,x
			// TODO: cómo lo defino?
			operand_pointer new_register = new_register_operand(register_id::R8D);
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
			translation->push_back(new_mov_instruction(y, new_register, ops_type));
			translation->push_back(new_sub_instruction(z, new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));
			break;
		}

		// TODO: está bien poner less, less_equal, etc aquí?
		/*case quad_oper::LESS:

		case quad_oper::LESS_EQUAL:

		case quad_oper::GREATER:

		case quad_oper::GREATER_EQUAL:

		case quad_oper::EQUAL:

		case quad_oper::DISTINCT:

		case quad_oper::AND:

		case quad_oper::OR:*/
	}
}

void asm_code_generator::translate_unary_op(const quad_pointer& instruction){

	switch(instruction->op){
		case quad_oper::NEGATIVE:{
			// UNARY_ASSIGN x = -y
			// 		negw y (y = −y)
			// 		mov[b|w|l|q] y,x
			// TODO: cómo lo defino?
			operand_pointer new_register = new_register_operand(register_id::R8D);
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
			// We use a new register to ensure that the second operand can be
			// the destination of the product.
			// TODO: en la generación de código intermedio, no nos aseguramos de
			// esto?
			translation->push_back(new_mov_instruction(y, new_register, ops_type));
			translation->push_back(new_neg_instruction(new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));
			break;
		}

		case quad_oper::NEGATION:{
			// UNARY_ASSIGN x = not y
			// notw y (y =∼ y (bitwise complement))
			// mov[b|w|l|q] y,x
			// TODO: cómo lo defino?
			operand_pointer new_register = new_register_operand(register_id::R8B);
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
			// We use a new register to ensure that the second operand can be
			// the destination of the product.
			// TODO: en la generación de código intermedio, no nos aseguramos de
			// esto?
			translation->push_back(new_mov_instruction(y, new_register, ops_type));
			translation->push_back(new_not_instruction(new_register, ops_type));
			translation->push_back(new_mov_instruction(new_register, x, ops_type));
			break;
		}
	}
}

void asm_code_generator::translate_copy(const quad_pointer& instruction){
	// COPY x = y:
	// 		mov[b|w|l|q] y,x (move s to d; tendremos que ver el tipo de los operandos
	// 		para determinar el tipo de instrucción?)
	// TODO: por ahora no hacemos arreglos

	operand_pointer x = get_address(instruction->result);
	operand_pointer y = get_address(instruction->arg1);
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	// We use a new register to ensure that the second operand can be
	// the destination of the product.
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_mov_instruction(y, x, ops_type));
}

void asm_code_generator::translate_indexed_copy_to(const quad_pointer& instruction){
	// INDEXED_COPY_TO,	// x[i] = y
	//		mov[b|w|l|q] y,x[i] (igual al anterior, solo que para representar x[i]
	//		tenemos que recordar la notación offset(base,index,scale) que representa
	//		la dirección (base + index × scale + offset) TODO: ver qué significa cada
	//		componente)
	// TODO: por ahora no hacemos arreglos
	/*
	// TODO: de donde saco offset, base, etc?
	operand_pointer x = new_memory_operand(offset, base, index, scale);
	operand_pointer y = get_address(instruction->arg2);
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	// We use a new register to ensure that the second operand can be
	// the destination of the product.
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_mov_instruction(y, x, ops_type));*/
}

void asm_code_generator::translate_indexed_copy_from(const quad_pointer& instruction){
	// INDEXED_COPY_FROM,	// x = y[i]
	// 		mov[b|w|l|q] y[i], x (idem caso anterior)
	// TODO: por ahora no hacemos arreglos
	/*
	// TODO: de donde saco offset, base, etc?
	operand_pointer y = new_memory_operand(offset, base, index, scale);
	operand_pointer x = get_address(instruction->result);
	data_type ops_type; // TODO: cómo determino el tipo de los operandos?
	// We use a new register to ensure that the second operand can be
	// the destination of the product.
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_mov_instruction(y, x, ops_type));*/
}

void asm_code_generator::translate_unconditional_jump(const quad_pointer& instruction){
	// UNCONDITIONAL_JUMP,	// goto L
	// jmp L
	// TODO: está en arg1 la etiqueta?
	translation->push_back(new_jmp_instruction(*instruction->arg1->value.label));
}

void asm_code_generator::translate_conditional_jump(const quad_pointer& instruction){
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
	data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	operand_pointer aux = new_immediate_integer_operand(1);
	translation->push_back(new_mov_instruction(aux,
												aux_reg,
												ops_type));
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

void asm_code_generator::translate_relational_jump(const quad_pointer& instruction){
	operand_pointer x = get_address(instruction->arg1);
	operand_pointer y = get_address(instruction->arg2);
	data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	translation->push_back(new_cmp_instruction(x, y, ops_type));

	switch(instruction->op){
		case quad_oper::LESS:
			// RELATIONAL_JUMP: if x < y goto L
			//		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jl L 				(salta si la comparación resultó less (signed <))
			// TODO: fijarse si estamos extrayendo correctamente las distintas componentes
			// de instruction
			translation->push_back(new_jl_instruction(*instruction->result->value.label));
			break;

		case quad_oper::LESS_EQUAL:
			// RELATIONAL_JUMP,	// if x <= y goto L
			//		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jle L 				(salta si la comparación resultó less or
			//		equal (signed <=))
			translation->push_back(new_jle_instruction(*instruction->result->value.label));
			break;

		case quad_oper::GREATER:
			// RELATIONAL_JUMP,	// if x > y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jg L
			translation->push_back(new_jg_instruction(*instruction->result->value.label));
			break;

		case quad_oper::GREATER_EQUAL:
			// RELATIONAL_JUMP,	// if x >= y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jge L 				(salta si la comparación resultó greater or equal (signed >=)
			translation->push_back(new_jge_instruction(*instruction->result->value.label));
			break;

		case quad_oper::EQUAL:
			// RELATIONAL_JUMP,	// if x == y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		je L 				(salta si la comparación resultó equal to zero)
			translation->push_back(new_je_instruction(*instruction->result->value.label));
			break;

		case quad_oper::DISTINCT:
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

void asm_code_generator::translate_parameter(const quad_pointer& instruction){
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
	data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
	operand_pointer reg; // TODO: cómo lo defino?
	operand_pointer x = get_address(instruction->arg1);
	translation->push_back(new_mov_instruction(x, reg, ops_type));
}

void asm_code_generator::translate_procedure_call(const quad_pointer& instruction){
	// PROCEDURE_CALL: call p, n
	// 		call p				(n no hace falta?)
	// 		TODO: qué deberíamos hacer con esto:
	// 		Registers %rbx,%rbp,%r12,%r13,%r14,%r15 are callee-save; that is, the
	// 		callee is responsible for making sure that the values in these registers
	// 		are the same at exit as they were on entry.
	translation->push_back(new_call_instruction(*instruction->result->value.label));
}

void asm_code_generator::translate_function_call(const quad_pointer& instruction){

}

void asm_code_generator::translate_return(const quad_pointer& instruction){
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
	data_type ops_type = data_type::L; // TODO: cómo determino el tipo de los operandos?
	operand_pointer rax = new_register_operand(register_id::RAX);
	operand_pointer x = get_address(instruction->arg1);
	translation->push_back(new_mov_instruction(x, rax, ops_type));
	translation->push_back(new_leave_instruction());
	translation->push_back(new_ret_instruction());
}

void asm_code_generator::translate_label(const quad_pointer& instruction){
	// LABEL: L :
	//		L:
	translation->push_back(new_label_instruction(*instruction->result->value.label));
}

void asm_code_generator::translate_enter_procedure(const quad_pointer& instruction){
	// BEGIN_PROCEDURE n
	//		enter n
	operand_pointer stack_space = new_immediate_integer_operand(
									instruction->arg1->value.constant.val.ival
								);
	// TODO: cómo determino nesting?
	operand_pointer nesting = new_immediate_integer_operand(0);
	translation->push_back(new_enter_instruction(stack_space, nesting));
}

void asm_code_generator::translate_ir(void){
	// TODO: esta es la forma de iterar una instructions_list?

	// Translator with window size of 1
	for(instructions_list::iterator it = ir->begin();
	it != ir->end(); ++it){

		switch((*it)->type){
			case quad_type::BINARY_ASSIGN:
				translate_binary_op(*it);
				break;

			case quad_type::UNARY_ASSIGN:
				translate_unary_op(*it);
				break;

			case quad_type::COPY:
				translate_copy(*it);
				break;


			case quad_type::INDEXED_COPY_TO:
				translate_indexed_copy_to(*it);
				break;

			case quad_type::INDEXED_COPY_FROM:
				translate_indexed_copy_from(*it);
				break;

			case quad_type::UNCONDITIONAL_JUMP:
				translate_unconditional_jump(*it);
				break;

			case quad_type::CONDITIONAL_JUMP:
				translate_conditional_jump(*it);
				break;

			case quad_type::RELATIONAL_JUMP:
				translate_relational_jump(*it);
				break;

			case quad_type::PARAMETER:
				translate_parameter(*it);
				break;

			case quad_type::PROCEDURE_CALL:
				translate_procedure_call(*it);
				break;

			case quad_type::FUNCTION_CALL:
				translate_function_call(*it);
				break;

			case quad_type::RETURN:
				translate_return(*it);
				break;

			case quad_type::LABEL:
				translate_label(*it);
				break;

			case quad_type::ENTER_PROCEDURE:
				translate_enter_procedure(*it);
		}

	}
}
