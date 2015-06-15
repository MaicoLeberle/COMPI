#ifndef _SEMANTIC_ANALYSIS_H_
#define _SEMANTIC_ANALYSIS_H_

#include <iostream>
#include "visitor.h" // Visitor's interface
#include "symtable.h" // Symbol table's implementation
#include "node.h"

/*  Checking of the semantic rules. */
class semantic_analysis : public visitor {
public:
    // See COMPI's documentation for a description of each error.
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
    };

    semantic_analysis (void);

    /* Adds, and flush, a string to the error's output and counts the
       error. */
    void register_error(std::string, error_id);

    error_id get_last_error();

    /* Converts a type expression described in terms of the Type enum of
     * node.h, to a type expression suitable for storage into
     * a symtable_element object. */
    symtable_element::id_type determine_type(Type::_Type);

    /* Determines and returns the symtable_element referred by a given
     * reference_list.
     * PRE : {s_table.get_length() > 0}*/
    symtable_element::id_type dereference_get_element(reference_list);

    /* Obtains a symtable_element identified by a string, from a given
     * element of the symbol table.
     * PRE : {symtable_element->get_class() != symtable_element::NOT_FOUND}
     * */
    symtable_element *get_next_symtable_element(symtable_element*, std::string);

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
	symtables_stack s_table; // TODO: si no describo este atributo en el constructor
							// qué pasa al intentar crear una instancia?
	symtable_element::id_type type_l_expr; // Type of the last expression analyzed.
					// TODO: está bueno hacerlo así?
	int errors; // Number of errores encountered.
	error_id last_error; // For unit-testing purposes.
	bool well_formed; // Is well-formed the analyzed expression?
    // Attributes to check context-dependent rules.
    // TODO: peligroso?
    bool into_for_or_while; // Are we into a for or while loop?
};

#endif
