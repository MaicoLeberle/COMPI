#ifndef VISITOR_H_
#define VISITOR_H_

#include "node.h"

/* Implementation of the class "visitor", that defines the interface for
   every visitor. */
class visitor {
public:
	// Program
	void visit(const node_program& node);
	// Class declaration
	void visit(const node_class_decl& node);
	void visit(const node_field_decl& node);
	void visit(const node_id& node);
	void visit(const node_method_decl& node);
	void visit(const node_parameter_identifier& node);
	void visit(const node_body& node);
	void visit(const node_block& node);
	// Statements
	void visit(const node_assignment_statement& node);
	void visit(const node_method_call& node);
	void visit(const node_if_statement& node);
	void visit(const node_for_statement& node);
	void visit(const node_while_statement& node);
	void visit(const node_return_statement& node);
	void visit(const node_break_statement& node);
	void visit(const node_continue_statement& node);
	void visit(const node_skip_statement& node);
	// Expressions
	void visit(const node_int_literal& node);
	void visit(const node_float_literal& node);
	void visit(const node_bool_literal& node);
	void visit(const node_string_literal& node);
	void visit(const node_binary_operation_expr& node);
	void visit(const node_location& node);
	void visit(const node_negate_expr& node);
	void visit(const node_negative_expr& node);
	void visit(const node_parentheses_expr& node);
};

#endif
