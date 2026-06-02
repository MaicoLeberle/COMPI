#ifndef LLVM_IR_GENERATOR_H
#define LLVM_IR_GENERATOR_H

#include "visitor.h"

/* Extending the compiler's accepted target languages by adding the possibility
 * of compiling to the LLVM intermediate representation.
 */

class llvm_ir_generator : public visitor {
public:
    /* Inherited interface, allowing the definition to appear outside of this
     * class declaration.
     */
    // Programs
    virtual void visit(node_program&);
    // Class declarations
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
    
};

#endif
