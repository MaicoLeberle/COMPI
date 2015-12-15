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
 * de C: qué label utilizamos?
 *
 * TODO: agregar al Makefile la dependendencia de las unidades de compilación
 * con respecto a los archivos de header.
 *
 * TODO: agregar el chequeo en tiempo de compilación de los índices con los que
 * se accede al vector.
 *
 * TODO: agregar lo de la identificación del Main!: tiene que ser sólo main!
 * y no main.main*/

asm_code_generator::asm_code_generator(instructions_list *_ir,
										ids_info *_s_table) :
ir(_ir), s_table(_s_table) {

	// PRE
	#ifdef __DEBUG
		assert(_s_table != nullptr);
	#endif

	this->translation = new asm_instructions_list();
	this->stack_params = nullptr;
	this->last_reg_used = register_id::NONE;
	this->params_in_registers = 0;
	this->offset = -1;
}

register_id asm_code_generator::get_next_reg_av(register_id reg){
	register_id ret;

	switch(reg){
		case register_id::NONE:
			ret = register_id::RDI;
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
				assert(reg == register_id::R9);
			#endif
			ret = register_id::NONE;
	}

	return ret;
}

// TODO: esto debería estar en ids_info?
bool asm_code_generator::is_attribute(std::string var_name){
	bool ret = false;

	for(t_attributes::iterator it = this->actual_class_attributes.begin();
	it != this->actual_class_attributes.end(); it++){
		if(var_name == std::get<0>(*it)){
			ret = true;
			break;
		}
	}

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
			std::string var_name = get_address_name(address);
			ret = new_memory_operand(
								s_table->get_offset(get_address_name(address)),
								register_id::RBP,
								register_id::NONE,
								1);
			break;
		}

		case address_type::ADDRESS_TEMP:{
			ret = new_memory_operand(s_table->get_offset(
									get_label_address_value(address)),
									register_id::RBP,
									register_id::NONE,
									1);
			break;
		}

		case address_type::ADDRESS_CONSTANT:{
			switch(get_constant_address_type(address)){
				case value_type::BOOLEAN:
					if(get_constant_address_boolean_value(address)){
						ret = new_immediate_integer_operand(1);
						break;
					}
					else{
						// {not get_constant_address_boolean_value(address)}
						ret = new_immediate_integer_operand(0);
						break;
					}

				case value_type::INTEGER:
					ret = new_immediate_integer_operand(
							get_constant_address_integer_value(address));
					break;

				case value_type::FLOAT:
					ret = new_immediate_float_operand(
							get_constant_address_float_value(address));
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
			data_type ops_type = data_type::L;
			// TODO: cada vez que llamo a get_address, tengo que averiguar si
			// el address se refiere a un atributo. Si así fuera, tengo que
			// determinar la ubicación del atributo, en base al uso de la
			// referencia this, y utilizar, quizás lea en vez de mov. El offset
			// de los atributos deberia entonces referirse a su pos dentro del
			// objeto.
			translation->push_back(new_mov_instruction(y,
														new_register,
														data_type::L));
			// TODO: signed?
			translation->push_back(new_mul_instruction(z,
														new_register,
														data_type::L,
														true));
			translation->push_back(new_mov_instruction(new_register,
														x,
														data_type::L));
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

		default:{
			// UNARY_ASSIGN x = & y
			// lea[b|w|l|q] y,x
			operand_pointer x = get_address(get_unary_assign_dest(instruction));
			operand_pointer y = get_address(get_unary_assign_src(instruction));
			data_type ops_type = data_type::L;

			translation->push_back(new_lea_instruction(y, x, ops_type));
			break;
		}
	}
}

void asm_code_generator::translate_copy(const quad_pointer& instruction){
	// COPY x = y:
	// 		mov[b|w|l|q] y,x
	operand_pointer x = get_address(get_copy_inst_dest(instruction));
	operand_pointer y = get_address(get_copy_inst_orig(instruction));

	#ifdef __DEBUG
		address_type type = get_address_type(get_copy_inst_dest(instruction));
		assert(type == address_type::ADDRESS_NAME ||
			   type == address_type::ADDRESS_TEMP);
	#endif

	translation->push_back(new_mov_instruction(y, x, data_type::L));
}

void asm_code_generator::translate_indexed_copy_to(const quad_pointer&
														instruction){
	// INDEXED_COPY_TO,	// x[i] = y
	//		mov[b|w|l|q] y,x[i] (igual al anterior, solo que para representar x[i]
	//		tenemos que recordar la notación offset(base,index,scale) que representa
	//		la dirección (base + index × scale + offset) TODO: ver qué significa cada
	//		componente)

	// TODO: que hay de la directiva PTR?
	address_pointer dest_add = get_indexed_copy_to_dest(instruction);
	operand_pointer dest = get_address(dest_add);

	address_pointer index_add = get_indexed_copy_to_index(instruction);
	//operand_pointer index = get_address(index);

	int offset = get_constant_address_integer_value(index_add);

	operand_pointer orig = get_address(get_indexed_copy_to_src(instruction));

	#ifdef __DEBUG
		assert(get_address_type(dest_add) == address_type::ADDRESS_NAME);
	#endif

	// Determine if destination is an array position or an object's attribute.
	std::string name = get_address_name(dest_add);
	operand_pointer pos = nullptr;
	if(s_table->get_kind(name) == id_kind::K_OBJECT){
		// TODO: está bien este registro?
		operand_pointer reg = new_register_operand(register_id::RDI);

		// Move the address of the object into register register_id::RDI.
		translation->push_back(new_mov_instruction(dest, reg, data_type::L));

		// TODO: quizás nos podamos ahorrar la instrucción que pasa el address
		// a rdi, si interpretamos directamente el contenido de dest como
		// un puntero al objeto?
		// Define the position of the attribute, as an offset with respect
		// to the initial address of the instance.
		pos = new_memory_operand(offset,
								register_id::RDI,
								register_id::NONE,
								1);
	}
	else{
		// {s_table->get_kind(name) != id_kind::K_OBJECT}
		// It is an array. Then, "dest" always refer to a position into the
		// actual stack frame.
		// TODO: pedir que id_kind también tengo un valor para representar
		// arrays.
		pos = new_memory_operand(offset + get_memory_operand_offset(dest),
								register_id::RBP,
								register_id::NONE,
								1);
	}

	// Move the desired value to the defined position into the instance.
	translation->push_back(new_mov_instruction(orig, pos, data_type::L));
}

void asm_code_generator::translate_indexed_copy_from(const quad_pointer& instruction){
	// INDEXED_COPY_FROM,	// x = y[i]
	// 		mov[b|w|l|q] y[i], x (idem caso anterior)
	// TODO: que hay de la directiva PTR?
	operand_pointer dest = get_address(get_indexed_copy_from_dest(instruction));

	address_pointer index_add = get_indexed_copy_from_index(instruction);

	int offset = get_constant_address_integer_value(index_add);

	address_pointer orig_add = get_indexed_copy_from_src(instruction);
	operand_pointer orig = get_address(orig_add);

	#ifdef __DEBUG
		assert(get_address_type(orig_add) == address_type::ADDRESS_NAME);
	#endif

	// Determine if the source is an array position or an object's attribute.
	std::string name = get_address_name(orig_add);
	operand_pointer pos = nullptr;
	if(s_table->get_kind(name) == id_kind::K_OBJECT){
		// TODO: está bien este registro?
		operand_pointer reg = new_register_operand(register_id::RDI);

		// Move the address of the object into register register_id::RDI.
		translation->push_back(new_mov_instruction(orig, reg, data_type::L));

		// TODO: quizás nos podamos ahorrar la instrucción que pasa el address
		// a rdi, si interpretamos directamente el contenido de dest como
		// un puntero al objeto?
		// Define the position of the attribute, as an offset with respect
		// to the initial address of the instance.
		pos = new_memory_operand(offset,
								register_id::RDI,
								register_id::NONE,
								1);
	}
	else{
		// {s_table->get_kind(name) != id_kind::K_OBJECT}
		// It is an array. Then, "orig" always refer to a position into the
		// actual stack frame.
		// TODO: pedir que id_kind también tengo un valor para representar
		// arrays.
		pos = new_memory_operand(offset + get_memory_operand_offset(orig),
								register_id::RBP,
								register_id::NONE,
								1);
	}

	// Move the desired value to the defined position.
	translation->push_back(new_mov_instruction(pos, dest, data_type::L));
}

void asm_code_generator::translate_unconditional_jump(
											const quad_pointer& instruction){
	// UNCONDITIONAL_JUMP,	// goto L
	// jmp L
	translation->push_back(
					new_jmp_instruction(
							get_unconditional_jmp_label(instruction)));
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
	operand_pointer x = get_address(get_conditional_jmp_guard(instruction));
	// TODO: en la generación de código intermedio, no nos aseguramos de
	// esto?
	operand_pointer aux = new_immediate_integer_operand(1);
	translation->push_back(new_mov_instruction(aux,
												aux_reg,
												data_type::L));
	translation->push_back(new_cmp_instruction(x, aux_reg, data_type::L));

	switch(instruction->op){
		case quad_oper::IFTRUE:
			translation->push_back(new_je_instruction(
										get_conditional_jmp_label(instruction)));
			break;

		case quad_oper::IFFALSE:
			translation->push_back(new_jne_instruction(
										get_conditional_jmp_label(instruction)));
			break;

		default:
			assert(false);
	}
}

void asm_code_generator::translate_relational_jump(const quad_pointer& instruction){
	operand_pointer x = get_address(instruction->arg1);
	operand_pointer y = get_address(instruction->arg2);
	data_type ops_type = data_type::L;
	translation->push_back(new_cmp_instruction(x, y, ops_type));

	switch(get_inst_op(instruction)){
		case quad_oper::LESS:
			// RELATIONAL_JUMP: if x < y goto L
			//		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jl L 				(salta si la comparación resultó less (signed <))
			translation->push_back(new_jl_instruction(get_relational_jmp_label(instruction)));
			break;

		case quad_oper::LESS_EQUAL:
			// RELATIONAL_JUMP,	// if x <= y goto L
			//		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jle L 				(salta si la comparación resultó less or
			//		equal (signed <=))
			translation->push_back(new_jle_instruction(get_relational_jmp_label(instruction)));
			break;

		case quad_oper::GREATER:
			// RELATIONAL_JUMP,	// if x > y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jg L
			translation->push_back(new_jg_instruction(get_relational_jmp_label(instruction)));
			break;

		case quad_oper::GREATER_EQUAL:
			// RELATIONAL_JUMP,	// if x >= y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jge L 				(salta si la comparación resultó greater or equal (signed >=)
			translation->push_back(new_jge_instruction(get_relational_jmp_label(instruction)));
			break;

		case quad_oper::EQUAL:
			// RELATIONAL_JUMP,	// if x == y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		je L 				(salta si la comparación resultó equal to zero)
			translation->push_back(new_je_instruction(get_relational_jmp_label(instruction)));
			break;

		case quad_oper::DISTINCT:
			// RELATIONAL_JUMP,	// if x != y goto L
			// 		cmp[b|w|l|q] x , y (set flags based on y − x)
			//		jne L 				(salta si la comparación resultó not equal to zero)
			translation->push_back(new_jne_instruction(get_relational_jmp_label(instruction)));
			break;

		default:
			// TODO: así fallamos?
			assert(false);
	}

}

void asm_code_generator::allocate_integer_param(const operand_pointer& val,
												bool pass_address){
	register_id reg = get_next_reg_av(last_reg_used);
	if(reg != register_id::NONE and params_in_registers < INT_PARAMS_INTO_REG){
		if(pass_address){
			translation->push_back(new_lea_instruction(val,
													new_register_operand(reg),
													data_type::L));
		}
		else{
			// {not pass_address}
			translation->push_back(new_mov_instruction(val,
													new_register_operand(reg),
													data_type::L));
		}

		last_reg_used = reg;
		params_in_registers++;
	}
	else{
		// {reg == register_id::NONE or
		//	params_in_registers >= INT_PARAMS_INTO_REG}

		// No more registers available: additional arguments,
		// if needed, are passed into stack slots immediately
		// above the return address, in inverse order.
		if(stack_params == nullptr){
			stack_params = new asm_instructions_list();
		}

		if(pass_address){
			// TODO: tengo que definir otro registro, ya que RDI puede estar
			// siendo utilizado a esta altura.
			operand_pointer reg = new_register_operand(register_id::RDI);
			stack_params->insert(stack_params->begin(),
								new_lea_instruction(val,
													reg,
													data_type::L));

			stack_params->insert(stack_params->begin(),
									new_pushq_instruction(reg, data_type::L));
		}
		else{
			// {not pass_address}
			stack_params->insert(stack_params->begin(),
								new_pushq_instruction(val, data_type::L));
		}

	}
}

void asm_code_generator::translate_parameter(const quad_pointer& instruction){
	if(this->offset < 0){
		// Parameters have positive offsets, with respect to the rBP pointer.
		this->offset = RBP_REGISTER_SIZE;
	}
	address_pointer param = get_param_inst_param(instruction);
	operand_pointer param_address = get_address(param);

	// TODO: vamos a poder manejar strings...fijarse qué pasa cuando compilamos
	// un string con gcc: declara una sección identificada con etiqueta, fuera
	// del método en donde se declara.
	address_type addr_type = get_address_type(param);

	if(addr_type == address_type::ADDRESS_NAME or
	addr_type == address_type::ADDRESS_TEMP){
			std::string name = get_address_name(param);

			switch(s_table->get_kind(name)){
				case K_OBJECT:
					// Objects are managed by reference.
					// Update its offset.
					s_table->set_offset(name, this->offset);
					allocate_integer_param(param_address, true);

				default:
					id_type type = s_table->get_type(name);

					switch(type){
						case T_INT:
							allocate_integer_param(param_address, false);
							break;

						case T_BOOL:
							// TODO: lo tratamos como un entero de 32 bits?. En
							// abi.pdf, se indica que los booleanos deben tener un byte
							// de longitud, y sólo el bit 0 debe contener el valor booleano
							// mientras que los demás bits deben ser 0.
							allocate_integer_param(param_address, false);
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
		}
	else{
		// {addr_type == address_type::ADDRESS_CONSTANT}
		switch(get_constant_address_type(param)){
			case value_type::INTEGER:{
				allocate_integer_param(param_address, false);
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
	translation->push_back(
					new_call_instruction(
						get_procedure_or_function_call_label(instruction)));

	// Reset the state of attributes used to manage the passing of parameters.
	last_reg_used = register_id::NONE;
	params_in_registers = 0;
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
	// If the label doesn't represents the beginning of a method's definition,
	// this strings are not used, so their actual values are of no interest.
	this->actual_method_name = get_label_inst_method_name(instruction);
	this->actual_class_name = get_label_inst_class_name(instruction);
	this->actual_class_attributes = this->s_table->get_list_attributes(
													this->actual_class_name);
	this->last_label = get_label_inst_label(instruction);

	translation->push_back(new_label_instruction(this->last_label));
}

void asm_code_generator::translate_enter_procedure(const quad_pointer&
													instruction){
	// Update the stack space to be allocated, taking into account the
	// integer parameters.
	// TODO: estoy asumiendo que siempre last_label tiene la etiqueta correcta.
	t_params params = s_table->get_list_params(this->actual_method_name);
	std::string param_name;
	t_params int_params;

	for(t_params::iterator it = params.begin(); it != params.end(); it++){
		param_name = *it;

		if(s_table->get_type(param_name) == T_INT){
			int_params.push_back(*it);
		}
	}

	#ifdef __DEBUG
		assert(int_params.size() <= 6);
	#endif

	// TODO: vamos a tener que actualizar la cantidad de bytes que pedimos en
	// el stack, para que ahora incluya los parámetros que pasamos de los
	// registros, al stack.
	// TODO: no acceder directamente a los atributos de las quad. Utilizar
	// los getters!
	operand_pointer stack_space = new_immediate_integer_operand(
								get_enter_inst_bytes(instruction)
								+ integer_width*int_params.size());

	operand_pointer nesting = new_immediate_integer_operand(0);
	translation->push_back(new_enter_instruction(stack_space, nesting));

	// Put the parameters into the stack, right into the beginning of the stack
	// frame.
	int offset = 0;
	register_id reg = register_id::NONE;
	for(t_params::iterator it = int_params.begin(); it != int_params.end(); it++){
		reg = get_next_reg_av(reg);

		// The new positions of this parameters are below the rBP pointer.
		offset -= integer_width;
		translation->push_back(
					new_mov_instruction(new_register_operand(reg),
										new_memory_operand(offset,
															register_id::RBP,
															register_id::NONE,
															1),
										data_type::L));

		// Update the offset of the parameter.
		s_table->set_offset(*it, offset);
	}
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
