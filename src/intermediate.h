#ifndef INTERMEDIATE_H_
#define INTERMEDIATE_H_

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
	FLOAT,
	STRING
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
				std::string *sval;
			} val;
		} constant; // Constant, formed by type and a value.
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
	RELATIONAL_JUMP,	// if x relop y goto L
	PARAMETER,			// param x
	PROCEDURE_CALL,		// call p, n
	FUNCTION_CALL,		// y = call p, n
	RETURN,				// return [y]
	LABEL				// L: skip
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
	quad_oper op;
	address_pointer arg1;
	address_pointer arg2;
	address_pointer result;
};

typedef std::shared_ptr<quad> quad_pointer;

class instructions : public std::vector<quad_pointer> {};

#endif // INTERMEDIATE_H_
