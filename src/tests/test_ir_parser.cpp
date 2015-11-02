#include "test_ir_parser.h"

extern instructions_list *ir_code;
/*typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int irparse();
extern YY_BUFFER_STATE ir_scan_string(const char * str);
extern void ir_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void ir_delete_buffer(YY_BUFFER_STATE buffer);*/
extern void translate_ir_code(std::string program);
/*void translate_ir_code(std::string program){
	YY_BUFFER_STATE program_buffer = ir_scan_string(program.c_str());
	ir_switch_to_buffer(program_buffer);

	irparse();

	ir_delete_buffer(program_buffer);
}*/

void test_binary_assignment_translation(){
	std::cout << "1) Translation of binary assignments:";

	////////////////////////
	// PLUS
	////////////////////////
	translate_ir_code(std::string("X = Y + Z"));

	// TODO: borrar todo lo que estoy metiendo en el heap.

	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_binary_assignment(*it,
							new_name_address(std::string("X")),
							new_name_address(std::string("Y")),
							new_name_address(std::string("Z")),
							quad_oper::PLUS));

	// Using some constants as values.
	translate_ir_code(std::string("X = 1 + 2"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_binary_assignment(*it,
							new_name_address(std::string("X")),
							new_integer_constant(1),
							new_integer_constant(2),
							quad_oper::PLUS));

	////////////////////////
	// MINUS
	////////////////////////
	translate_ir_code(std::string("X = Y - Z"));
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_binary_assignment(*it,
							new_name_address(std::string("X")),
							new_name_address(std::string("Y")),
							new_name_address(std::string("Z")),
							quad_oper::MINUS));

	////////////////////////
	// TIMES
	////////////////////////
	translate_ir_code(std::string("X = Y * Z"));
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_binary_assignment(*it,
							new_name_address(std::string("X")),
							new_name_address(std::string("Y")),
							new_name_address(std::string("Z")),
							quad_oper::TIMES));

	////////////////////////
	// DIVIDE
	////////////////////////
	translate_ir_code(std::string("X = Y / 2.2"));
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_binary_assignment(*it,
							new_name_address(std::string("X")),
							new_name_address(std::string("Y")),
							new_float_constant(2.2),
							quad_oper::DIVIDE));

	////////////////////////
	// MOD
	////////////////////////
	translate_ir_code(std::string("X = Y % 2"));
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_binary_assignment(*it,
							new_name_address(std::string("X")),
							new_name_address(std::string("Y")),
							new_integer_constant(2),
							quad_oper::MOD));

	std::cout << "OK. " << std::endl;
}

void test_unary_assignment_translation(){
	std::cout << "2) Translation of unary assignments:";

	////////////////////////
	// NEGATIVE
	////////////////////////
	translate_ir_code(std::string("X = - Y"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_unary_assignment(*it,
							new_name_address(std::string("X")),
							new_name_address(std::string("Y")),
							quad_oper::NEGATIVE));

	////////////////////////
	// NEGATION
	////////////////////////
	translate_ir_code(std::string("X = not Y"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_unary_assignment(*it,
							new_name_address(std::string("X")),
							new_name_address(std::string("Y")),
							quad_oper::NEGATION));

	std::cout << "OK. " << std::endl;
}

void test_copy_translation(){
	std::cout << "3) Translation of copy instruction:";

	translate_ir_code(std::string("X = Y"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_copy(*it,
				   new_name_address(std::string("X")),
				   new_name_address(std::string("Y"))));

	// Copying an integer.
	translate_ir_code(std::string("X = 1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_copy(*it,
				   new_name_address(std::string("X")),
				   new_integer_constant(1)));

	translate_ir_code(std::string("X = 1"));

	// Copying a boolean.
	translate_ir_code(std::string("X = true"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_copy(*it,
				   new_name_address(std::string("X")),
				   new_boolean_constant(true)));

	std::cout << "OK. " << std::endl;
}

void test_indexed_copy_to_translation(){
	std::cout << "4) Translation of indexed copy to instruction:";

	translate_ir_code(std::string("X[1] = Y"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_indexed_copy_to(*it,
				   new_name_address(std::string("X")),
				   new_integer_constant(1),
				   new_name_address(std::string("Y"))));

	std::cout << "OK. " << std::endl;
}

void test_indexed_copy_from_translation(){
	std::cout << "5) Translation of indexed copy from instruction:";

	translate_ir_code(std::string("X = Y[1]"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_indexed_copy_from(*it,
				   new_name_address(std::string("X")),
				   new_name_address(std::string("Y")),
				   new_integer_constant(1)));

	std::cout << "OK. " << std::endl;
}

void test_jump_translation(){
	std::cout << "6) Translation of jump instructions:";

	///////////////////////////////////
	// UNCONDITIONAL JUMP
	///////////////////////////////////
	translate_ir_code(std::string("goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_unconditional_jump_inst(*it,std::string("L1")));

	///////////////////////////////////
	// CONDITIONAL JUMP
	///////////////////////////////////
	// iftrue conditional jump
	translate_ir_code(std::string("ifTrue x goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_conditional_jump_inst(*it,
							new_name_address(std::string("x")),
							std::string("L1"),
							quad_oper::IFTRUE));

	// iffalse unconditional jump
	translate_ir_code(std::string("ifFalse x goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_conditional_jump_inst(*it,
							new_name_address(std::string("x")),
							std::string("L1"),
							quad_oper::IFFALSE));

	///////////////////////////////////
	// RELATIONAL JUMP
	///////////////////////////////////

	// less than conditional jump.
	translate_ir_code(std::string("if x < y goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_relational_jump_inst(*it,
							new_name_address(std::string("x")),
							new_name_address(std::string("y")),
							quad_oper::LESS,
							std::string("L1")));

	// less or equal than conditional jump.
	translate_ir_code(std::string("if x <= y goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_relational_jump_inst(*it,
							new_name_address(std::string("x")),
							new_name_address(std::string("y")),
							quad_oper::LESS_EQUAL,
							std::string("L1")));

	// greater than conditional jump.
	translate_ir_code(std::string("if x > y goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_relational_jump_inst(*it,
							new_name_address(std::string("x")),
							new_name_address(std::string("y")),
							quad_oper::GREATER,
							std::string("L1")));

	// greater or equal than conditional jump.
	translate_ir_code(std::string("if x >= y goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_relational_jump_inst(*it,
							new_name_address(std::string("x")),
							new_name_address(std::string("y")),
							quad_oper::GREATER_EQUAL,
							std::string("L1")));

	// equal conditional jump.
	translate_ir_code(std::string("if x == y goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_relational_jump_inst(*it,
							new_name_address(std::string("x")),
							new_name_address(std::string("y")),
							quad_oper::EQUAL,
							std::string("L1")));

	// distinct conditional jump.
	translate_ir_code(std::string("if x != y goto L1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_relational_jump_inst(*it,
							new_name_address(std::string("x")),
							new_name_address(std::string("y")),
							quad_oper::DISTINCT,
							std::string("L1")));

	std::cout << "OK. " << std::endl;
}

void test_param_translation(){
	std::cout << "7) Translation of parameter instruction:";

	translate_ir_code(std::string("param x"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_parameter_inst(*it,
				   new_name_address(std::string("x"))));

	std::cout << "OK. " << std::endl;
}

void test_procedure_call_translation(){
	std::cout << "8) Translation of procedure call instruction:";

	/////////////////////////////////
	// PROCEDURE CALL
	/////////////////////////////////
	translate_ir_code(std::string("call x , 1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_procedure_call(*it,
				   new_label_address(std::string("x")),
				   1));

	/////////////////////////////////
	// FUNCTION CALL
	/////////////////////////////////
	translate_ir_code(std::string("y = call x , 1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_function_call(*it,
				   new_name_address(std::string("y")),
				   new_label_address(std::string("x")),
				   new_integer_constant(1)));

	std::cout << "OK. " << std::endl;
}

void test_return_translation(){
	std::cout << "9) Translation of return instruction:";

	// Empty return.
	translate_ir_code(std::string("return"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_return_inst(*it, nullptr));

	// Non-empty return.
	translate_ir_code(std::string("return x"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	it = ir_code->begin();
	assert(is_return_inst(*it, new_name_address(std::string("x"))));

	std::cout << "OK. " << std::endl;
}

void test_label_translation(){
	std::cout << "10) Translation of label instruction:";

	// Empty return.
	translate_ir_code(std::string("x:"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_label(*it, std::string("x")));

	std::cout << "OK. " << std::endl;
}

void test_enter_translation(){
	std::cout << "11) Translation of enter instruction:";

	// Empty return.
	translate_ir_code(std::string("enter 1"));

	// TODO: borrar todo lo que estoy metiendo en el heap.
	assert(ir_code->size() == 1);
	instructions_list::iterator it = ir_code->begin();
	assert(is_enter_procedure(*it, 1));

	std::cout << "OK. " << std::endl;
}

void test_ir_parser(){
	std::cout << "\nTesting intermediate code's parser:" << std::endl;
	test_binary_assignment_translation();
	test_unary_assignment_translation();
	test_copy_translation();
	test_indexed_copy_to_translation();
	test_indexed_copy_from_translation();
	test_jump_translation();
	test_param_translation();
	test_procedure_call_translation();
	test_return_translation();
	test_label_translation();
	test_enter_translation();
}
