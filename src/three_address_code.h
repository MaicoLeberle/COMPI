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
		} constant; // Constant, formed by type and a value. For now no string
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
	BEGIN_PROCEDURE		// BEGIN_PROCEDURE Nmbr. of bytes for locals and
						// temporals in stack frame
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
quad_pointer new_label(const std::string&);
quad_pointer new_copy(const std::string&, const address_pointer arg);
quad_pointer new_int_field(const std::string&);
quad_pointer new_float_field(const std::string&);
quad_pointer new_boolean_field(const std::string&);
quad_pointer new_instance_field(const std::string&, const std::string&);
quad_pointer new_int_array_field(const std::string&, const int);
quad_pointer new_float_array_field(const std::string&, const int);
quad_pointer new_boolean_array_field(const std::string&, const int);

// Constructors of specific type of addresses
address_pointer new_integer_constant(int value);
address_pointer new_float_constant(float value);
address_pointer new_boolean_constant(bool value);

// Procedures for debugging.
// TODO: quizás debería recibir un address_pointer
bool is_label(const quad_pointer& instruction, const std::string& label);

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
bool is_copy(const quad_pointer& instruction, const std::string& x,
					const std::string& y);

bool is_indexed_copy_to(const quad_pointer& instruction, const std::string& x,
		const std::string& i, const std::string& y);

#endif // THREE_ADDRESS_CODE_H_
