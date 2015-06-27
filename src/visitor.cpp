#include "visitor.h"

void visitor::expr_call_appropriate_accept(expr_pointer e) {
	switch(e->type_of_expression()) {
			case node_expr::int_literal :
				node_int_literal& aux = static_cast<node_int_literal&> (*e);
				aux.accept(*this);
				break;

			case node_expr::bool_literal :
				node_bool_literal& aux = static_cast<node_bool_literal&> (*e);
				aux.accept(*this);
				break;

			case node_expr::string_literal :
				node_string_literal& aux = static_cast<node_string_literal&> (*e);
				aux.accept(*this);
				break;

			case node_expr::binary_operation_expr :
				node_binary_operation_expr& aux = static_cast<node_binary_operation_expr&> (*e);
				aux.accept(*this);
				break;

			case node_expr::location :
				node_location& aux = static_cast<node_location&> (*e);
				aux.accept(*this);
				break;

			case node_expr::negate_expr :
				node_negate_expr& aux = static_cast<node_negate_expr&> (*e);
				aux.accept(*this);
				break;

			case node_expr::negative_expr :
				node_negative_expr& aux = static_cast<node_negative_expr&> (*e);
				aux.accept(*this);
				break;

			case node_expr::parentheses_expr :
				node_parentheses_expr& aux = static_cast<node_parentheses_expr&> (*e);
				aux.accept(*this);
				break;

			case node_expr::method_call_expr :
				node_method_call& aux = static_cast<node_method_call&> (*e);
				aux.accept(*this);
				break;
		}
}

void visitor::stm_call_appropriate_accept(statement_pointer s){
	switch(s->type_of_statement()) {
		case node_statement::field_decl :
				node_field_decl& aux = static_cast<node_field_decl&> (*s);
				aux.accept(*this);
				break;

		case node_statement::block :
				node_block& aux = static_cast<node_block&> (*s);
				aux.accept(*this);
				break;

		case node_statement::assignment_statement :
				node_assignment_statement& aux = static_cast<node_assignment_statement&> (*s);
				aux.accept(*this);
				break;

		case node_statement::method_call_statement :
			node_method_call& aux = static_cast<node_method_call&> (*s);
			aux.accept(*this);
			break;

		case node_statement::if_statement :
			node_if_statement& aux = static_cast<node_if_statement&> (*s);
			aux.accept(*this);
			break;

		case node_statement::for_statement :
			node_for_statement& aux = static_cast<node_for_statement&> (*s);
			aux.accept(*this);
			break;

		case node_statement::while_statement :
			node_while_statement& aux = static_cast<node_while_statement&> (*s);
			aux.accept(*this);
			break;

		case node_statement::return_statement :
			node_return_statement& aux = static_cast<node_return_statement&> (*s);
			aux.accept(*this);
			break;

		case node_statement::break_statement :
			node_break_statement& aux = static_cast<node_break_statement&> (*s);
			aux.accept(*this);
			break;

		case node_statement::continue_statement :
			node_continue_statement& aux = static_cast<node_continue_statement&> (*s);
			aux.accept(*this);
			break;

		case node_statement::skip_statement :
			node_skip_statement& aux = static_cast<node_skip_statement&> (*s);
			aux.accept(*this);
	}
}
