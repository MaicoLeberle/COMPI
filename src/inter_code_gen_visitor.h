#ifndef _INTER_CODE_GEN_VISITOR_H_
#define _INTER_CODE_GEN_VISITOR_H_

#include "visitor.h" // Visitor's interface
#include "symtable.h" // Symbol table's implementation.

class inter_code_gen_visitor : public visitor{
public:
	// TODO: esto habría que pasarlo a visitor.h?
	/* Determines the kind of node_statement that it receives as parameter,
	 * and calls the appropriate accept method. */
	void stm_call_appropriate_accept(statement_pointer);

	/* Determines the kind of node_expr that it receives as parameter,
	 * and calls the appropriate accept method. */
	void expr_call_appropriate_accept(expr_pointer);

	// Inherited interface, to allow the definition outside this
	// class declaration.

	// Program
	virtual void visit(const node_program&);
	// Class declaration
	virtual void visit(const node_class_decl& node);
	virtual void visit(const node_field_decl& node);
	virtual void visit(const node_id& node);
	virtual void visit(const node_method_decl& node);
	virtual void visit(const node_parameter_identifier& node);
	virtual void visit(const node_body& node);
	virtual void visit(const node_block& node);
	// Statements
	virtual void visit(const node_assignment_statement& node);
	virtual void visit(const node_method_call& node);
	virtual void visit(const node_if_statement& node);
	virtual void visit(const node_for_statement& node);
	virtual void visit(const node_while_statement& node);
	virtual void visit(const node_return_statement& node);
	virtual void visit(const node_break_statement& node);
	virtual void visit(const node_continue_statement& node);
	virtual void visit(const node_skip_statement& node);
	// Expressions
	virtual void visit(const node_int_literal& node);
	virtual void visit(const node_float_literal& node);
	virtual void visit(const node_bool_literal& node);
	virtual void visit(const node_string_literal& node);
	virtual void visit(const node_binary_operation_expr& node);
	virtual void visit(const node_location& node);
	virtual void visit(const node_negate_expr& node);
	virtual void visit(const node_negative_expr& node);
	virtual void visit(const node_parentheses_expr& node);

private:
	std::list<ThreeAddressCode> *inst_list; // List of three-address instructions
	symtables_stack s_table; // Symbol table

	/* Converts a type expression described in terms of the Type enum of
	 * node.h, to a type expression suitable for storage into
	 * a symtable_element object. */
	symtable_element::id_type determine_type(Type::_Type);
}

#endif
