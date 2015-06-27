#ifndef VISITOR_H_
#define VISITOR_H_

#include "node.h"

class node_program;
class node_class_decl;
class node_field_decl;
class node_id;
class node_method_decl;
class node_parameter_identifier;
class node_body;
class node_block;
class node_assignment_statement;
class node_method_call;
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

/* Implementation of the class "visitor", that defines the interface for
   every visitor. */
class visitor {
public:
	/* Determines the kind of node_expr that it receives as parameter,
	 * and calls the appropriate accept method. */
	void expr_call_appropriate_accept(expr_pointer e);

	/* Determines the kind of node_statement that it receives as parameter,
	 * and calls the appropriate accept method. */
	void stm_call_appropriate_accept(statement_pointer s);

	// Program
	virtual void visit(const node_program& node) = 0;

	// Class declaration
	virtual void visit(const node_class_decl& node) = 0;
	virtual void visit(const node_field_decl& node) = 0;
	virtual void visit(const node_id& node) = 0;
	virtual void visit(const node_method_decl& node) = 0;
	virtual void visit(const node_parameter_identifier& node) = 0;
	virtual void visit(const node_body& node) = 0;
	virtual void visit(const node_block& node) = 0;

	// Statements
	virtual void visit(const node_assignment_statement& node) = 0;
	virtual void visit(const node_method_call& node) = 0;
	virtual void visit(const node_if_statement& node) = 0;
	virtual void visit(const node_for_statement& node) = 0;
	virtual void visit(const node_while_statement& node) = 0;
	virtual void visit(const node_return_statement& node) = 0;
	virtual void visit(const node_break_statement& node) = 0;
	virtual void visit(const node_continue_statement& node) = 0;
	virtual void visit(const node_skip_statement& node) = 0;

	// Expressions
	virtual void visit(const node_int_literal& node) = 0;
	virtual void visit(const node_float_literal& node) = 0;
	virtual void visit(const node_bool_literal& node) = 0;
	virtual void visit(const node_string_literal& node) = 0;
	virtual void visit(const node_binary_operation_expr& node) = 0;
	virtual void visit(const node_location& node) = 0;
	virtual void visit(const node_negate_expr& node) = 0;
	virtual void visit(const node_negative_expr& node) = 0;
	virtual void visit(const node_parentheses_expr& node) = 0;

	// Destructor
	virtual ~visitor() = 0;
};

#endif
