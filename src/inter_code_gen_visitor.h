#ifndef _INTER_CODE_GEN_VISITOR_H_
#define _INTER_CODE_GEN_VISITOR_H_

#include <iostream>
#include <memory>
#include "visitor.h" // Visitor's interface
#include "intermediate_symtable.h" // Symbol table's implementation.
#include "three_address_code.h" // 3-address code's implementation.


class inter_code_gen_visitor : public visitor {
public:
	inter_code_gen_visitor(void);

	const instructions_list* get_inst_list();
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
	virtual void visit(const node_method_call_statement& node);
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
	virtual void visit(const node_method_call_expr& node);



private:
	// List of three-address instructions
	instructions_list *inst_list;
	intermediate_symtable s_table;

	// Information for context-sensitive translation.
	// TODO: el hecho de salirnos de un esquema puro de traducción
	// dirigida por sintaxis, señala algún problema de diseño de
	// nuestro trabajo?
	symtable_element *actual_class;
	unsigned int offset; // To keep track of the next available relative address
				// (page 376 of Dragon book), and share this information
				// between nodes.
	bool into_method;
	unsigned int next_temp; // Next number of temporal variable.
	address_pointer temp; // Temporal where the value of the last analyzed
						   // expression is saved.

	// Initial values of attributes.
	int integer_initial_value;
	float float_initial_value;
	bool boolean_initial_value;

	// Type's width (in bytes).
	unsigned int integer_width;
	unsigned int float_width;
	unsigned int boolean_width;

	/* Translate an instance declaration into a sequence of code to
	 * initialize each attribute.
	 * PRE : {std::string argument is a valid class name}
	 * POS : {instructions to initialize the corresponding attributes are
	 * 		  pushed into the end of inst_list, and the corresponding new
	 * 		  identifiers and offsets are calculated and added to s_table}
	 * */
	void instance_initialization(std::string);

};

#endif
