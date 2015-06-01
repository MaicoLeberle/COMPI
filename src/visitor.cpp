#include <iostream>
#include <typeinfo>
#include "visitor.h"

void visitor::visit(const node_program& node) {
	for(auto c : node.classes) {
		c->accept(*this);
	}
}

void visitor::visit(const node_class_decl& node) {
	std::cout << "Accessing class " << node.id << std::endl;

	for(auto cb : node.class_block) {
		cb->accept(*this);
	}
}

void visitor::visit(const node_field_decl& node) {
	for(auto f : node.ids) {
		f->accept(*this);
	}
}

void visitor::visit(const node_id& node) {
	std::cout << "Accessing field " << node.id << " of dim " << node.array_size << std::endl;
}

void visitor::visit(const node_method_decl& node) {
	std::cout << "Accessing method " << node.id << std::endl;

	for(auto p : node.parameters) {
		p->accept(*this);
	}

	node.body->accept(*this);
}

void visitor::visit(const node_parameter_identifier& node) {
	std::cout << "Accessing parameter " << node.id << std::endl;
}

void visitor::visit(const node_body& node) {
	std::cout << "Accessing body of method" << std::endl;

	if(node.is_extern)
		std::cout << "The body is extern" << std::endl;
	else {
		std::cout << "The body is declared" << std::endl;

		node.block->accept(*this);
	}
}

void visitor::visit(const node_block& node) {
	for(auto s : node.content) {
		s->accept(*this);
	}
}

void visitor::visit(const node_assignment_statement& node) {
	std::cout << "Accessing assignment statement" << std::endl;

	node.location->accept(*this);

	node.expression->accept(*this);
}

void visitor::visit(const node_method_call& node) {
	std::cout << "Accessing method call statement" << std::endl;

	for(auto r : node.ids) {
		std::cout << "Accessing reference " << r << std::endl;
	}

	for(auto e : node.parameters) {
		e->accept(*this);
	}
}

void visitor::visit(const node_if_statement& node){
	std::cout << "Accessing if statement" << std::endl;

	node.expression->accept(*this);

	node.then_statement->accept(*this);

	if(node.else_statement != nullptr) {
		node.else_statement->accept(*this);
	}
}

void visitor::visit(const node_for_statement& node){
	std::cout << "Accessing for statement" << std::endl;

	node.from->accept(*this);

	node.to->accept(*this);

	node.body->accept(*this);
}

void visitor::visit(const node_while_statement& node) {
	std::cout << "Accessing while statement" << std::endl;

	node.expression->accept(*this);

	node.body->accept(*this);
}

void visitor::visit(const node_return_statement& node) {
	std::cout << "Accessing return statement" << std::endl;

	if(node.expression != nullptr) {
		node.expression->accept(*this);
	}
}

void visitor::visit(const node_break_statement& node) {
	std::cout << "Accessing break statement" << std::endl;
}

void visitor::visit(const node_continue_statement& node) {
	std::cout << "Accessing continue statement" << std::endl;
}

void visitor::visit(const node_skip_statement& node) {
	std::cout << "Accessing skip statement" << std::endl;
}

void visitor::visit(const node_int_literal& node) {
	std::cout << "Accessing int literal of value " << node.value << std::endl;
}

void visitor::visit(const node_float_literal& node) {
	std::cout << "Accessing float literal of value " << node.value << std::endl;
}

void visitor::visit(const node_bool_literal& node) {
	std::cout << "Accessing boolean literal of value " << node.value << std::endl;
}

void visitor::visit(const node_string_literal& node) {
	std::cout << "Accessing float literal of value " << node.value << std::endl;
}

void visitor::visit(const node_binary_operation_expr& node) {
	std::cout << "Accessing binary operation" << std::endl;

	node.left->accept(*this);

	node.right->accept(*this);
}

void visitor::visit(const node_location& node) {
	std::cout << "Accessing location expression" << std::endl;

	for(auto r : node.ids) {
		std::cout << "Accessing reference " << r << std::endl;
	}

	node.array_idx_expr->accept(*this);
}

void visitor::visit(const node_negate_expr& node) {
	std::cout << "Accessing negate expression" << std::endl;

	node.expression->accept(*this);
}

void visitor::visit(const node_negative_expr& node) {
	std::cout << "Accessing negative expression" << std::endl;

	node.expression->accept(*this);
}

void visitor::visit(const node_parentheses_expr& node) {
	std::cout << "Accessing parentheses expression" << std::endl;

	node.expression->accept(*this);
}



