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

address_pointer new_name_address(std::string name){
	address_pointer addr = address_pointer(new address);
	addr->type = address_type::ADDRESS_NAME;
	addr->value.name = new std::string(name);

	return addr;
}

address_pointer new_label_address(std::string label){
	address_pointer addr = address_pointer(new address);
	addr->type = address_type::ADDRESS_LABEL;
	addr->value.label = new std::string(label);

	return addr;
}

quad_pointer new_label(const std::string& label){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::LABEL;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_LABEL;
	instruction->result->value.label = new std::string(label); // TODO: Hay que liberar esta memoria

	return instruction;
}

quad_pointer new_copy(const address_pointer dest, const address_pointer orig){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::COPY;
	instruction->result = dest;
	instruction->arg1 = orig;

	return instruction;
}

quad_pointer new_enter_procedure(const unsigned int nmbr){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::ENTER_PROCEDURE;
	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_CONSTANT;
	instruction->arg1->value.constant.type = value_type::INTEGER;
	instruction->arg1->value.constant.val.ival = nmbr;

	return instruction;
}

quad_pointer new_binary_assign(const address_pointer dest,
							   const address_pointer arg1,
							   const address_pointer arg2,
							   quad_oper op){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::BINARY_ASSIGN;
	instruction->op = op;
	instruction->arg1 = arg1;
	instruction->arg2 = arg2;
	instruction->result = dest;

	return instruction;
}

quad_pointer new_parameter_inst(const address_pointer param){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::PARAMETER;
	instruction->op = quad_oper::NONE;
	instruction->arg1 = param;

	return instruction;
}

quad_pointer new_function_call_inst(const address_pointer dest,
									const address_pointer func_label,
									const address_pointer param_quantity){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::FUNCTION_CALL;
	instruction->op = quad_oper::NONE;
	instruction->result = dest;
	instruction->arg1 = func_label;
	instruction->arg2 = param_quantity;

	return instruction;
}

quad_pointer new_procedure_call_inst(const address_pointer proc_label,
									const address_pointer param_quantity){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::PROCEDURE_CALL;
	instruction->op = quad_oper::NONE;
	instruction->arg1 = proc_label;
	instruction->arg2 = param_quantity;

	return instruction;
}

quad_pointer new_conditional_jump_inst(const address_pointer guard,
										std::string label,
										quad_oper op){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CONDITIONAL_JUMP;
	instruction->op = op;
	instruction->arg1 = guard;
	instruction->arg2 = new_label_address(label);

	return instruction;
}

quad_pointer new_unconditional_jump_inst(std::string label){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::UNCONDITIONAL_JUMP;
	instruction->op = quad_oper::NONE;
	instruction->arg1 = new_label_address(label);

	return instruction;
}

quad_pointer new_relational_jump_inst(const address_pointer x, const address_pointer y,
								quad_oper relop, std::string label){

	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::RELATIONAL_JUMP;
	instruction->op = relop;
	instruction->arg1 = x;
	instruction->arg2 = y;
	instruction->result = new_label_address(label);
	return instruction;
}

bool is_label(const quad_pointer& instruction, const std::string& label){
	return instruction->type == quad_type::LABEL &&
		   instruction->op == quad_oper::NONE &&
		   instruction->result->type == address_type::ADDRESS_LABEL &&
		   *(instruction->result->value.label) == label;
}

bool is_enter_procedure(const quad_pointer& instruction, unsigned int bytes){
	return instruction->type == quad_type::ENTER_PROCEDURE &&
			   instruction->op == quad_oper::NONE &&
			   instruction->arg1->type == address_type::ADDRESS_CONSTANT &&
			   instruction->arg1->value.constant.val.ival == bytes;
}

bool is_procedure_call(const quad_pointer& instruction,
						const address_pointer proc_label,
						int param_quantity){

	return instruction->type == quad_type::PROCEDURE_CALL &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_pointers(instruction->arg1, proc_label) &&
		   are_equal_pointers(instruction->arg2, new_integer_constant(param_quantity));
}

bool is_function_call(const quad_pointer& instruction,
						const address_pointer dest,
						const address_pointer proc_label,
						const address_pointer param_quantity){

	return instruction->type == quad_type::PROCEDURE_CALL &&
				   instruction->op == quad_oper::NONE &&
				   are_equal_pointers(instruction->arg1, proc_label) &&
				   are_equal_pointers(instruction->arg2, param_quantity) &&
				   are_equal_pointers(instruction->result, dest);
}

bool is_copy(const quad_pointer& instruction, const address_pointer& dest,
		const address_pointer& orig){
	// TODO: revisar si el orden de arg1 y result es correcto
	// TODO: para usar este procedimiento va a ser preciso indicar el tipo
	// de x e y (si son temporales o identificadores del código). Quizás
	// debería recibir un par de address
	return instruction->type == quad_type::COPY &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_pointers(instruction->arg1, orig) &&
		   are_equal_pointers(instruction->result, dest);
}

bool is_binary_assignment(const quad_pointer& instruction,
						 const address_pointer& dest,
						 const address_pointer& arg1,
						 const address_pointer& arg2,
						 quad_oper op){

	return instruction->type == quad_type::BINARY_ASSIGN &&
		   instruction->op == op &&
		   are_equal_pointers(instruction->arg1, arg1) &&
		   are_equal_pointers(instruction->arg2, arg2) &&
		   are_equal_pointers(instruction->result, dest);
}

bool is_parameter_inst(const quad_pointer& instruction,
						const address_pointer param){

	// TODO: se supone que si estamos creando las instrucciones con los
	// correspondientes constructores, instruction->op sí o sí es NONE...
	return instruction->type == quad_type::PARAMETER &&
		   instruction->op == quad_oper::NONE &&
		   are_equal_pointers(instruction->arg1, param);
}

bool is_conditional_jump_inst(const quad_pointer& instruction,
								const address_pointer guard,
								std::string label,
								quad_oper op){

	// TODO: se supone que si estamos creando las instrucciones con los
	// correspondientes constructores, instruction->arg2 sí o sí es un label
	return instruction->type == quad_type::CONDITIONAL_JUMP &&
			instruction->op == op &&
			are_equal_pointers(instruction->arg1, guard) &&
			*instruction->arg2->value.label == label;
}

bool is_unconditional_jump_inst(const quad_pointer& instruction,
								std::string label){

	// TODO: se supone que si estamos creando las instrucciones con los
	// correspondientes constructores, instruction->arg1 sí o sí es un label
	return instruction->type == quad_type::UNCONDITIONAL_JUMP &&
			*instruction->arg1->value.label == label;
}

bool is_relational_jump_inst(const quad_pointer& instruction,
							const address_pointer x, const address_pointer y,
							quad_oper relop, std::string label){

	return instruction->type == quad_type::RELATIONAL_JUMP &&
				instruction->op == relop &&
				are_equal_pointers(instruction->arg1, x) &&
				are_equal_pointers(instruction->arg2, y) &&
				*instruction->result->value.label == label;
}

// TODO: cambiar por are_equal_addresses
bool are_equal_pointers(const address_pointer& x, const address_pointer& y){
	bool ret = false;

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

	return ret;
}
