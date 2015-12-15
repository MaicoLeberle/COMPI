#include "test_asm_code_generator.h"

/*extern instructions_list *ir_code;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int irparse();
extern YY_BUFFER_STATE ir_scan_string(const char * str);
extern void ir_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void ir_delete_buffer(YY_BUFFER_STATE buffer);

void translate(std::string ir_program){
	YY_BUFFER_STATE program_buffer = ir_scan_string(ir_program.c_str());
	ir_switch_to_buffer(program_buffer);

	irparse();

	ir_delete_buffer(program_buffer);
}*/
extern instructions_list *ir_code;
extern void translate_ir_code(std::string program);

extern asm_instructions_list *asm_code;

extern void translate_asm_code(std::string program);

ids_info *s_table = nullptr; // Global symbols' table.

void set_symbol_table(){
	s_table = new ids_info();
	s_table->register_var(std::string("x"), -4, T_UNDEFINED);
	s_table->register_var(std::string("y"), 4, T_UNDEFINED);

	// Register information about "method" and "class".
	std::string method("method");
	std::string param("param1");
	s_table->register_class(std::string("class"), t_attributes());
	s_table->register_method(method, 0, std::string("class"));

	// "method" has an integer parameter.
	s_table->get_list_params(method).push_back(param);
	s_table->register_var(param, -4, T_INT);

	// Register information about an instance "obj".
	s_table->register_obj(std::string("obj"),
							8,
							std::string("class"),
							std::string("obj.att1"));

	s_table->register_var(std::string("obj.att1"), 0, id_type::T_INT);
}

void test_binary_assign(){
	std::cout << "1) Translation of binary assign: ";

	////////////////////////
	// PLUS
	////////////////////////

	// x = 2 + 1
	translate_ir_code(std::string("class.method:"
									"x = 2 + 1"));

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "class.method:"
									"movl $2, %r8d\n"
									"addl $1, %r8d\n"
									"movl %r8d, -4(%rbp)";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));


	////////////////////////
	// MINUS
	////////////////////////
	// x = y - z
	translate_ir_code(std::string("class.method:"
									"x = 2 - 1"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"movl $2, %r8d\n"
						"subl $1, %r8d\n"
						"movl %r8d, -4(%rbp)";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// TIMES
	////////////////////////
	// x = y * z
	translate_ir_code(std::string("class.method:"
									"x = 2 * 1"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"movl $2, %r8d\n"
						"imull $1, %r8d\n"
						"movl %r8d, -4(%rbp)";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// DIVIDE
	////////////////////////
	// x = y / z
	translate_ir_code(std::string("class.method:"
									"x = 2 / 1"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = 	"class.method:"
						"movl %edx, %r8d\n"
						"movl %eax, %r9d\n"
						"movl $2, %eax\n"
						"movl $0, %edx\n"
						"idivl $1\n"
						"movl %eax, -4(%rbp)\n"
						"movl %r8d, %edx\n"
						"movl %r9d, %eax\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// MOD
	////////////////////////
	// x = y % z
	translate_ir_code(std::string("class.method:"
									"x = 2 % 1"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = 	"class.method:"
						"movl %edx, %r8d\n"
						"movl %eax, %r9d\n"
						"movl $2, %eax\n"
						"movl $0, %edx\n"
						"idivl $1\n"
						"movl %edx, -4(%rbp)\n"
						"movl %r8d, %edx\n"
						"movl %r9d, %eax\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_unary_assign(){
	std::cout << "2) Translation of unary assign: ";

	////////////////////////
	// NEGATIVE
	////////////////////////

	// x = - 1
	translate_ir_code(std::string("class.method:"
									"x = - 1"));

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "class.method:"
									"movl $1, %r8d\n"
									"negl %r8d\n"
									"movl %r8d, -4(%rbp)\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// NEGATION
	////////////////////////

	// x = not y
	translate_ir_code(std::string("class.method:"
									"x = not true"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"movl $1, %r8d\n"
						"notl %r8d\n"
						"movl %r8d, -4(%rbp)\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_copy(){
	std::cout << "3) Translation of copy instruction: ";

	////////////////////////
	// COPY
	////////////////////////

	// x = y
	translate_ir_code(std::string("class.method:"
									"x = 1"));
	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "class.method:"
									"movl $1, -4(%rbp)\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_jmp(){
	std::cout << "4) Translation of jump instructions: ";

	////////////////////////
	// UNCONDITIONAL JUMP
	////////////////////////

	// goto L
	translate_ir_code(std::string("class.method:"
									"goto L"));
	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "class.method:"
									"jmp L\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// CONDITIONAL JUMP
	////////////////////////

	//
	// IfTrue
	//
	translate_ir_code(std::string("class.method:"
									"ifTrue true goto L"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"movl $1 , %r8d\n"
						"cmpl $1 , %r8d\n"
						"je L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	//
	// IfFalse
	//
	translate_ir_code(std::string("class.method:"
									"ifFalse true goto L"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"movl $1 , %r8d\n"
						"cmpl $1 , %r8d\n"
						"jne L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// RELATIONAL JUMP
	////////////////////////

	//
	// Less
	//
	translate_ir_code(std::string("class.method:"
									"if 1 < 2 goto L"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"cmpl $1 , $2\n"
						"jl L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	//
	// Less or equal
	//
	translate_ir_code(std::string("class.method:"
									"if 1 <= 2 goto L"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"cmpl $1 , $2\n"
						"jle L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	//
	// Greater
	//
	translate_ir_code(std::string("class.method:"
									"if 1 > 2 goto L"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"cmpl $1 , $2\n"
						"jg L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	//
	// Greater or equal
	//
	translate_ir_code(std::string("class.method:"
									"if 1 >= 2 goto L"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"cmpl $1 , $2\n"
						"jge L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_parameter_and_call(){
	std::cout << "5) Translation of param and instructions: ";

	// param x
	// call p,n
	// 8 integer parameters
	translate_ir_code(std::string("class.method:"
								"param 1"
								"param 2"
								"param 3"
								"param 4"
								"param 5"
								"param 6"
								"param 7"
								"param 8"
								"call class.method, 8"));

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "class.method:"
									"movl $1, %rdi\n"
									"movl $2, %rsi\n"
									"movl $3, %rdx\n"
									"movl $4, %rcx\n"
									"movl $5, %r8\n"
									"movl $6, %r9\n"
									"pushq $8\n"
									"pushq $7\n"
									"movl $0, %rax\n"
									"call class.method";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	// param x
	// y = call p,n
	// 8 integer parameters
	translate_ir_code(std::string("class.method:"
								"param 1"
								"param 2"
								"param 3"
								"param 4"
								"param 5"
								"param 6"
								"param 7"
								"param 8"
								"y = call x,8"));

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "class.method:"
						"movl $1, %rdi\n"
						"movl $2, %rsi\n"
						"movl $3, %rdx\n"
						"movl $4, %rcx\n"
						"movl $5, %r8\n"
						"movl $6, %r9\n"
						"pushq $8\n"
						"pushq $7\n"
						"movl $0, %rax\n"
						"call x\n"
						"movl %rax, 4(%rbp)";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_enter(){
	std::cout << "6) Translation of the enter instruction: ";

	// enter x
	translate_ir_code(std::string("class.method:\n"
								"enter 4"));

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	// 0 levels of nesting.
	std::string asm_program_text = "class.method:"
									"enter $8, $0\n"
									"movl %rdi, -4(%rbp)";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_return(){
	std::cout << "7) Translation of the return instruction: ";

	// return x
	translate_ir_code(std::string("class.method:"
									"return 1"));

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "class.method:"
									"movl $1, %rax\n"
									"leave\n"
									"ret";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_objects_and_arrays(){
	std::cout << "8) Translation of objects and arrays: ";

	// Object creation.
	// TODO: los offsets tendrían que ser negativos.
	translate_ir_code(std::string("class.method:"
									"obj[0] = 0\n"
									"obj[4] = 0\n"
									"obj = &obj.att1\n"
									"param1 = obj[0]"));

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "class.method:\n"
									"movl 8(%rbp) , %rdi\n"
									"movl $0 , 0(%rdi)\n"
									"movl 8(%rbp) , %rdi\n"
									"movl $0 , 4(%rdi)\n"
									"leal 0(%rbp) , 8(%rbp)\n"
									"movl 8(%rbp) , %rdi\n"
									"movl 0(%rdi) , -4(%rbp)\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_asm_code_generator(){
	std::cout << "\nTesting assembly code generation:" << std::endl;
	set_symbol_table();
	test_binary_assign();
	test_unary_assign();
	test_copy();
	test_jmp();
	test_parameter_and_call();
	test_enter();
	test_return();
	test_objects_and_arrays();
}
