#ifndef NODE_H_
#define NODE_H_

#include <memory>
#include <vector>
#include <cassert>
#include <cstdlib>

// Helper enumeration classes /////////////////////////////////////////////////
/**
 * Structure to save the type of a method or location. In case the type is "ID" the
 * tuple also stores the id
 */
struct Type {
    enum _Type {TINTEGER, TBOOLEAN, TFLOAT, TVOID, TID, TSTRING};
    _Type type;
    std::string id;

    // Empty constructor, that lets us declare Type variables, with unknown
    // type information.
    Type() {}

    Type(_Type type_) : type(type_) {}
    // TODO: no haría falta el parámetro type_ en este caso?.
    Type(_Type type_, std::string id_) : type(type_), id(id_) {}
};

/**
 * Enumerator class for binary operators. Operators are sorted according to
 * precedence, from most to least. Operators in the same line have the same
 * precedence.
 */
enum class Oper {
	OTIMES, ODIVIDE, OMOD,
	OPLUS, OMINUS,
	OLESS, OLESS_EQUAL, OGREATER, OGREATER_EQUAL,
	OEQUAL, ODISTINCT,
	OAND,
	OOR
};

/**
 * Enumerator class for assignment operators.
 */
enum class AssignOper {
	AASSIGN,
	APLUS_ASSIGN,
	AMINUS_ASSIGN
};

// Abstract classes ///////////////////////////////////////////////////////////
/**
 * Main tree class. All classes are derived from node.
 */
class node {
public:
	virtual ~node() {}

	template<typename visitor>
	void accept(visitor& v) {}
};

class node_class_block : public node {
public:
	virtual bool is_node_field_decl() = 0;
};

class node_statement : public node {

public:
	// To resolve type erasure
	enum statement {
		field_decl,
		block,
		assignment_statement,
		if_statement,
		for_statement,
		while_statement,
		return_statement,
		break_statement,
		continue_statement,
		skip_statement,
		method_call_statement,
	};

	virtual statement type_of_statement() = 0;
};

class node_expr : public node {
private:
	// Type of the expression, to be inferred by the semantic analysis.
	Type type_expr;

public:
	// To resolve type erasure
	enum expression {
		// Note that this is not directly related with our language's types.
		// This enumeration codes "syntactic" kind of expressions.
		int_literal,
		float_literal,
		bool_literal,
		string_literal,
		binary_operation_expr,
		location,
		negate_expr,
		negative_expr,
		parentheses_expr,
		method_call_expr,
	};

	virtual expression type_of_expression(void) = 0;

	void set_type(Type::_Type _type){
		type_expr.type = _type;
	}

	void set_type(std::string id){
		type_expr.type = Type::_Type::TID;
		type_expr.id = id;
	}

	Type get_type(void){
		return this->type_expr;
	}
};

class node_literal : public node_expr {};

// Classes prototypes /////////////////////////////////////////////////////////
class node_program;
class node_class_decl;
class node_id;
class node_field_decl;
class node_method_decl;
class node_parameter_identifier;
class node_body;
class node_block;
class node_assignment_statement;
class node_method_call_statement;
class node_if_statement;
class node_for_statement;
class node_while_statement;
class node_return_statement;
class node_break_statement;
class node_continue_statement;
class node_skip_statement;
class node_int_literal;
class node_float_literal;
class node_bool_literal;
class node_string_literal;
class node_binary_operation_expr;
class node_location;
class node_negate_expr;
class node_negative_expr;
class node_parentheses_expr;
class node_method_call_expr;

// Smart pointers wrappers ////////////////////////////////////////////////////
typedef std::shared_ptr<node_class_block>             class_block_pointer;
typedef std::shared_ptr<node_statement>               statement_pointer;
typedef std::shared_ptr<node_expr>                    expr_pointer;
typedef std::shared_ptr<node_program>                 program_pointer;
typedef std::shared_ptr<node_class_decl>              class_pointer;
typedef std::shared_ptr<node_id>                      id_pointer;
typedef std::shared_ptr<node_field_decl>              field_pointer;
typedef std::shared_ptr<node_method_decl>             method_pointer;
typedef std::shared_ptr<node_parameter_identifier>    parameter_pointer;
typedef std::shared_ptr<node_body>                    body_pointer;
typedef std::shared_ptr<node_block>                   block_pointer;
typedef std::shared_ptr<node_assignment_statement>    assignment_pointer;
typedef std::shared_ptr<node_method_call_statement>   method_call_statement_pointer;
typedef std::shared_ptr<node_if_statement>            if_pointer;
typedef std::shared_ptr<node_for_statement>           for_pointer;
typedef std::shared_ptr<node_while_statement>         while_pointer;
typedef std::shared_ptr<node_return_statement>        return_pointer;
typedef std::shared_ptr<node_break_statement>         break_pointer;
typedef std::shared_ptr<node_continue_statement>      continue_pointer;
typedef std::shared_ptr<node_skip_statement>          skip_pointer;
typedef std::shared_ptr<node_int_literal>             int_pointer;
typedef std::shared_ptr<node_float_literal>           float_pointer;
typedef std::shared_ptr<node_bool_literal>            bool_pointer;
typedef std::shared_ptr<node_string_literal>          string_pointer;
typedef std::shared_ptr<node_binary_operation_expr>   binary_operation_pointer;
typedef std::shared_ptr<node_location>                location_pointer;
typedef std::shared_ptr<node_negate_expr>             negate_expr_pointer;
typedef std::shared_ptr<node_negative_expr>           negative_expr_pointer;
typedef std::shared_ptr<node_parentheses_expr>        parentheses_expr_pointer;
typedef std::shared_ptr<node_method_call_expr>        method_call_expr_pointer;

// List wrappers //////////////////////////////////////////////////////////////
class classes_list      : public std::vector<class_pointer> {};
class class_block_list  : public std::vector<class_block_pointer> {};
class id_list           : public std::vector<id_pointer> {};
class parameter_list    : public std::vector<parameter_pointer> {};
class statement_list    : public std::vector<statement_pointer> {};
class reference_list    : public std::vector<std::string> {
public:
	/* Construct a string representation of a variable, represented by a reference_list.
	 * It uses the dotted representation, if the variable references to an
	 * instance's field. */
	std::string printable_field_id() const{
		reference_list::const_iterator it = this->begin();
		std::string name(*it), dot(".");
		it++;
		for(; it != this->end(); it++){
			name += dot + *it;
		}

		return name;
	}
};
class expression_list   : public std::vector<expr_pointer> {};

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

    node_program(class_pointer class_) {
        classes.push_back(class_);
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
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

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
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

    node_id(std::string id_, int array_size_ = -1) :
    	id(id_), array_size(array_size_) {}

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node holding a list of field declarations of the same type
 * It can be treated both as part of a class block or as a statement
 */
class node_field_decl : public node_class_block, public node_statement {
public:
    /**
     * :field type: Type of the field.
     *                 REQUIRES: type != Type::VOID
     * :field ids: List of identifiers for the field declaration.
     */
    Type type;
    id_list ids;

    node_field_decl(Type type_, id_list ids_) : type(type_), ids(ids_) {
        assert(type_.type != Type::TVOID);
    }

    bool is_node_field_decl(){
    	return true;
    }

    statement type_of_statement(void) {
		return field_decl;
	}

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node holding a method declaration
 */
class node_method_decl : public node_class_block {
public:
    /**
     * :field type: Returning type of the method. Can be of type Type::VOID
     * :field id: Identifier name of the method.
     * :field parameters: List of the parameters in the method.
     * :field body: Body of the method.
     */
    Type type;
    std::string id;
    parameter_list parameters;
    body_pointer body;

    node_method_decl(Type type_, std::string id_, parameter_list parameters_,
        body_pointer body_) : type(type_), id(id_), parameters(parameters_), body(body_) {}

    bool is_node_field_decl(){
    	return false;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node for a single parameter identifier
 */
class node_parameter_identifier : public node {
public:
    /**
     * :field type: Type of the parameter
     *                 REQUIRES: type != Type::VOID
     * :field id: Identifier name of the method.
     */
    Type type;
    std::string id;

    node_parameter_identifier(Type type_, std::string id_) :
        type(type_), id(id_) {
        assert(type_.type != Type::TVOID);
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
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
    block_pointer block;

    node_body(block_pointer block_) : is_extern(false), block(block_) {}
    node_body() : is_extern(true), block(nullptr) {}

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
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

    statement type_of_statement(void) {
    	return block;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
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
    location_pointer location;
    AssignOper oper;
    expr_pointer expression;

    node_assignment_statement(location_pointer location_,
        AssignOper oper_, expr_pointer expression_) :
        location(location_), oper(oper_), expression(expression_) {}

    statement type_of_statement(void) {
    	return assignment_statement;
    }

    location_pointer get_location(void){
    	return this->location;
    }

    expr_pointer get_expression(void){
		return this->expression;
	}

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node for a method call. Can be treated both as an expression or as a statement.
 */
class method_call {
public:
    /**
     * :field ids: List of ids references in the method call. Can be of size 1.
     * :field parameters: List of expressions called in the method parameters.
     */
    reference_list ids;  // TODO: Change references from string to node?
    expression_list parameters;

    method_call(reference_list ids_) : ids(ids_) {}
    method_call(reference_list ids_, expression_list parameters_) :
        ids(ids_), parameters(parameters_) {}

};

class node_method_call_expr : public node_expr{
public:
	method_call *method_call_data; // Object that stores information about
									// the call.

	node_method_call_expr(method_call *_method_call_data) :
		method_call_data(_method_call_data) {}

	expression type_of_expression(void) {
		return method_call_expr;
	}

	template<typename visitor>
	void accept(visitor& v) {
		v.visit(*this);
	}

};

class node_method_call_statement : public node_statement{
public:
	method_call *method_call_data; // Object that stores information about
									// the call.

	node_method_call_statement(method_call *_method_call_data) :
		method_call_data(_method_call_data) {}

	statement type_of_statement(void) {
		return method_call_statement;
	}

	template<typename visitor>
	void accept(visitor& v) {
		v.visit(*this);
	}

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
    expr_pointer expression;
    statement_pointer then_statement;
    statement_pointer else_statement;

    node_if_statement(expr_pointer expression_, statement_pointer then_statement_) :
        expression(expression_), then_statement(then_statement_), else_statement(nullptr) {}
    node_if_statement(expr_pointer expression_, statement_pointer then_statement_,
        statement_pointer else_statement_) :
        expression(expression_), then_statement(then_statement_), else_statement(else_statement_) {}

    statement type_of_statement(void) {
    	return if_statement;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node of a for loop
 */
class node_for_statement : public node_statement {
public:
    /**
     * :field id: Name of the variable to iterate inside the for
     * :field from: Expression to start the loop
     * :field to: Expression to end the loop
     * :field body: Statement(s) to execute inside the loop
     */
    std::string id;  // TODO: How to check which id is? It is declared for the loop?
    expr_pointer from;
    expr_pointer to;
    statement_pointer body;

    node_for_statement(std::string id_, expr_pointer from_, expr_pointer to_,
        statement_pointer body_) :
        id(id_), from(from_), to(to_), body(body_) {}

    statement type_of_statement(void) {
    	return for_statement;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
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
    expr_pointer expression;
    statement_pointer body;

    node_while_statement(expr_pointer expression_, statement_pointer body_)
    : expression(expression_), body(body_) {}

    statement type_of_statement(void) {
    	return while_statement;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node of a return
 */
class node_return_statement : public node_statement {
public:
    /**
     * :field expression: Expression to return. Can be null.
     */
    expr_pointer expression;

    node_return_statement() : expression(nullptr) {}
    node_return_statement(expr_pointer expression_) : expression(expression_) {}

    statement type_of_statement(void) {
    	return return_statement;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node of a break
 */
class node_break_statement : public node_statement {
public:
    statement type_of_statement(void) {
    	return break_statement;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node of a continue
 */
class node_continue_statement : public node_statement {
public:

    statement type_of_statement(void) {
		return continue_statement;
	}

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node of a skip
 */
class node_skip_statement : public node_statement {
public:

    statement type_of_statement(void) {
		return skip_statement;
	}

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

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

    expression type_of_expression(void) {
    	return int_literal;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
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

    expression type_of_expression(void) {
    	return float_literal;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
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

    expression type_of_expression(void) {
    	return bool_literal;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
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

    expression type_of_expression(void) {
    	return string_literal;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
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
    expr_pointer left;
    expr_pointer right;

    node_binary_operation_expr(Oper oper_, expr_pointer left_, expr_pointer right_) :
        oper(oper_), left(left_), right(right_) {}

    expression type_of_expression(void) {
    	return binary_operation_expr;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node for a location
 */
class node_location : public node_expr {
public:
    /**
     * :field ids: List of ids references in the location. Can be of size 1.
     * :field array_idx_expr: Expression to evaluate to get the index if the location is an array. Can be null.
     */
    reference_list ids;
    expr_pointer array_idx_expr;
    bool is_attribute_from_actual_object;

    node_location(reference_list ids_) : ids(ids_), array_idx_expr(nullptr) {}
    node_location(reference_list ids_, expr_pointer array_idx_expr_) :
        ids(ids_), array_idx_expr(array_idx_expr_) {}

    expression type_of_expression(void) {
    	return location;
    }

    expr_pointer get_array_idx_expr(void){
        return this->array_idx_expr;
    }

    bool is_object_field(void){
    	return this->ids.size() > 1;
    }

    bool is_array_pos(void){
    	return this->array_idx_expr != nullptr;
    }

    void set_is_attribute_from_actual_object(bool flag){
        this->is_attribute_from_actual_object = flag;        
    }

    bool get_is_attribute_from_actual_object(void){
        return this->is_attribute_from_actual_object;        
    }

    reference_list get_ids(void){
    	return this->ids;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node for a boolean negation of an expression
 */
class node_negate_expr : public node_expr {
public:
    /**
     * :field expression: Expression to apply logical negation
     */
    expr_pointer expression;

    node_negate_expr(expr_pointer expression_) : expression(expression_) {}

    node_expr::expression type_of_expression(void) {
    	return negate_expr;
    }

    Type get_type(void){
    	return this->expression->get_type();
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node for a negative numeric expression
 */
class node_negative_expr : public node_expr {
public:
    /**
     * :field expression: Expression to apply the numerical negative
     */
    expr_pointer expression;

    node_negative_expr(expr_pointer expression_) : expression(expression_) {}

    node_expr::expression type_of_expression(void) {
    	return negative_expr;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

/**
 * Node for an expression between parentheses
 */
class node_parentheses_expr : public node_expr {
public:
    /**
     * :field expression: Expression between the parentheses.
     */
    expr_pointer expression;

    node_parentheses_expr(expr_pointer expression_) : expression(expression_) { }

    node_expr::expression type_of_expression(void) {
    	return parentheses_expr;
    }

    template<typename visitor>
	void accept(visitor& v) {
    	v.visit(*this);
    }
};

#endif
