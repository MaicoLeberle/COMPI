#ifndef THREE_ADDRESS_CODE_H_
#define THREE_ADDRESS_CODE_H_

#include <memory>
#include <string>
#include <vector>

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
		std::string *label; // In case the address is a label.
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
quad_pointer new_label(const std::string&);
quad_pointer new_copy(const address_pointer dest, const address_pointer orig);
quad_pointer new_indexed_copy_to(const address_pointer dest,
								  const address_pointer index,
								  const address_pointer orig);
quad_pointer new_indexed_copy_from(const address_pointer dest,
								  const address_pointer orig,
								  const address_pointer index);
quad_pointer new_enter_procedure(const unsigned int);
quad_pointer new_binary_assign(const address_pointer dest,
							   const address_pointer arg1,
							   const address_pointer arg2,
							   quad_oper op);
quad_pointer new_unary_assign(const address_pointer dest,
							   const address_pointer arg,
							   quad_oper op);
quad_pointer new_parameter_inst(const address_pointer param);
quad_pointer new_function_call_inst(const address_pointer dest,
									const address_pointer func_label,
									const address_pointer param_quantity);
quad_pointer new_procedure_call_inst(const address_pointer proc_label,
									const address_pointer param_quantity);
quad_pointer new_conditional_jump_inst(const address_pointer guard,
									std::string label,
									quad_oper op);
quad_pointer new_unconditional_jump_inst(std::string label);
quad_pointer new_return_inst(address_pointer);

// if x relop y goto label
quad_pointer new_relational_jump_inst(const address_pointer x, const address_pointer y,
								quad_oper relop, std::string label);
// Constructors of specific type of addresses
// TODO: cambiarles el nombre, para que quede claro que construyen addresses
address_pointer new_integer_constant(int value);
address_pointer new_float_constant(float value);
address_pointer new_boolean_constant(bool value);
address_pointer new_name_address(std::string name);
address_pointer new_label_address(std::string label);

// Procedures for debugging.
// TODO: quizás debería recibir un address_pointer
bool is_label(const quad_pointer& instruction, const std::string& label);

// TODO: no hace falta todas estas is_plus_oper, etc: basta con is_binary_assignment
// x = y + z
bool is_plus_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y * z
bool is_times_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y / z
bool is_divide_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y % z
bool is_mod_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y - z
bool is_minus_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y < z
bool is_less_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y <= z
bool is_less_equal_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y > z
bool is_greater_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y >= z
bool is_greater_equal_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);
// x = y == z
bool is_equal_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y != z
bool is_distinct_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y and z
bool is_and_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = y or z
bool is_or_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y, const std::string& z);

// x = - y
bool is_negative_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y);

// x = not y
bool is_negation_oper(const quad_pointer& instruction, const std::string& x,
					const std::string& y);

// x = y
bool is_copy(const quad_pointer& instruction, const address_pointer& dest,
		const address_pointer& orig);

// x[i] = y
bool is_indexed_copy_to(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& index,
						const address_pointer& orig);

// x = y op z
bool is_binary_assignment(const quad_pointer& instruction,
						 const address_pointer& dest,
						 const address_pointer& arg1,
						 const address_pointer& arg2,
						 quad_oper op);

bool is_unary_assignment(const quad_pointer& instruction,
						 const address_pointer& dest,
						 const address_pointer& arg,
						 quad_oper op);

// x[i] = y
bool is_indexed_copy_to(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& index,
						const address_pointer& orig);

// x = y[i]
bool is_indexed_copy_from(const quad_pointer& instruction,
						const address_pointer& dest,
						const address_pointer& orig,
						const address_pointer& index);

bool is_enter_procedure(const quad_pointer& instruction, unsigned int bytes);

bool is_procedure_call(const quad_pointer& instruction,
						const address_pointer proc_label,
						int param_quantity);

bool is_function_call(const quad_pointer& instruction,
						const address_pointer dest,
						const address_pointer func_label,
						const address_pointer param_quantity);

bool is_parameter_inst(const quad_pointer& instruction,
						const address_pointer param);

bool is_conditional_jump_inst(const quad_pointer& instruction,
								const address_pointer guard,
								std::string label,
								quad_oper op);

bool is_unconditional_jump_inst(const quad_pointer& instruction,
								std::string label);

bool is_return_inst(const quad_pointer& instruction,
					const address_pointer& ret_value);

bool is_relational_jump_inst(const quad_pointer& instruction,
		const address_pointer x, const address_pointer y,
		quad_oper relop, std::string label);

bool are_equal_address_pointers(const address_pointer&, const address_pointer&);

bool are_equal_quad_pointers(const quad_pointer&, const quad_pointer&);

#endif // THREE_ADDRESS_CODE_H_
