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

/* Implementation of the class "visitor", that defines the interface for
   every visitor. */

/* TODO: necesité eliminar el calificador const porque hay visitors (como
 	 el de semantic analysis) cuyos métodos sí modifican el nodo que reciben
 	 por parámetro */
class visitor {
public:
	/* Determines the kind of node_expr that it receives as parameter,
	 * and calls the appropriate accept method. */
	void expr_call_appropriate_accept(expr_pointer e) {
		switch(e->type_of_expression()) {
			case node_expr::int_literal: {
				node_int_literal& aux = static_cast<node_int_literal&> (*e);
				aux.accept(*this);
				break;
			}

			case node_expr::bool_literal: {
				node_bool_literal& aux = static_cast<node_bool_literal&> (*e);
				aux.accept(*this);
				break;
			}

			case node_expr::string_literal: {
				node_string_literal& aux = static_cast<node_string_literal&> (*e);
				aux.accept(*this);
				break;
			}

			case node_expr::binary_operation_expr: {
				node_binary_operation_expr& aux = static_cast<node_binary_operation_expr&> (*e);
				aux.accept(*this);
				break;
			}

			case node_expr::location : {
				node_location& aux = static_cast<node_location&> (*e);
				aux.accept(*this);
				break;
			}

			case node_expr::negate_expr : {
				node_negate_expr& aux = static_cast<node_negate_expr&> (*e);
				aux.accept(*this);
				break;
			}

			case node_expr::negative_expr : {
				node_negative_expr& aux = static_cast<node_negative_expr&> (*e);
				aux.accept(*this);
				break;
			}

			case node_expr::parentheses_expr : {
				node_parentheses_expr& aux = static_cast<node_parentheses_expr&> (*e);
				aux.accept(*this);
				break;
			}

			case node_expr::method_call_expr : {
				node_method_call_expr& aux = static_cast<node_method_call_expr&> (*e);
				aux.accept(*this);
				break;
			}
		}
	}

	/* Determines the kind of node_statement that it receives as parameter,
	 * and calls the appropriate accept method. */
	void stm_call_appropriate_accept(statement_pointer s){
		switch(s->type_of_statement()) {
			case node_statement::field_decl : {
					node_field_decl& aux = static_cast<node_field_decl&> (*s);
					aux.accept(*this);
					break;
			}

			case node_statement::block : {
					node_block& aux = static_cast<node_block&> (*s);
					aux.accept(*this);
					break;
			}

			case node_statement::assignment_statement : {
					node_assignment_statement& aux = static_cast<node_assignment_statement&> (*s);
					aux.accept(*this);
					break;
			}

			case node_statement::method_call_statement : {
				node_method_call_statement& aux = static_cast<node_method_call_statement&> (*s);
				aux.accept(*this);
				break;
			}

			case node_statement::if_statement : {
				node_if_statement& aux = static_cast<node_if_statement&> (*s);
				aux.accept(*this);
				break;
			}

			case node_statement::for_statement : {
				node_for_statement& aux = static_cast<node_for_statement&> (*s);
				aux.accept(*this);
				break;
			}

			case node_statement::while_statement : {
				node_while_statement& aux = static_cast<node_while_statement&> (*s);
				aux.accept(*this);
				break;
			}

			case node_statement::return_statement : {
				node_return_statement& aux = static_cast<node_return_statement&> (*s);
				aux.accept(*this);
				break;
			}

			case node_statement::break_statement : {
				node_break_statement& aux = static_cast<node_break_statement&> (*s);
				aux.accept(*this);
				break;
			}

			case node_statement::continue_statement : {
				node_continue_statement& aux = static_cast<node_continue_statement&> (*s);
				aux.accept(*this);
				break;
			}

			case node_statement::skip_statement : {
				node_skip_statement& aux = static_cast<node_skip_statement&> (*s);
				aux.accept(*this);
			}
		}
	}

	// Program
	virtual void visit(node_program& node) = 0;

	// Class declaration
	virtual void visit(node_class_decl& node) = 0;
	virtual void visit(node_field_decl& node) = 0;
	virtual void visit(node_id& node) = 0;
	virtual void visit(node_method_decl& node) = 0;
	virtual void visit(node_parameter_identifier& node) = 0;
	virtual void visit(node_body& node) = 0;
	virtual void visit(node_block& node) = 0;

	// Statements
	virtual void visit(node_assignment_statement& node) = 0;
	virtual void visit(node_method_call_statement& node) = 0;
	virtual void visit(node_if_statement& node) = 0;
	virtual void visit(node_for_statement& node) = 0;
	virtual void visit(node_while_statement& node) = 0;
	virtual void visit(node_return_statement& node) = 0;
	virtual void visit(node_break_statement& node) = 0;
	virtual void visit(node_continue_statement& node) = 0;
	virtual void visit(node_skip_statement& node) = 0;

	// Expressions
	virtual void visit(node_int_literal& node) = 0;
	virtual void visit(node_float_literal& node) = 0;
	virtual void visit(node_bool_literal& node) = 0;
	virtual void visit(node_string_literal& node) = 0;
	virtual void visit(node_binary_operation_expr& node) = 0;
	virtual void visit(node_location& node) = 0;
	virtual void visit(node_negate_expr& node) = 0;
	virtual void visit(node_negative_expr& node) = 0;
	virtual void visit(node_parentheses_expr& node) = 0;
	virtual void visit(node_method_call_expr& node) = 0;

	// Destructor
	//virtual ~visitor() = 0;
};

#endif
