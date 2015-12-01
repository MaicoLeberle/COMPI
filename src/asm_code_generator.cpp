#include "asm_code_generator.h"

/* TODO: temas a resolver:
 * 		_ resolver cómo determinar el signo correcto de los offsets de las
 * 		variables (positivo para parámetros de una función, negativos para las
 * 		variables locales). Creo que esto lo debería resolver al nivel de
 * 		generador de código assembly y no a nivel de generación de código
 * 		intermedio.
 *
 * 		_ cuando una función recibe parámetros a través de registros (por ej.
 * 		los primeros 6 param enteros), el preludio de la misma debería pasar
 * 		a memoria estos parámetros?
 *
 * TODO: fijarse cómo resolver la llamada a servicios de la librería estándar
 * de C: qué label utilizamos? */

asm_code_generator::asm_code_generator(instructions_list *_ir, ids_info *_s_table) :
ir(_ir), s_table(_s_table) {

	translation = new asm_instructions_list();
	stack_params = nullptr;
	act_reg_av = register_id::NONE;
	using_registers = false;
}

register_id asm_code_generator::get_next_reg_av(){
	register_id ret;

	switch(act_reg_av){
		case register_id::NONE:
			if(not using_registers){
				ret = register_id::RDI;
				using_registers = true;
			}
			else{
				// {using_registers}
				ret = register_id::NONE;
			}
			break;

		case register_id::RDI:
			ret = register_id::RSI;
			break;

		case register_id::RSI:
			ret = register_id::RDX;
			break;

		case register_id::RDX:
			ret = register_id::RCX;
			break;

		case register_id::RCX:
			ret = register_id::R8;
			break;

		case register_id::R8:
			ret = register_id::R9;
			break;

		default:
			#ifdef __DEBUG
				assert(act_reg_av == register_id::R9);
			#endif
			ret = register_id::NONE;
	}
	act_reg_av = ret;

	return ret;
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
	operand_pointer ret = nullptr;

	switch(address->type){
		case address_type::ADDRESS_NAME:{
			ret = new_memory_operand(s_table->get_offset(*address->value.name),
									register_id::RBP,
									register_id::NONE,
									1);
			break;
		}

		case address_type::ADDRESS_TEMP:{
			ret = new_memory_operand(s_table->get_offset(*address->value.label),
									register_id::RBP,
									register_id::NONE,
									1);
			break;
		}

		case address_type::ADDRESS_CONSTANT:{
			switch(address->value.constant.type){
				case value_type::BOOLEAN:
					if(address->value.constant.val.bval){
						ret = new_immediate_integer_operand(1);
						break;
					}
					else{
						// {not address->value.constant.val.bval}
						ret = new_immediate_integer_operand(0);
						break;
					}

				case value_type::INTEGER:
					ret = new_immediate_integer_operand(
							address->value.constant.val.ival);
					break;

				case value_type::FLOAT:
					ret = new_immediate_float_operand(
							address->value.constant.val.fval);
					break;
			}

			break;
		}
	}

	return ret;

}

void asm_code_generator::translate_binary_op(const quad_pointer& instruction){

	switch(get_inst_op(instruction)){
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
			translation->push_back(new_mov_instruction(y, new_register, data_type::L));
			// TODO: signed?
			translation->push_back(new_mul_instruction(z, new_register, data_type::L, true));
			translation->push_back(new_mov_instruction(new_register, x, data_type::L));
			break;
		}

		case quad_oper::DIVIDE:{
			// Register to store the dividend.
			operand_pointer edx_reg = new_register_operand(register_id::EDX);
			operand_pointer aux_edx_reg = new_register_operand(register_id::R8D);
			operand_pointer eax_reg = new_register_operand(register_id::EAX);
			operand_pointer aux_eax_reg = new_register_operand(register_id::R9D);

			// Operands are always of 32-bit long.
			data_type ops_type = data_type::L;

			// Operands.
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			// TODO: quizás deberíamos guardar "y" en algún registro, hacer corrimiento
			// de 32 posiciones hacia la derecha (para quedarnos con los 32 bits
			// de mayor peso), y guardar el resultado en EDX
			operand_pointer zero = new_immediate_integer_operand(0);

			// Save content of registers edx and eax (because they could be
			// used for store parameters of a procedure).
			translation->push_back(new_mov_instruction(edx_reg,
														aux_edx_reg,
														ops_type));
			translation->push_back(new_mov_instruction(eax_reg,
														aux_eax_reg,
														ops_type));
			// Copy operand y into register eax: COMPI's integers are signed
			// 32 bits long.
			translation->push_back(new_mov_instruction(y, eax_reg, ops_type));
			// Copy the immediate value 0 into register edx.
			translation->push_back(new_mov_instruction(zero, edx_reg, ops_type));
			// Divide %edx::%eax by operand z.
			// TODO: no hace falta indicar que signed, ya que...siempre es signed
			translation->push_back(new_div_instruction(z, ops_type, true));
			// Quotient is into register eax, move it to x.
			translation->push_back(new_mov_instruction(eax_reg, x, ops_type));
			// Restore the content of registers edx and eax.
			translation->push_back(new_mov_instruction(aux_edx_reg,
														edx_reg,
														ops_type));
			translation->push_back(new_mov_instruction(aux_eax_reg,
														eax_reg,
														ops_type));

			break;
		}

		case quad_oper::MOD:{
			// Register to store the dividend.
			operand_pointer edx_reg = new_register_operand(register_id::EDX);
			operand_pointer aux_edx_reg = new_register_operand(register_id::R8D);
			operand_pointer eax_reg = new_register_operand(register_id::EAX);
			operand_pointer aux_eax_reg = new_register_operand(register_id::R9D);

			// Operands are always of 32-bit long.
			data_type ops_type = data_type::L;

			// Operands.
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			operand_pointer z = get_address(instruction->arg2);
			// TODO: quizás deberíamos guardar "y" en algún registro, hacer corrimiento
			// de 32 posiciones hacia la derecha (para quedarnos con los 32 bits
			// de mayor peso), y guardar el resultado en EDX
			operand_pointer zero = new_immediate_integer_operand(0);

			// Save content of registers edx and eax (because they could be
			// used for store parameters of a procedure).
			translation->push_back(new_mov_instruction(edx_reg,
														aux_edx_reg,
														ops_type));
			translation->push_back(new_mov_instruction(eax_reg,
														aux_eax_reg,
														ops_type));
			// Copy operand y into register eax: COMPI's integers are signed
			// 32 bits long.
			translation->push_back(new_mov_instruction(y, eax_reg, ops_type));
			// Copy the immediate value 0 into register edx.
			translation->push_back(new_mov_instruction(zero,
														edx_reg,
														ops_type));
			// Divide %edx::%eax by operand z.
			// TODO: no hace falta indicar que signed, ya que...siempre es signed
			translation->push_back(new_div_instruction(z, ops_type, true));
			// Quotient is into register eax, move it to x.
			translation->push_back(new_mov_instruction(edx_reg, x, ops_type));
			// Restore the content of registers edx and eax.
			translation->push_back(new_mov_instruction(aux_edx_reg,
														edx_reg,
														ops_type));
			translation->push_back(new_mov_instruction(aux_eax_reg,
														eax_reg,
														ops_type));

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
			// TODO: estamos asumiendo que los booleanos los representamos con
			// 32-bits.
			operand_pointer new_register = new_register_operand(register_id::R8D);
			operand_pointer x = get_address(instruction->result);
			operand_pointer y = get_address(instruction->arg1);
			data_type ops_type = data_type::L;

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
	operand_pointer x = get_address(instruction->result);
	operand_pointer y = get_address(instruction->arg1);

	#ifdef __DEBUG
		assert(instruction->result->type == ADDRESS_NAME ||
				instruction->result->type == ADDRESS_TEMP);
	#endif

	translation->push_back(new_mov_instruction(y, x, data_type::L));
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
	operand_pointer aux_reg = new_register_operand(register_id::R8D);
	// TODO: fijarse si estamos extrayendo correctamente las distintas componentes
	// de instruction
	operand_pointer x = get_address(instruction->arg1);
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	operand_pointer aux = new_immediate_integer_operand(1);
	translation->push_back(new_mov_instruction(aux,
												aux_reg,
												data_type::L));
	translation->push_back(new_cmp_instruction(x, aux_reg, data_type::L));

	switch(instruction->op){
		case quad_oper::IFTRUE:
			translation->push_back(new_je_instruction(*instruction->arg2->value.label));
			break;

		case quad_oper::IFFALSE:
			translation->push_back(new_jne_instruction(*instruction->arg2->value.label));
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

	// TODO: utilizar encapsulamiento!!!
	switch(get_inst_op(instruction)){
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

void asm_code_generator::allocate_integer_param(const operand_pointer& val){
	// TODO: en qué momento libero los registros?
	register_id reg = get_next_reg_av();
	if(reg != register_id::NONE){
		translation->push_back(
				new_mov_instruction(val, new_register_operand(reg), data_type::L));
	}
	else{
		// {reg == register_id::NONE}
		// No more registers available: additional arguments,
		// if needed, are passed into stack slots immmediately
		// above the return address, in inverse order.
		if(stack_params == nullptr){
			// TODO: borrar!
			stack_params = new asm_instructions_list();
		}
		stack_params->insert(stack_params->begin(),
							new_pushq_instruction(val, data_type::L));
	}
}

void asm_code_generator::translate_parameter(const quad_pointer& instruction){
	address_pointer arg1 = get_inst_arg1(instruction);
	operand_pointer x = get_address(arg1);

	// TODO: vamos a poder manejar strings...fijarse qué pasa cuando compilamos
	// un string con gcc: declara una sección identificada con etiqueta, fuera
	// del método en donde se declara.
	address_type addr_type = get_address_type(arg1);

	if(addr_type == address_type::ADDRESS_NAME or addr_type == address_type::ADDRESS_TEMP){
			std::string name = get_address_name(arg1);
			id_type type = s_table->get_type(name);

			switch(type){
				case T_INT:
					allocate_integer_param(x);
					break;

				case T_BOOL:
					// TODO: lo tratamos como un entero de 32 bits?. En
					// abi.pdf, se indica que los booleanos deben tener un byte
					// de longitud, y sólo el bit 0 debe contener el valor booleano
					// mientras que los demás bits deben ser 0.
					allocate_integer_param(x);
					break;

				case T_FLOAT:
					// TODO: terminar
					break;

				case T_STRING:
					break;

				case T_CHAR:
					break;

				default:
					// {type == T_UNDEFINED}
					break;
			}
		}
	else{
		// {addr_type == address_type::ADDRESS_CONSTANT}
		switch(get_constant_address_type(arg1)){
			case value_type::INTEGER:{
				allocate_integer_param(x);
				break;
			}

			case value_type::FLOAT:
				// TODO
				break;

			default:
				// {get_constant_address_type(arg1) == value_type::BOOLEAN}
				// TODO: directamente lo considero como un entero?
				;
		}
	}
}

void asm_code_generator::translate_procedure_call(const quad_pointer& instruction){
	// If present, add pushq instructions, to save parameters into the stack.
	if(stack_params != nullptr){
		for (asm_instructions_list::iterator it = stack_params->begin();
		it != stack_params->end(); ++it){
			translation->push_back(*it);
		}

		delete stack_params;
		stack_params = nullptr;
	}

	// TODO: por ahora, no pasamos ningún parámetro que requiera el uso del
	// vector-registers...
	// Indicate, in register %rax, the total number of floating point
	// parameters passed to the function in vector registers.
	operand_pointer const_0 = new_immediate_integer_operand(0);
	operand_pointer reg_rax = new_register_operand(register_id::RAX);

	translation->push_back(new_mov_instruction(const_0,
												reg_rax,
												data_type::L));

	// Save EIP and jump to the called procedure.
	translation->push_back(new_call_instruction(*instruction->arg1->value.label));
}

void asm_code_generator::translate_function_call(const quad_pointer& instruction){
	operand_pointer dest = get_address(instruction->result);
	operand_pointer reg_rax = new_register_operand(register_id::RAX);

	translate_procedure_call(instruction);

	translation->push_back(new_mov_instruction(reg_rax, dest, data_type::L));
}

void asm_code_generator::translate_return(const quad_pointer& instruction){
	data_type ops_type = data_type::L;
	// TODO: por ahora sólo resolvemos el caso para los enteros.
	operand_pointer rax = new_register_operand(register_id::RAX);
	operand_pointer x = get_address(instruction->arg1);
	translation->push_back(new_mov_instruction(x, rax, ops_type));
	translation->push_back(new_leave_instruction());
	translation->push_back(new_ret_instruction());
}

void asm_code_generator::translate_label(const quad_pointer& instruction){
	translation->push_back(new_label_instruction(*instruction->result->value.label));
}

void asm_code_generator::translate_enter_procedure(const quad_pointer&
													instruction){

	operand_pointer stack_space = new_immediate_integer_operand(
									instruction->arg1->value.constant.val.ival
									);
	operand_pointer nesting = new_immediate_integer_operand(0);
	translation->push_back(new_enter_instruction(stack_space, nesting));
}

void asm_code_generator::translate_ir(void){
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
