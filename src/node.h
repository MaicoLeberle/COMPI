#ifndef NODE_H_
#define NODE_H_

#include <iostream>
#include <vector>
#include <cassert>
#include <cstdlib>

// TODO: Use smart pointers

// Helper enumeration classes /////////////////////////////////////////////////
enum class Type { INTEGER, BOOLEAN, FLOAT, VOID, ID };
enum class Oper { PLUS, MINUS, TIMES, DIVIDE, MOD, LESS, GREATER, LESS_EQUAL,
				  GREATER_EQUAL, EQUAL, DISTINCT, AND, OR };
enum class AssignOper { ASSIGN, PLUS_ASSIGN, MINUS_ASSING };

// Abstract classes ///////////////////////////////////////////////////////////
/**
 * Main tree class. All classes are derived from node.
 */
class node {
public:
    virtual ~node() {}
};
class node_class_block : public node {};
class node_statement : public node {};
class node_expr : public node {};
class node_literal : public node_expr {};

// Classes prototypes /////////////////////////////////////////////////////////
class node_class_decl;
class node_body;
class node_id;
class node_parameter_identifier;
class node_block;
class node_location;
class node_expr;
class node_method_call;

// Wrappers ///////////////////////////////////////////////////////////////////
class classes_list : public std::vector<node_class_decl*> {};
class class_block_list : public std::vector<node_class_block*> {};
class id_list : public std::vector<node_id*> {};
class parameter_list : public std::vector<node_parameter_identifier*> {};
class statement_list : public std::vector<node_statement*> {};
class reference_list : public std::vector<std::string> {};
class expression_list : public std::vector<node_expr*> {};

// Classes ////////////////////////////////////////////////////////////////////
/**
 * Main program node. This is always the root of the AST.
 */
class node_program : public node {
public:
	/**
	 * :field classes: List of all the classes in the program (children of the tree)
	 */
    classes_list classes;

    node_program(node_class_decl *class_) {
        classes.push_back(class_);
    }
};

/**
 * Block of the class. Contains the sequence of fields and methods.
 */
class node_class_decl : public node {
public:
	/**
	 * :field id: Name of the class
	 * :field class_block: List of fields and methods of the class
	 */
    std::string id;
    class_block_list class_block;

    node_class_decl(std::string id_, class_block_list class_block_) :
    	id(id_), class_block(class_block_) {}
};

/**
 * Node for a single identifier
 */
class node_id : public node {
public:
	/**
	 * :field id: Identifier name of the field
	 * :field array_size: If >= 0 the identifier is an array of array_size elements
	 */
	std::string id;
	int array_size;
	// TODO: Check if we need to include a reference to the symbol's table

	node_id(std::string id_, int array_size_ = -1) : id(id_), array_size(array_size_) {}
};

/**
 * Node holding a list of field declarations of the same type
 * It can be treated both as part of a class block or as a statement
 */
class node_field_decl : public node_class_block, public node_statement {
public:
	/**
	 * :field type: Type of the field.
	 * 				REQUIRES: type != Type::VOID
	 * :field id_idx: Index for the ID in the symbol table (only if type == Type::ID)
	 * :field ids: List of identifiers for the field declaration.
	 */
	Type type;
    int id_idx;
    id_list ids;

    node_field_decl(Type type_, id_list ids_, int id_idx_ = 0) :
    	type(type_), id_idx(id_idx_), ids(ids_) {
    	assert(type_ != Type::VOID);
    }
};

/**
 * Node holding a method declaration
 */
class node_method_decl : public node_class_block {
public:
	/**
	 * :field type: Returning type of the method. Can be of type Type::VOID
	 * :field id_idx: Index for the ID in the symbol table (only if type == Type::ID)
	 * :field id: Identifier name of the method.
	 * :field parameters: List of the parameters in the method.
	 * :field body: Body of the method.
	 */
    Type type;
    int id_idx;
    std::string id;
    parameter_list parameters;
    node_body *body;

    node_method_decl(Type type_, std::string id_, parameter_list parameters_,
		node_body *body_, int id_idx_ = 0) :
    	type(type_), id_idx(id_idx_), id(id_), parameters(parameters_), body(body_) {}
};

/**
 * Node for a single parameter identifier
 */
class node_parameter_identifier : public node {
public:
	/**
	 * :field type: Type of the parameter
	 * 				REQUIRES: type != Type::VOID
	 * :field id_idx: Index for the ID in the symbol table (only if type == Type::ID)
	 * :field id: Identifier name of the method.
	 */
    Type type;
    int id_idx;
    std::string id;

    node_parameter_identifier(Type type_, std::string id_, int id_idx_ = 0) :
    	type(type_), id_idx(id_idx_), id(id_) {
    	assert(type_ != Type::VOID);
    }
};

/**
 * Node for the body of a method
 */
class node_body : public node {
public:
	/**
	 * :field is_extern: Whether the block of the body is contained elsewhere
	 * :field block: The node block of the body with the statements
	 */
    bool is_extern;
    node_block *block;

    node_body(node_block *block_) : is_extern(false), block(block_) {}
    node_body() : is_extern(true), block(NULL) {}
};

/**
 * Node for a block of statements that defines a new local scope
 */
class node_block : public node_statement {
public:
	/**
	 * :field content: List of statements in the block.
	 */
    statement_list content;

    node_block(statement_list content_) : content(content_) {}
};

/**
 * Node for assignation of a variable to a value
 */
class node_assignment_statement : public node_statement {
public:
	/**
	 * :field location: Location to assign the value
	 * :field oper: Operator of assignation
	 * :field expression: Expression of assignation
	 */
    node_location* location;
    AssignOper oper;
    node_expr* expression;

    node_assignment_statement(node_location* location_, AssignOper oper_,
		node_expr* expression_) :
    	location(location_), oper(oper_), expression(expression_) {}
};

/**
 * Node for a method call. Can be treated both as an expression or as a statement.
 */
class node_method_call : public node_expr, public node_statement {
public:
	/**
	 * :field ids: List of ids references in the method call. Can be of size 1.
	 * :field parameters: List of expressions called in the method parameters.
	 */
    reference_list ids;
    expression_list parameters;

    node_method_call(reference_list ids_) : ids(ids_) {}
    node_method_call(reference_list ids_, expression_list parameters_) :
    	ids(ids_), parameters(parameters_) {}
};

/**
 * Node of an if conditional
 */
class node_if_statement : public node_statement {
public:
	/**
	 * :field expression: Expression to evaluate.
	 * :field then_statement: Statement to execute in case expression == true.
	 * :field else_statement: Statement to execute in case expression == false.
	 */
    node_expr* expression;
    node_statement* then_statement;
    node_statement* else_statement;

    node_if_statement(node_expr* expression_, node_statement* then_statement_) :
    	expression(expression_), then_statement(then_statement_), else_statement(NULL) {}
    node_if_statement(node_expr* expression_, node_statement* then_statement_, node_statement* else_statement_) :
    	expression(expression_), then_statement(then_statement_), else_statement(else_statement_) {}
};

/**
 * Node of a for loop
 */
class node_for_statement : public node_statement {
public:
	/**
	 * :field id: Name of the variable to iterate inside the for
	 * 			  (TODO: Maybe should be the identifier index in the symbol table)
	 * :field from: Expression to start the loop
	 * :field to: Expression to end the loop
	 * :field body: Statement(s) to execute inside the loop
	 */
    std::string id;
    node_expr* from;
    node_expr* to;
    node_statement* body;

    node_for_statement(std::string id_, node_expr* from_, node_expr* to_, node_statement* body_) :
    	id(id_), from(from_), to(to_), body(body_) {}
};

/**
 * Node of a while loop
 */
class node_while_statement : public node_statement {
public:
	/**
	 * :field expression: Expression to evaluate for loop termination
	 * :field body: Statement(s) to execute inside the loop
	 */
    node_expr* expression;
    node_statement* body;

    node_while_statement(node_expr* expression_, node_statement* body_)
    : expression(expression_), body(body_) {}
};

/**
 * Node of a return
 */
class node_return_statement : public node_statement {
public:
	/**
	 * :field expression: Expression to return. Can be NULL.
	 */
    node_expr* expression;

    node_return_statement() : expression(NULL) {}
    node_return_statement(node_expr* expression_) : expression(expression_) {}
};

/**
 * Node of a break
 */
class node_break_statement : public node_statement {};

/**
 * Node of a continue
 */
class node_continue_statement : public node_statement {};

/**
 * Node of a skip
 */
class node_skip_statement : public node_statement {};

/**
 * Node of an integer
 */
class node_int_literal : public node_literal {
public:
	/**
	 * :field value: Value of the integer literal
	 */
	long long value;

    node_int_literal(long long value_) : value(value_) {}
};

/**
 * Node of a float
 */
class node_float_literal : public node_literal {
public:
	/**
	 * :field value: Value of the float literal
	 */
    double value;

    node_float_literal(double value_) : value(value_) {}
};

/**
 * Node of a boolean
 */
class node_bool_literal : public node_literal {
public:
	/**
	 * :field value: Value of the bool literal
	 */
	bool value;

    node_bool_literal(bool value_) : value(value_) {}
};

/**
 * Node of a string
 */
class node_string_literal : public node_literal {
public:
	/**
	 * :field value: Value of the string literal
	 */
    std::string value;

    node_string_literal(std::string value_) : value(value_) { }
};

/**
 * Node of a binary operation
 */
class node_binary_operation_expr : public node_expr {
public:
	/**
	 * :field oper: Operator of the operation
	 * :field left: Left expression of the operation
	 * :field right: Right expression of the operation
	 */
    Oper oper;
    node_expr* left;
    node_expr* right;

    node_binary_operation_expr(Oper oper_, node_expr* left_, node_expr* right_) :
    	oper(oper_), left(left_), right(right_) {}
};

/**
 * Node for a location
 */
class node_location : public node_expr {
public:
	/**
	 * :field ids: List of ids references in the location. Can be of size 1.
	 * :field array_idx_expr: Expression to evaluate to get the index if the location is an array. Can be NULL.
	 */
	reference_list ids;
    node_expr* array_idx_expr;

    node_location(reference_list ids_) : ids(ids_), array_idx_expr(NULL) {}
    node_location(reference_list ids_, node_expr* array_idx_expr_) :
    	ids(ids_), array_idx_expr(array_idx_expr_) {}
};

/**
 * Node for a boolean negation of an expression
 */
class node_negate_expr : public node_expr {
public:
	/**
	 * :field expression: Expression to apply logical negation
	 */
    node_expr *expression;

    node_negate_expr(node_expr *expression_) : expression(expression_) {}
};

/**
 * Node for a negative numeric expression
 */
class node_negative_expr : public node_expr {
public:
	/**
	 * :field expression: Expression to apply the numerical negative
	 */
    node_expr *expression;

    node_negative_expr(node_expr *expression_) : expression(expression_) {}
};

/**
 * Node for an expression between parentheses
 */
class node_parentheses_expr : public node_expr {
public:
	/**
	 * :field expression: Expression between the parentheses.
	 */
    node_expr *expression;

    node_parentheses_expr(node_expr *expression_) : expression(expression_) { }
};

#endif
