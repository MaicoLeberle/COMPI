#include "node.h"

/* Implementation of the virtual class "visitor", that defines the interface for
   every visitor. */
class visitor {
    public:
        // Program
        virtual void visit(node_program node) = 0;
        // Class declaration
        virtual void visit(node_class_decl node) = 0;
        virtual void visit(node_class_block node) = 0;
        virtual void visit(node_field_decl node) = 0;
        virtual void visit(node_method_decl node) = 0;
        virtual void visit(node_body node) = 0;
        virtual void visit(node_parameter_identifier node) = 0;
        virtual void visit(node_block node) = 0;
        // Statements
        virtual void visit(node_assignment_statement node) = 0;
        virtual void visit(node_method_call node) = 0;
        virtual void visit(node_if_statement node) = 0;
        virtual void visit(node_for_statement node) = 0;
        virtual void visit(node_while_statement node) = 0;
        virtual void visit(node_return_statement node) = 0;
        virtual void visit(node_break_statement node) = 0;
        virtual void visit(node_continue_statement node) = 0;
        virtual void visit(node_skip_statement node) = 0;
        // Expressions
        virtual void visit(node_negative_expr node) = 0;
        virtual void visit(node_location node) = 0;
        virtual void visit(node_int_literal node) = 0;
        virtual void visit(node_float_literal node) = 0;
        virtual void visit(node_bool_literal node) = 0;
        virtual void visit(node_string_literal node) = 0;
        virtual void visit(node_parentheses_expr node) = 0;
        virtual void visit(node_negate_expr node) = 0;
        virtual void visit(node_binary_operation_expr node) = 0;
    
}
