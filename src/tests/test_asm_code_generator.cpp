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

void test_binary_assign(){
	std::cout << "1) Translation of binary assign: ";

	////////////////////////
	// PLUS
	////////////////////////

	// x = 2 + 1
	std::string var("x");
	translate_ir_code(std::string("x = 2 + 1"));
	ids_info *s_table = new ids_info();
	s_table->register_var(var, -4, T_UNDEFINED);

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "movl $2, %r8d\n"
									"addl $1, %r8d\n"
									"movl %r8d, -4(%rbp)\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));


	////////////////////////
	// MINUS
	////////////////////////
	// x = y - z
	translate_ir_code(std::string("x = 2 - 1"));

	s_table = new ids_info();
	s_table->register_var(var, -4, T_UNDEFINED);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "movl $2, %r8d\n"
						"subl $1, %r8d\n"
						"movl %r8d, -4(%rbp)";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// TIMES
	////////////////////////
	// x = y * z
	translate_ir_code(std::string("x = 2 * 1"));

	s_table = new ids_info();
	s_table->register_var(var, -4, T_UNDEFINED);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "movl $2, %r8d\n"
						"imull $1, %r8d\n"
						"movl %r8d, -4(%rbp)";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// DIVIDE
	////////////////////////
	// x = y / z
	translate_ir_code(std::string("x = 2 / 1"));

	s_table = new ids_info();
	s_table->register_var(var, -4, T_UNDEFINED);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = 	"movl %edx, %r8d\n"
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
	translate_ir_code(std::string("x = 2 % 1"));

	s_table = new ids_info();
	s_table->register_var(var, -4, T_UNDEFINED);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = 	"movl %edx, %r8d\n"
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
	std::string var("x");
	translate_ir_code(std::string("x = - 1"));
	ids_info *s_table = new ids_info();
	s_table->register_var(var, -4, T_UNDEFINED);

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "movl $1, %r8d\n"
									"negl %r8d\n"
									"movl %r8d, -4(%rbp)\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// NEGATION
	////////////////////////

	// x = not y
	translate_ir_code(std::string("x = not true"));
	s_table = new ids_info();
	s_table->register_var(var, -4, T_UNDEFINED);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "movl $1, %r8d\n"
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
	std::string var("x");
	translate_ir_code(std::string("x = 1"));
	ids_info *s_table = new ids_info();
	s_table->register_var(var, -4, T_UNDEFINED);

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "movl $1, -4(%rbp)\n";

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
	translate_ir_code(std::string("goto L"));
	ids_info *s_table = new ids_info();

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "jmp L\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	////////////////////////
	// CONDITIONAL JUMP
	////////////////////////

	//
	// IfTrue
	//
	translate_ir_code(std::string("ifTrue true goto L"));
	//s_table = new ids_info();
	s_table = new ids_info();

	//g = asm_code_generator(ir_code, s_table);
	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	//translation = g.get_translation();
	translation = g.get_translation();

	asm_program_text = "movl $1 , %r8d\n"
						"cmpl $1 , %r8d\n"
						"je L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	//
	// IfFalse
	//
	translate_ir_code(std::string("ifFalse true goto L"));
	s_table = new ids_info();

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "movl $1 , %r8d\n"
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
	translate_ir_code(std::string("if 1 < 2 goto L"));
	s_table = new ids_info();

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "cmpl $1 , $2\n"
						"jl L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	//
	// Less or equal
	//
	translate_ir_code(std::string("if 1 <= 2 goto L"));
	s_table = new ids_info();

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "cmpl $1 , $2\n"
						"jle L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	//
	// Greater
	//
	translate_ir_code(std::string("if 1 > 2 goto L"));
	s_table = new ids_info();

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "cmpl $1 , $2\n"
						"jg L";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	//
	// Greater or equal
	//
	translate_ir_code(std::string("if 1 >= 2 goto L"));
	s_table = new ids_info();

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "cmpl $1 , $2\n"
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
	translate_ir_code(std::string("param 1"
								"param 2"
								"param 3"
								"param 4"
								"param 5"
								"param 6"
								"param 7"
								"param 8"
								"call x,8"));
	ids_info *s_table = new ids_info();

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "movl $1, %rdi\n"
									"movl $2, %rsi\n"
									"movl $3, %rdx\n"
									"movl $4, %rcx\n"
									"movl $5, %r8\n"
									"movl $6, %r9\n"
									"pushq $8\n"
									"pushq $7\n"
									"movl $0, %rax\n"
									"call x";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	// param x
	// y = call p,n
	// 8 integer parameters
	translate_ir_code(std::string("param 1"
								"param 2"
								"param 3"
								"param 4"
								"param 5"
								"param 6"
								"param 7"
								"param 8"
								"y = call x,8"));
	std::string var("y");
	s_table = new ids_info();
	s_table->register_var(var, 4, T_UNDEFINED);

	g = asm_code_generator(ir_code, s_table);
	g.translate_ir();

	translation = g.get_translation();

	asm_program_text = "movl $1, %rdi\n"
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
	translate_ir_code(std::string("enter 1"));
	ids_info *s_table = new ids_info();

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	// 0 levels of nesting.
	std::string asm_program_text = "enter $1, $0\n";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_return(){
	std::cout << "7) Translation of the return instruction: ";

	// return x
	translate_ir_code(std::string("return 1"));
	ids_info *s_table = new ids_info();

	asm_code_generator g(ir_code, s_table);
	g.translate_ir();

	asm_instructions_list *translation = g.get_translation();

	std::string asm_program_text = "movl $1, %rax\n"
									"leave\n"
									"ret";

	translate_asm_code(asm_program_text);

	assert(are_equal_instructions_list(*translation, *asm_code));

	std::cout << "OK. " << std::endl;
}

void test_asm_code_generator(){
	std::cout << "\nTesting assembly code generation:" << std::endl;
	test_binary_assign();
	test_unary_assign();
	test_copy();
	test_jmp();
	test_parameter_and_call();
	test_enter();
	test_return();
}
