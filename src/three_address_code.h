#ifndef THREE_ADDRESS_CODE_H_
#define THREE_ADDRESS_CODE_H_

#include <memory>
#include <string>
#include <vector>
#include <cassert>

// Macro to convert booleans into strings (because std::to_string does it
// wrong...). Taken from
// http://stackoverflow.com/questions/29383/converting-bool-to-text-in-c
#define BOOL_STR(b) ((b)?"true":"false")

enum class address_type {
	ADDRESS_NAME,
	ADDRESS_CONSTANT,
	ADDRESS_TEMP,
	ADDRESS_LABEL
};

enum class value_type {
	BOOLEAN,
	INTEGER,
	FLOAT
};

struct address {
	address_type type;
	union {
		std::string *name; // Address name.
		struct {
			value_type type;
			union {
				int ival;
				float fval;
				bool bval;
				// TODO: esto está bien así?
				unsigned int dimension; // dimension > 0 => address points to an
										// array.
			} val;
		} constant; // Constant, formed by type and a value. For the moment no string
					// as type.
		unsigned int temp; // Compiler-generated temporary.
		struct{
			std::string *val;
			std::string *class_name; // In case the label is a method's name.
			std::string *method_name;
		} label; // In case the address is a label.
	} value;
};

typedef std::shared_ptr<address> address_pointer;

enum class quad_type {
	BINARY_ASSIGN, 		// x = y op z
	UNARY_ASSIGN,  		// x = op y
	COPY,				// x = y
	INDEXED_COPY_TO,	// x[i] = y
	INDEXED_COPY_FROM,	// x = y[i]
	UNCONDITIONAL_JUMP,	// goto L
	CONDITIONAL_JUMP,	// if x goto L || ifFalse x goto L
						// TODO: cual es la diferencia entre esto y primero asignar
						// en un temporal el resultado de la comparación, y luego
						// hacer if x goto L?
	RELATIONAL_JUMP,	// if x relop y goto L
	PARAMETER,			// param x
	PROCEDURE_CALL,		// call p, n
	FUNCTION_CALL,		// y = call p, n
	RETURN,				// return [y]
	LABEL,				// L: skip
	// TODO: está bien?
	ENTER_PROCEDURE		// ENTER_PROCEDURE Nmbr. of bytes for locals and
						// temporal variables in stack frame
};

enum class quad_oper {
	ADDRESS_OF,
	IFTRUE,
	IFFALSE,
	NEGATIVE,
	NEGATION,
	TIMES,
	DIVIDE,
	MOD,
	PLUS,
	MINUS,
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	EQUAL,
	DISTINCT,
	AND,
	OR,
	NONE // Use in case the instruction doesn't need an operator
};

struct quad {
	quad_type type;
	quad_oper op = quad_oper::NONE;
	address_pointer arg1 = nullptr;
	address_pointer arg2 = nullptr;
	address_pointer result = nullptr;
};

typedef std::shared_ptr<quad> quad_pointer;

class instructions_list : public std::vector<quad_pointer> {};

// Constructors of specific 3-address instructions.
// TODO: cambiarles el nombre, para quede claro que generan instrucciones
// TODO: utilizar address_pointers solo en situaciones en las que se pueda
// recibir por parámetros addresses de distinto tipo, y no cuando la instrucción
// debe recibir un address de un tipo en particular. Ejemplo: functional call
// recibe algo que necesariamente va a ser un label (y, por lo tanto, bastaría
// con recibir sólo un std::string)


/////////////////////////
// @QUAD
/////////////////////////
quad_oper get_inst_op(const quad_pointer&);

quad_type get_inst_type(const quad_pointer&);

bool are_equal_quad_pointers(const quad_pointer&, const quad_pointer&);

bool are_equal_instructions_list(const instructions_list& x,
								const instructions_list& y);

std::string print_instructions_list(const instructions_list&);


/////////////////////////
// @LABEL INSTRUCTION
/////////////////////////
//quad_pointer new_label_inst(const std::string&);

quad_pointer new_label_inst(const address_pointer& label);

//quad_pointer new_method_label_inst(const std::string& method_name,
//								   const std::string& class_name);

/* PRE : {quad->type == quad_type::LABEL} */
std::string get_label_inst_label(const quad_pointer&);

/* PRE : {quad->type == quad_type::LABEL} */
std::string get_label_inst_method_name(const quad_pointer&);

/* PRE : {quad->type == quad_type::LABEL} */
std::string get_label_inst_class_name(const quad_pointer&);

bool is_label(const quad_pointer& instruction,
				const std::string& label);

/////////////////////////
// @COPY
/////////////////////////
quad_pointer new_copy(const address_pointer& dest,
						const address_pointer& orig);

/* PRE : {quad->type == quad_type::COPY} */
address_pointer get_copy_inst_dest(const quad_pointer&);

/* PRE : {quad->type == quad_type::COPY} */
address_pointer get_copy_inst_orig(const quad_pointer&);

bool is_copy(const quad_pointer& instruction,
			const address_pointer& dest,
			const address_pointer& orig);


/////////////////////////
// @INDEXED COPY TO
/////////////////////////
quad_pointer new_indexed_copy_to(const address_pointer& dest,
								  const address_pointer& index,
								  const address_pointer& orig);

/* PRE : {quad->type == quad_type::INDEXED_COPY_TO} */
address_pointer get_indexed_copy_to_dest(const quad_pointer&);

/* PRE : {quad->type == quad_type::INDEXED_COPY_TO} */
address_pointer get_indexed_copy_to_index(const quad_pointer&);

/* PRE : {quad->type == quad_type::INDEXED_COPY_TO} */
address_pointer get_indexed_copy_to_src(const quad_pointer&);

bool is_indexed_copy_to(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& index,
						const address_pointer& orig);


/////////////////////////
// @INDEXED COPY FROM
/////////////////////////
quad_pointer new_indexed_copy_from(const address_pointer& dest,
								  const address_pointer& orig,
								  const address_pointer& index);

/* PRE : {quad->type == quad_type::INDEXED_COPY_FROM} */
address_pointer get_indexed_copy_from_dest(const quad_pointer&);

/* PRE : {quad->type == quad_type::INDEXED_COPY_FROM} */
address_pointer get_indexed_copy_from_index(const quad_pointer&);

/* PRE : {quad->type == quad_type::INDEXED_COPY_FROM} */
address_pointer get_indexed_copy_from_src(const quad_pointer&);

bool is_indexed_copy_from(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& orig,
						const address_pointer& index);


/////////////////////////
// @ENTER PROCEDURE
/////////////////////////
quad_pointer new_enter_procedure(unsigned int);

/* PRE : {quad->type == quad_type::ENTER} */
int get_enter_inst_bytes(const quad_pointer&);

bool is_enter_procedure(const quad_pointer& instruction, unsigned int bytes);


/////////////////////////
// @BINARY ASSIGN
/////////////////////////
quad_pointer new_binary_assign(const address_pointer& dest,
							   const address_pointer& arg1,
							   const address_pointer& arg2,
							   quad_oper op);

bool is_binary_assignment(const quad_pointer& instruction,
						 const address_pointer& dest,
						 const address_pointer& arg1,
						 const address_pointer& arg2,
						 quad_oper op);


/////////////////////////
// @UNARY ASSIGN
/////////////////////////
quad_pointer new_unary_assign(const address_pointer& dest,
							   const address_pointer& arg,
							   quad_oper op);

/* PRE : {quad->type == quad_type::UNARY_ASSIGN} */
address_pointer get_unary_assign_dest(const quad_pointer&);

/* PRE : {quad->type == quad_type::UNARY_ASSIGN} */
address_pointer get_unary_assign_src(const quad_pointer&);

bool is_unary_assignment(const quad_pointer& instruction,
						 const address_pointer& dest,
						 const address_pointer& arg,
						 quad_oper op);


/////////////////////////
// @PARAM
/////////////////////////
quad_pointer new_parameter_inst(const address_pointer& param);

bool is_parameter_inst(const quad_pointer& instruction,
						const address_pointer& param);

address_pointer get_param_inst_param(const quad_pointer&);


/////////////////////////
// @FUNCTION CALL
/////////////////////////
quad_pointer new_function_call_inst(const address_pointer& dest,
									const address_pointer& func_label,
									const address_pointer& param_quantity);

/* PRE : {quad->type == quad_type::FUNCTION_CALL} */
address_pointer get_function_call_dest(const quad_pointer&);

bool is_function_call(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& func_label,
						const address_pointer& param_quantity);


/////////////////////////
// @PROCEDURE CALL
/////////////////////////
quad_pointer new_procedure_call_inst(const address_pointer& proc_label,
									const address_pointer& param_quantity);

/* PRE : {quad->type == quad_type::PROCEDURE_CALL or
 * 			quad->type == quad_type::FUNCTION_CALL} */
std::string get_procedure_or_function_call_label(const quad_pointer&);

bool is_procedure_call(const quad_pointer& instruction,
						const address_pointer& proc_label,
						int param_quantity);


/////////////////////////
// @CONDITIONAL JUMP
/////////////////////////
quad_pointer new_conditional_jump_inst(const address_pointer& guard,
									const address_pointer& label,
									quad_oper op);

/* PRE : {quad->type == quad_type::CONDITIONAL_JUMP} */
std::string get_conditional_jmp_label(const quad_pointer&);

/* PRE : {quad->type == quad_type::CONDITIONAL_JUMP} */
address_pointer get_conditional_jmp_guard(const quad_pointer&);

bool is_conditional_jump_inst(const quad_pointer& instruction,
								const address_pointer& guard,
								const address_pointer& label,
								quad_oper op);


/////////////////////////
// @UNCONDITIONAL JUMP
/////////////////////////
quad_pointer new_unconditional_jump_inst(const address_pointer& label);

/* PRE : {quad->type == quad_type::UNCONDITIONAL_JUMP} */
std::string get_unconditional_jmp_label(const quad_pointer&);

bool is_unconditional_jump_inst(const quad_pointer& instruction,
								const address_pointer& label);


/////////////////////////
// RELATIONAL JUMP
/////////////////////////
quad_pointer new_relational_jump_inst(const address_pointer& x,
									const address_pointer& y,
									quad_oper relop,
									const address_pointer& label);

/* PRE : {quad->type == quad_type::RELATIONAL_JUMP} */
std::string get_relational_jmp_label(const quad_pointer&);

bool is_relational_jump_inst(const quad_pointer& instruction,
							const address_pointer& x,
							const address_pointer& y,
							quad_oper relop,
							const address_pointer& label);


/////////////////////////
// RETURN
/////////////////////////
quad_pointer new_return_inst(const address_pointer&);

bool is_return_inst(const quad_pointer& instruction,
					const address_pointer& ret_value);


/////////////////////////
// @ADDRESS
/////////////////////////
/* PRE : {address->type == address_type::ADDRESS_CONSTANT} */
value_type get_constant_address_type(const address_pointer&);

address_type get_address_type(const address_pointer&);

bool are_equal_address_pointers(const address_pointer&, const address_pointer&);


/////////////////////////
// @INTEGER ADDRESS
/////////////////////////
// TODO: cambiarles el nombre, para que quede claro que construyen addresses
address_pointer new_integer_constant(int value);

/* PRE : {address->type == address_type::ADDRESS_CONSTANT and
 * 			get_constant_address_type(address) == INTEGER} */
int get_constant_address_integer_value(const address_pointer&);



/////////////////////////
// FLOAT ADDRESS
/////////////////////////
address_pointer new_float_constant(float value);

/* PRE : {address->type == address_type::ADDRESS_CONSTANT and
 * 			get_constant_address_type(address) == FLOAT} */
float get_constant_address_float_value(const address_pointer&);


/////////////////////////
// BOOLEAN ADDRESS
/////////////////////////
address_pointer new_boolean_constant(bool value);

/* PRE : {address->type == address_type::ADDRESS_CONSTANT and
 * 			get_constant_address_type(address) == BOOLEAN} */
bool get_constant_address_boolean_value(const address_pointer&);


/////////////////////////
// @NAME ADDRESS
/////////////////////////
address_pointer new_name_address(const std::string& name);

/* PRE : {address->type == address_type::ADDRESS_NAME} */
std::string get_address_name(const address_pointer&);


/////////////////////////
// @LABEL ADDRESS
/////////////////////////
address_pointer new_label_address(const std::string& label);

address_pointer new_method_label_address(const std::string& method_name,
											const std::string& class_name);

/* PRE : {address->type == address_type::ADDRESS_LABEL} */
std::string get_label_address_value(const address_pointer&);

/* PRE : {address->type == address_type::ADDRESS_LABEL} */
std::string get_label_address_class_name(const address_pointer&);

/* PRE : {address->type == address_type::ADDRESS_LABEL} */
std::string get_label_address_method_name(const address_pointer&);


#endif // THREE_ADDRESS_CODE_H_
