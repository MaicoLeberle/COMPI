#ifndef _INTER_CODE_GEN_VISITOR_H_
#define _INTER_CODE_GEN_VISITOR_H_

#include <iostream>
#include <memory>
#include "visitor.h" // Visitor's interface
#include "intermediate_symtable.h" // Symbol table's implementation.
#include "three_address_code.h" // 3-address code's implementation.
#include "constants.h" // Information about types.

class inter_code_gen_visitor : public visitor {
public:
	inter_code_gen_visitor(void);

	instructions_list* get_inst_list();

	// TODO: hace falta este método?
	intermediate_symtable* get_symtable(void);

	ids_info* get_ids_info(void);

	// Inherited interface, to allow the definition outside this
	// class declaration.
	// Program
	virtual void visit(node_program&);
	// Class declaration
	virtual void visit(node_class_decl& node);
	virtual void visit(node_field_decl& node);
	virtual void visit(node_id& node);
	virtual void visit(node_method_decl& node);
	virtual void visit(node_parameter_identifier& node);
	virtual void visit(node_body& node);
	virtual void visit(node_block& node);
	// Statements
	virtual void visit(node_assignment_statement& node);
	virtual void visit(node_method_call_statement& node);
	virtual void visit(node_if_statement& node);
	virtual void visit(node_for_statement& node);
	virtual void visit(node_while_statement& node);
	virtual void visit(node_return_statement& node);
	virtual void visit(node_break_statement& node);
	virtual void visit(node_continue_statement& node);
	virtual void visit(node_skip_statement& node);
	// Expressions
	virtual void visit(node_int_literal& node);
	virtual void visit(node_float_literal& node);
	virtual void visit(node_bool_literal& node);
	virtual void visit(node_string_literal& node);
	virtual void visit(node_binary_operation_expr& node);
	virtual void visit(node_location& node);
	virtual void visit(node_negate_expr& node);
	virtual void visit(node_negative_expr& node);
	virtual void visit(node_parentheses_expr& node);
	virtual void visit(node_method_call_expr& node);



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


	/* PRE : {id_class is an already registered class name}
	 * POS : {instructions to initialize the attributes of id_instance are
	 * 		  added, and the corresponding new identifiers and offsets are
	 * 		  calculated and added to symbol's table}
	 * */
	std::string instance_initialization(std::string id_class,
										std::string id_instance);

	void array_initialization(symtable_element::id_type type,
							std::string id,
							unsigned int array_length);

	unsigned int calculate_size(symtable_element::id_type, std::string);

	symtable_element::id_type determine_type(Type::_Type);

	void translate_var_def(symtable_element::id_type, std::string, std::string,
							unsigned int array_size);

	/* Returns the label that identifies the beginning of the method's translation
	 * (in the intermediate code generated).
	 * PARAM: ids: a reference_list that represents the identifier used for the call.
	 * RETURNS: a std::string representing the label's content.
	 * */
	std::string obtain_methods_label(reference_list ids);

	int get_attribute_offset(reference_list ids);

};

#endif
