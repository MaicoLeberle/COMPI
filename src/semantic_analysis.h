#ifndef _SEMANTIC_ANALYSIS_H_
#define _SEMANTIC_ANALYSIS_H_

#include <iostream>
#include "visitor.h" // Visitor's interface
#include "symtable.h" // Symbol table's implementation
#include "node.h"

/*  Checking of the semantic rules. */
class semantic_analysis : public visitor {
public:
    // See COMPI's documentation for a description of error 1 to 19.
    enum error_id {
    	ERROR_1,
    	ERROR_2,
    	ERROR_3,
    	ERROR_4,
    	ERROR_5,
    	ERROR_6,
    	ERROR_7,
    	ERROR_8,
    	ERROR_9,
    	ERROR_10,
    	ERROR_11,
    	ERROR_12,
    	ERROR_13,
    	ERROR_14,
    	ERROR_15,
    	ERROR_16,
    	ERROR_17,
    	ERROR_18,
    	ERROR_19,
    	ERROR_20, // Declaration of identifier with unknown type.
    	ERROR_21, // Trying to access a field from a value that is not an object.
    	ERROR_22, // Method call operation over a value different than a method.
    	ERROR_23, // Parameter's identifier and method's name coincide.
    	ERROR_24,  // Type of attribute is the class where it belongs to.
    	NONE
    };

    semantic_analysis (void);

    error_id get_last_error();

    int get_errors();

    bool is_analysis_successful();

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
	symtables_stack s_table;
	id_type type_l_expr; // Type of the last expression analyzed.
	std::string class_name_l_expr; // Class of the last expression analyzed.
	//symtable_element::id_class class_l_expr; // Symtable element's class of the
											 // last expression analyzed.
	int errors; // Number of errors encountered.
	error_id last_error; // For unit-testing purposes.
	bool well_formed; // Is well-formed the analyzed expression?
    // Attributes to check context-dependent rules.
    bool into_for_or_while; // Are we into a for or while loop?
    bool into_method;
    bool analysis_successful;
    symtable_element *actual_method;
    symtable_element *actual_class;

    /* Converts a type expression described in terms of the Type enum of
	 * node.h, to a type expression suitable for storage into
	 * a symtable_element object. */
	id_type determine_symtable_type(Type::_Type);

	/* Converts a type expression described in terms of the id_type enum of
	 * symtable.h, to a type expression in terms of the Typ enum of node.h.
	 * */
	Type::_Type determine_node_type(id_type type_symtable);

	/* Determines an returns the symtable_element referred by a given reference_list,
	 * , and set well_formed to true, if the given reference_list is well_formed.
	 * In any other case, well_formed is set to false, and NULL is returned.
	 * PRE : {s_table.get_length() > 0}*/
	symtable_element* dereference(reference_list);

	/* Obtains a symtable_element identified by a string, from a given
	 * symtable_element, which must represents a class, a method or a block.
	 * TODO: podría ser también la tabla global?. Returns a pointer to the
	 * symtable_element found or null, if that is not the case.
	 * PRE : {the given symtable_element must represent a class, a method or
	 * 		  a block}
	 * */
	symtable_element *get_next_symtable_element(symtable_element*, std::string);

	/* Adds, and flush, a string to the error's output and counts the
	       error. */
	void register_error(std::string, error_id);

	/* Determines a returns the "wider" type, from the 2 received as parameters,
	 * by considering the hierarchy of numeric types of the language.
	 * PRE : {the types received are numeric}
	 * */
	id_type get_wider_type(id_type, id_type);

	void analyze_method_call(method_call&);
};

#endif
