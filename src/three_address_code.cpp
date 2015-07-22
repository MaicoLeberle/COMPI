#include "three_address_code.h"

address_pointer new_integer_constant(int value){
	address_pointer address = address_pointer(new address);
	address->type = address_type::ADDRESS_CONSTANT;
	address->value.constant.val.ival = value;

	return address;
}

address_pointer new_float_constant(float value){
	address_pointer address = address_pointer(new address);
	address->type = address_type::ADDRESS_CONSTANT;
	address->value.constant.val.fval = value;

	return address;
}

address_pointer new_boolean_constant(bool value){
	address_pointer address = address_pointer(new address);
	address->type = address_type::ADDRESS_CONSTANT;
	address->value.constant.val.bval = value;

	return address;
}

quad_pointer new_label(const std::string& label){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::LABEL;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_LABEL;
	instruction->result->value.label = new std::string(label); // TODO: Hay que liberar esta memoria

	return instruction;
}

quad_pointer new_copy(const std::string& label, const address_pointer arg){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::COPY;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(label);
	instruction->arg1 = arg;

	return instruction;
}

quad_pointer new_int_field(const std::string& id){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CREATE_INT;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(id); // TODO: Hay que liberar esta memoria
	instruction->arg1 = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_CONSTANT;
	instruction->result->value.constant.val.ival = 0; // TODO: valor inicial?

	return instruction;
}

quad_pointer new_float_field(const std::string& id){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CREATE_FLOAT;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(id); // TODO: Hay que liberar esta memoria
	instruction->arg1 = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_CONSTANT;
	instruction->result->value.constant.val.fval = 0.0; // TODO: valor inicial?

	return instruction;
}

quad_pointer new_boolean_field(const std::string& id){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CREATE_BOOLEAN;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(id); // TODO: Hay que liberar esta memoria
	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_CONSTANT;
	instruction->arg1->value.constant.val.bval = false; // TODO: valor inicial?

	return instruction;
}

quad_pointer new_instance_field(const std::string& id, const std::string& class_name){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CREATE_ID;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(id);
	// TODO: así?
	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_NAME;
	instruction->arg1->value.name = new std::string(class_name);

	return instruction;
}

quad_pointer new_int_array_field(const std::string& id, const int dimension){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CREATE_INT_ARRAY;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(id);
	// TODO: así?
	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_CONSTANT;
	instruction->arg1->value.constant.val.dimension = dimension; // TODO: valor inicial?

	return instruction;
}

quad_pointer new_float_array_field(const std::string& id, const int dimension){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CREATE_FLOAT_ARRAY;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(id);
	// TODO: así?
	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_CONSTANT;
	instruction->arg1->value.constant.val.dimension = dimension; // TODO: valor inicial?

	return instruction;
}

quad_pointer new_boolean_array_field(const std::string& id, const int dimension){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::CREATE_BOOLEAN_ARRAY;
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_CONSTANT;
	instruction->result->value.label = new std::string(id);
	// TODO: así?
	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_NAME;
	instruction->arg1->value.constant.val.dimension = dimension; // TODO: valor inicial?

	return instruction;
}
/*
quad_pointer new_copy(const std::string& l_value, const std::string& r_value){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::COPY;

	// TODO: en qué lugar ubicamos a x e y?
	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(l_value); // TODO: Hay que liberar esta memoria

	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_NAME;
	instruction->arg1->value.label = new std::string(r_value);

	return instruction;
}

quad_pointer new_plus_assign(const std::string& l_value, const std::string& r_value){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::BINARY_ASSIGN;
	instruction->op = quad_oper::PLUS;

	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(l_value);

	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_NAME;
	instruction->arg1->value.label = new std::string(l_value);

	instruction->arg2 = address_pointer(new address);
	instruction->arg2->type = address_type::ADDRESS_NAME;
	instruction->arg2->value.label = new std::string(r_value);

	return instruction;
}

quad_pointer new_minus_assign(const std::string& l_value, const std::string& r_value){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::BINARY_ASSIGN;
	instruction->op = quad_oper::MINUS;

	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(l_value);

	instruction->arg1 = address_pointer(new address);
	instruction->arg1->type = address_type::ADDRESS_NAME;
	instruction->arg1->value.label = new std::string(l_value);

	instruction->arg2 = address_pointer(new address);
	instruction->arg2->type = address_type::ADDRESS_NAME;
	instruction->arg2->value.label = new std::string(r_value);

	return instruction;
}

quad_pointer new_parameter(const std::string& address){
	quad_pointer instruction = quad_pointer(new quad);
	instruction->type = quad_type::PARAMETER;
	instruction->op = quad_oper::NONE;

	instruction->result = address_pointer(new address);
	instruction->result->type = address_type::ADDRESS_NAME;
	instruction->result->value.label = new std::string(address);

	return instruction;
}*/

bool is_label(const quad_pointer& instruction, const std::string& label){
	return instruction->type == quad_type::LABEL &&
		   instruction->op == quad_oper::NONE &&
		   instruction->result->type == address_type::ADDRESS_LABEL &&
		   *(instruction->result->value.label) == label;
}
