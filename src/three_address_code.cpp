#include "three_address_code.h"

address_pointer new_integer_constant(int value){
	address_pointer addr = address_pointer(new address);
	addr->type = address_type::ADDRESS_CONSTANT;
	addr->value.constant.val.ival = value;
	addr->value.constant.type = value_type::INTEGER;

	return addr;
}

address_pointer new_float_constant(float value){
	address_pointer addr= address_pointer(new address);
	addr->type = address_type::ADDRESS_CONSTANT;
	addr->value.constant.val.fval = value;
	addr->value.constant.type = value_type::FLOAT;

	return addr;
}

address_pointer new_boolean_constant(bool value){
	address_pointer addr = address_pointer(new address);
	addr->type = address_type::ADDRESS_CONSTANT;
	addr->value.constant.val.bval = value;
	addr->value.constant.type = value_type::BOOLEAN;

	return addr;
}

address_pointer new_name_address(const std::string& name){
	address_pointer addr = address_pointer(new address);
	addr->type = address_type::ADDRESS_NAME;
	addr->value.name = new std::string(name);

	return addr;
}

address_pointer new_label_address(const std::string& label){
	address_pointer addr = address_pointer(new address);
	addr->type = address_type::ADDRESS_LABEL;
	addr->value.label = new std::string(label);

	return addr;
}

quad_pointer new_label(const std::string& label){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::LABEL;
	instruction->arg1 = new_label_address(label);

	return instruction;
}

quad_pointer new_indexed_copy_to(const address_pointer& dest,
								const address_pointer& index,
								const address_pointer& orig){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::INDEXED_COPY_TO;
	instruction->result = dest;
	instruction->arg1 = index;
	instruction->arg2 = orig;

	return instruction;
}

quad_pointer new_indexed_copy_from(const address_pointer& dest,
								const address_pointer& orig,
								const address_pointer& index){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::INDEXED_COPY_FROM;
	instruction->result = dest;
	instruction->arg1 = orig;
	instruction->arg2 = index;

	return instruction;
}

quad_pointer new_copy(const address_pointer& dest,
					const address_pointer& orig){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::COPY;
	instruction->result = dest;
	instruction->arg1 = orig;

	return instruction;
}

quad_pointer new_enter_procedure(unsigned int nmbr){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::ENTER_PROCEDURE;
	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_CONSTANT;
	instruction->arg1->value.constant.type = value_type::INTEGER;
	instruction->arg1->value.constant.val.ival = nmbr;

	return instruction;
}

quad_pointer new_binary_assign(const address_pointer& dest,
							   const address_pointer& arg1,
							   const address_pointer& arg2,
							   quad_oper op){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::BINARY_ASSIGN;
	instruction->op = op;
	instruction->arg1 = arg1;
	instruction->arg2 = arg2;
	instruction->result = dest;

	return instruction;
}

quad_pointer new_unary_assign(const address_pointer& dest,
							   const address_pointer& arg,
							   quad_oper op){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::UNARY_ASSIGN;
	instruction->op = op;
	instruction->arg1 = arg;
	instruction->result = dest;

	return instruction;
}

quad_pointer new_parameter_inst(const address_pointer& param){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::PARAMETER;
	instruction->op = quad_oper::NONE;
	instruction->arg1 = param;

	return instruction;
}

quad_pointer new_function_call_inst(const address_pointer& dest,
									const address_pointer& func_label,
									const address_pointer& param_quantity){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::FUNCTION_CALL;
	instruction->op = quad_oper::NONE;
	instruction->result = dest;
	instruction->arg1 = func_label;
	instruction->arg2 = param_quantity;

	return instruction;
}

quad_pointer new_procedure_call_inst(const address_pointer& proc_label,
									const address_pointer& param_quantity){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::PROCEDURE_CALL;
	instruction->op = quad_oper::NONE;
	instruction->arg1 = proc_label;
	instruction->arg2 = param_quantity;

	return instruction;
}

quad_pointer new_conditional_jump_inst(const address_pointer& guard,
										const std::string& label,
										quad_oper op){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CONDITIONAL_JUMP;
	instruction->op = op;
	instruction->arg1 = guard;
	instruction->arg2 = new_label_address(label);

	return instruction;
}

quad_pointer new_unconditional_jump_inst(const std::string& label){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::UNCONDITIONAL_JUMP;
	instruction->op = quad_oper::NONE;
	instruction->arg1 = new_label_address(label);

	return instruction;
}

quad_pointer new_relational_jump_inst(const address_pointer& x,
									const address_pointer& y,
									quad_oper relop,
									const std::string& label){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::RELATIONAL_JUMP;
	instruction->op = relop;
	instruction->arg1 = x;
	instruction->arg2 = y;
	instruction->result = new_label_address(label);
	return instruction;
}

quad_pointer new_return_inst(const address_pointer& value){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::RETURN;
	instruction->op = quad_oper::NONE;
	instruction->arg1 = value;

	return instruction;
}


bool is_label(const quad_pointer& instruction, const std::string& label){
	return instruction->type == quad_type::LABEL &&
		   instruction->op == quad_oper::NONE &&
		   instruction->arg1->type == address_type::ADDRESS_LABEL &&
		   *(instruction->arg1->value.label) == label;
	// TODO: podriamos definir un procedimiento dedicado a chequear el tipo
	// de un operando...
}

bool is_enter_procedure(const quad_pointer& instruction, unsigned int bytes){
	return instruction->type == quad_type::ENTER_PROCEDURE &&
			   instruction->op == quad_oper::NONE &&
			   instruction->arg1->type == address_type::ADDRESS_CONSTANT &&
			   instruction->arg1->value.constant.val.ival == bytes;
}

bool is_procedure_call(const quad_pointer& instruction,
						const address_pointer& proc_label,
						int param_quantity){

	return instruction->type == quad_type::PROCEDURE_CALL &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_address_pointers(instruction->arg1, proc_label) &&
		   are_equal_address_pointers(instruction->arg2, new_integer_constant(param_quantity));
}

bool is_function_call(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& proc_label,
						const address_pointer& param_quantity){

	return instruction->type == quad_type::FUNCTION_CALL &&
				   instruction->op == quad_oper::NONE &&
				   are_equal_address_pointers(instruction->arg1, proc_label) &&
				   are_equal_address_pointers(instruction->arg2, param_quantity) &&
				   are_equal_address_pointers(instruction->result, dest);
}

bool is_return_inst(const quad_pointer& instruction,
					const address_pointer& ret_value){
	return instruction->type == quad_type::RETURN &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_address_pointers(instruction->arg1, ret_value);
}

bool is_indexed_copy_to(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& index,
						const address_pointer& orig){
	// TODO: se puede mantener el orden result-arg1-arg2 al guardar inf. en
	// la instruccion?
	return instruction->type == quad_type::INDEXED_COPY_TO &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_address_pointers(instruction->result, dest) &&
		   are_equal_address_pointers(instruction->arg1, index) &&
		   are_equal_address_pointers(instruction->arg2, orig);
}

bool is_indexed_copy_from(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& orig,
						const address_pointer& index){
	// TODO: se puede mantener el orden result-arg1-arg2 al guardar inf. en
	// la instruccion?
	return instruction->type == quad_type::INDEXED_COPY_FROM &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_address_pointers(instruction->result, dest) &&
		   are_equal_address_pointers(instruction->arg1, orig) &&
		   are_equal_address_pointers(instruction->arg2, index);
}

bool is_copy(const quad_pointer& instruction,
			const address_pointer& dest,
			const address_pointer& orig){
	// TODO: revisar si el orden de arg1 y result es correcto
	// TODO: para usar este procedimiento va a ser preciso indicar el tipo
	// de x e y (si son temporales o identificadores del código). Quizás
	// debería recibir un par de address
	return instruction->type == quad_type::COPY &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_address_pointers(instruction->arg1, orig) &&
		   are_equal_address_pointers(instruction->result, dest);
}

bool is_binary_assignment(const quad_pointer& instruction,
						 const address_pointer& dest,
						 const address_pointer& arg1,
						 const address_pointer& arg2,
						 quad_oper op){

	return instruction->type == quad_type::BINARY_ASSIGN &&
		   instruction->op == op &&
		   are_equal_address_pointers(instruction->arg1, arg1) &&
		   are_equal_address_pointers(instruction->arg2, arg2) &&
		   are_equal_address_pointers(instruction->result, dest);
}

bool is_unary_assignment(const quad_pointer& instruction,
						 const address_pointer& dest,
						 const address_pointer& arg,
						 quad_oper op){

	return instruction->type == quad_type::UNARY_ASSIGN &&
		   instruction->op == op &&
		   are_equal_address_pointers(instruction->arg1, arg) &&
		   are_equal_address_pointers(instruction->result, dest);
}

bool is_parameter_inst(const quad_pointer& instruction,
						const address_pointer& param){

	// TODO: se supone que si estamos creando las instrucciones con los
	// correspondientes constructores, instruction->op sí o sí es NONE...
	return instruction->type == quad_type::PARAMETER &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_address_pointers(instruction->arg1, param);
}

bool is_conditional_jump_inst(const quad_pointer& instruction,
								const address_pointer& guard,
								const std::string& label,
								quad_oper op){

	// TODO: se supone que si estamos creando las instrucciones con los
	// correspondientes constructores, instruction->arg2 sí o sí es un label
	return instruction->type == quad_type::CONDITIONAL_JUMP &&
			instruction->op == op &&
			are_equal_address_pointers(instruction->arg1, guard) &&
			*instruction->arg2->value.label == label;
}

bool is_unconditional_jump_inst(const quad_pointer& instruction,
								const std::string& label){

	// TODO: se supone que si estamos creando las instrucciones con los
	// correspondientes constructores, instruction->arg1 sí o sí es un label
	return instruction->type == quad_type::UNCONDITIONAL_JUMP &&
			*instruction->arg1->value.label == label;
}

bool is_relational_jump_inst(const quad_pointer& instruction,
							const address_pointer& x,
							const address_pointer& y,
							quad_oper relop,
							const std::string& label){

	return instruction->type == quad_type::RELATIONAL_JUMP &&
				instruction->op == relop &&
				are_equal_address_pointers(instruction->arg1, x) &&
				are_equal_address_pointers(instruction->arg2, y) &&
				*instruction->result->value.label == label;
}

bool are_equal_address_pointers(const address_pointer& x,
								const address_pointer& y){
	bool ret = false;

	if(x != nullptr){
		switch(x->type){
			case address_type::ADDRESS_NAME:
				ret = y->type == address_type::ADDRESS_NAME &&
					*(x->value.name) == *(y->value.name);
				break;

			case address_type::ADDRESS_CONSTANT:
				if (y->type == address_type::ADDRESS_CONSTANT){
					switch(x->value.constant.type){
						case value_type::BOOLEAN:
							ret = y->value.constant.type == value_type::BOOLEAN &&
							y->value.constant.val.bval == x->value.constant.val.bval;
							break;

						case value_type::INTEGER:
							ret = y->value.constant.type == value_type::INTEGER &&
							y->value.constant.val.ival == x->value.constant.val.ival;
							break;

						case value_type::FLOAT:
							ret = y->value.constant.type == value_type::FLOAT &&
							y->value.constant.val.fval == x->value.constant.val.fval;
					}
				}
				break;

			case address_type::ADDRESS_TEMP:
				ret = y->type == address_type::ADDRESS_TEMP &&
					x->value.temp == y->value.temp;
				break;

			case address_type::ADDRESS_LABEL:
				ret = y->type == address_type::ADDRESS_LABEL &&
					*(x->value.label) == *(y->value.label);
		}
	}
	else{
		// {x == nullptr}
		ret = x == y;
	}

	return ret;
}

// TODO: el nombre no es adecuado.
bool are_equal_quad_pointers(const quad_pointer& x, const quad_pointer& y){
	bool ret = false;

	if(x != nullptr){
		ret = x->type == y->type &&
			x->op == y->op &&
			are_equal_address_pointers(x->arg1, y->arg1) &&
			are_equal_address_pointers(x->arg2, y->arg2) &&
			are_equal_address_pointers(x->result, y->result);
	}
	else{
		// {x == nullptr}
		ret = x == y;
	}

	return ret;
}

bool are_equal_instructions_list(const instructions_list& x,
								const instructions_list& y){
	bool ret;

	if(x.size() == y.size()){
		instructions_list::const_iterator it_x = x.begin();
		instructions_list::const_iterator it_y = y.begin();

		while(it_x != x.end()){
			ret = are_equal_quad_pointers(*it_x, *it_y);
			if(!ret){
				break;
			}
			else{
				// { ret }
				it_x++;
				it_y++;
			}
		}
	}
	else{
		// {x.size() != y.size()}
		ret = false;
	}

	return ret;
}

std::string print_operand(const address_pointer& operand){
	std::string ret;

	switch(operand->type){
		case address_type::ADDRESS_NAME:
			ret = *(operand->value).name;
			break;

		case address_type::ADDRESS_CONSTANT:
			switch(operand->value.constant.type){
				case value_type::BOOLEAN:
					ret = BOOL_STR(operand->value.constant.val.bval);
					break;

				case value_type::INTEGER:
					ret = std::to_string(operand->value.constant.val.ival);
					break;

				default:
					// {operand->value.constant.type == value_type::FLOAT}
					ret = std::to_string(operand->value.constant.val.fval);
			}
			break;

		case address_type::ADDRESS_TEMP:
			ret = std::to_string(operand->value.temp);
			break;

		default:
			// {operand->type == ADDRESS_LABEL}
			ret = *(operand->value.label);
	}

	return ret;
}

/*
 * PRE : {inst is a binary assign. instruction.}
 * */
std::string print_binary_assign(const quad_pointer& inst){
	std::string result = print_operand(inst->result);
	std::string operand_1 = print_operand(inst->arg1);
	std::string operand_2 = print_operand(inst->arg2);
	std::string op;

	switch(inst->op){
		case quad_oper::TIMES:
			op = " * ";
			break;

		case quad_oper::DIVIDE:
			op = " / ";
			break;

		case quad_oper::MOD:
			op = " % ";
			break;

		case quad_oper::PLUS:
			op = " + ";
			break;

		default:
			// {inst->op == quad_oper::MINUS}
			#ifdef __DEBUG
				assert(inst->op == quad_oper::MINUS);
			#endif
			op = " - ";
	}

	return result + " = " + operand_1 + op + operand_2;
}

/*
 * PRE : {inst is an unary assign. instruction.}
 * */
std::string print_unary_assign(const quad_pointer& inst){
	std::string result = print_operand(inst->result);
	std::string operand = print_operand(inst->arg1);
	std::string op;

	switch(inst->op){
		case quad_oper::NEGATIVE:
			op = " - ";
			break;

		default:
			// {inst->op == case quad_oper::NEGATION}
			#ifdef __DEBUG
				assert(inst->op == quad_oper::NEGATION);
			#endif
			op = " not ";
	}

	return result + " = " + op + operand;
}

/*
 * PRE : {inst is a copy instruction.}
 * */
std::string print_copy_inst(const quad_pointer& inst){
	std::string result = print_operand(inst->result);
	std::string ret;

	switch(inst->type){
		case quad_type::COPY:
			ret = result + " = " + print_operand(inst->arg1);
			break;

		case quad_type::INDEXED_COPY_TO:
			ret = result + "[" + print_operand(inst->arg1) + "] = " +
				  print_operand(inst->arg2);
			break;

		default:
			// {inst->type == quad_type::INDEXED_COPY_FROM}
			#ifdef __DEBUG
				assert(inst->type == quad_type::INDEXED_COPY_FROM);
			#endif
				ret = result + "= " + print_operand(inst->arg1) + "[" +
						print_operand(inst->arg2) + "]";
	}

	return ret;
}

/*
 * PRE : {inst is a jump instruction.}
 * */
std::string print_jump_inst(const quad_pointer& inst){
	std::string ret;

	switch(inst->type){
		case quad_type::UNCONDITIONAL_JUMP:
			ret = "goto " + print_operand(inst->arg1);
			break;

		case quad_type::CONDITIONAL_JUMP:
			switch(inst->op){
				case quad_oper::IFTRUE:
					ret = "ifTrue " + print_operand(inst->arg1) + " goto " +
									  print_operand(inst->arg2);
					break;

				default:
					// {inst->op == quad_oper::IFFALSE}
					#ifdef __DEBUG
						assert(inst->op == quad_oper::IFFALSE);
					#endif

					ret = "ifFalse " + print_operand(inst->arg1) + " goto " +
						print_operand(inst->arg2);
			}
			break;

		default:
			// {inst->type == quad_type::RELATIONAL_JUMP}
			#ifdef __DEBUG
				assert(inst->type == quad_type::RELATIONAL_JUMP);
			#endif
			switch(inst->op){
				case quad_oper::LESS:
					ret = "if " + print_operand(inst->arg1) + " < " +
						print_operand(inst->arg2) + " goto " +
						print_operand(inst->result);
					break;

				case quad_oper::LESS_EQUAL:
					ret = "if " + print_operand(inst->arg1) + " <= " +
						print_operand(inst->arg2) + " goto " +
						print_operand(inst->result);
					break;

				case quad_oper::GREATER:
					ret = "if " + print_operand(inst->arg1) + " > " +
						print_operand(inst->arg2) + " goto " +
						print_operand(inst->result);
					break;

				case quad_oper::GREATER_EQUAL:
					ret = "if " + print_operand(inst->arg1) + " >= " +
						print_operand(inst->arg2) + " goto " +
						print_operand(inst->result);
					break;

				case quad_oper::EQUAL:
					ret = "if " + print_operand(inst->arg1) + " == " +
						print_operand(inst->arg2) + " goto " +
						print_operand(inst->result);
					break;

				default:
					// {inst->op == quad_oper::DISTINCT}
					#ifdef __DEBUG
						assert(inst->op == quad_oper::DISTINCT);
					#endif

					ret = "if " + print_operand(inst->arg1) + " != " +
						print_operand(inst->arg2) + " goto " +
						print_operand(inst->result);
			}
	}

	return ret;
}

/*
 * PRE : {inst is a parameter instruction.}
 * */
std::string print_param_inst(const quad_pointer& inst){
	return "param " + print_operand(inst->arg1);
}

/*
 * PRE : {inst is a method call instruction.}
 * */
std::string print_method_call_inst(const quad_pointer& inst){
	std::string ret;

	switch(inst->type){
		case quad_type::PROCEDURE_CALL:
			ret = "call " + print_operand(inst->arg1) + ", " +
				print_operand(inst->arg2);
			break;

		default:
			// {inst->type == quad_type::FUNCTION_CALL}
			#ifdef __DEBUG
				assert(inst->type == quad_type::FUNCTION_CALL);
			#endif
			ret = print_operand(inst->result) + " = call " +
				print_operand(inst->arg1) + ", " + print_operand(inst->arg2);
	}
	return ret;
}

/*
 * PRE : {inst is a return instruction.}
 * */
std::string print_return_inst(const quad_pointer& inst){
	std::string ret = "return";

	if(inst->arg1 != nullptr){
		ret += " " + print_operand(inst->arg1);
	}

	return ret;
}

/*
 * PRE : {inst is a label instruction.}
 * */
std::string print_label_inst(const quad_pointer& inst){
	return print_operand(inst->arg1) + ":";
}

/*
 * PRE : {inst is an enter instruction.}
 * */
std::string print_enter_inst(const quad_pointer& inst){
	return "enter " + print_operand(inst->arg1);
}

std::string instruction_to_string(const quad_pointer& inst){
	std::string ret;

	switch(inst->type){
		case quad_type::BINARY_ASSIGN:
			ret = print_binary_assign(inst);
			break;

		case quad_type::UNARY_ASSIGN:
			ret = print_unary_assign(inst);
			break;

		case quad_type::COPY:
			ret = print_copy_inst(inst);
			break;

		case quad_type::INDEXED_COPY_TO:
			ret = print_copy_inst(inst);
			break;

		case quad_type::INDEXED_COPY_FROM:
			ret = print_copy_inst(inst);
			break;

		case quad_type::UNCONDITIONAL_JUMP:
			ret = print_jump_inst(inst);
			break;

		case quad_type::CONDITIONAL_JUMP:
			ret = print_jump_inst(inst);
			break;

		case quad_type::RELATIONAL_JUMP:
			ret = print_jump_inst(inst);
			break;

		case quad_type::PARAMETER:
			ret = print_param_inst(inst);
			break;

		case quad_type::PROCEDURE_CALL:
			ret = print_method_call_inst(inst);
			break;

		case quad_type::FUNCTION_CALL:
			ret = print_method_call_inst(inst);
			break;

		case quad_type::RETURN:
			ret = print_return_inst(inst);
			break;

		case quad_type::LABEL:
			ret = print_label_inst(inst);
			break;

		default:
			// {inst->type == quad_type::ENTER_PROCEDURE}
			ret = print_enter_inst(inst);
	}

	return ret;
}

std::string instructions_list_to_string(const instructions_list& x){
	std::string ret;

	for(instructions_list::const_iterator it = x.begin();
	it != x.end();it++){

		ret += instruction_to_string(*it) + "\n";
	}

	return ret;
}
