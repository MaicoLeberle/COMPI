#ifndef NODE2_H
#define NODE2_H

#include <iostream>
#include <vector>

class node {
public:
    virtual ~node() { }
    //virtual llvm::Value* codeGen(CodeGenContext& context) { }
};

class node_class_decl;
class node_program : public node {
public:
    std::vector<node_class_decl*> classes;
    // Constructors are defined to receive just the information
    // available in the semantic actions where a new
    // node must be constructed.
    program(node_class_decl *a_class){ 
        ids = new std::vector<node_class_decl*>(a_class);
    }
    //virtual llvm::Value* codeGen(CodeGenContext& context) { }
};

class node_class_decl : public node {
public:
    std::string id;
    std:vector<node_class_block*> *class_block;
    class_decl(std::string ident, std:vector<node_class_block*> *c_block) :
        id(ident), class_block(c_block) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_class_block : public node {
    
};


class node_field_decl : public node_class_block {
public:
    int type;
    node_ids *ids;
    field_decl(int t, node_ids *list_id) : type(t), ids(list_id) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_ids : public node {
public:
    std::vector<std::string*> ids;
    int index;
    node_ids(std::string* id){ 
        ids = new std::vector<std::string*>(id);
    }
    node_ids(std::string* id, int ind){ 
        ids = new std::vector<std::string*>(id);
        index = ind;
    }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_body;
class node_typed_identifier;
class node_method_decl : public node_class_block {
public:
    std::string m_type;
    std::string id;
    std::vector<node_typed_identifier*> *formal_params;
    node_body *m_body;
    method_decl(std::string method_type, std::string method_id, 
    std::vector<node_typed_identifier*> *method_formal_params, node_body *method_body) :
    m_type(method_type), id(method_id), formal_params(method_formal_params), 
    m_body(method_body);
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_typed_identifier :public node {
public:
    int type;
    std::string id;
    typed_identifier(int i_type, std::string i_value) : type(i_type), id(i_value) { }
};

class node_statement;
class node_body : public node {
public:
    // TODO: invariante de clase: extern_body <=> b_block == NULL ?
    bool extern_body;
    std::vector<node_statement*> *b_block;
    node_body(std::vector<node_statements*> *body_block) : extern_body(false), b_block(body_block) { }
    node_body() : extern_body(true), b_block(NULL) { }
    bool is_extern() { return extern_body; }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_statement : public node {

};

class node_location : public node_expr {
public:
    std::vector<string*> *ids;
    node_expr* e;
    location(std::vector<string*> location_ids) : ids(location_ids) { }
    location(std::vector<string*> location_ids, node_expr* index) :
        ids(location_ids), e(index) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_expr;
class node_assignment_statement : public node_statement {
public:
    node_location* loc;
    int assign_op;
    node_expr* e;
    node_assignment_statement(node_location* assignment_location, int a_op, 
    node_expr* assignment_expression) : 
        loc(assignment_location), assign_op(a_op), e(assignment_expression) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_method_call : public node {
public:
    std::vector<std::string*> *ids;
    std::vector<node_expr*> *actual_parameters;
    node_method_call(std::vector<std::string*> *list_ids) : ids(list_ids) { }
    node_method_call(std::vector<std::string*> *list_ids, std::vector<node_expr*> *expressions) :
        ids(list_ids), actual_parameters(expressions) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_if_statement : public node_statement {
public:
    node_expr* e;
    node_statement* true_statement;
    node_statement* false_statement;
    node_if_statement(node_expr* expression, node_statement* t) : 
        e(expression), true_statement(t), false_statement(NULL) {}
    node_if_statement(node_expr* expression, node_statement* t, node_statement* f) :
        e(expression), true_statement(t), false_statement(f) {}
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_for_statement : public node_statement {
public:
    std::string *id;
    node_expr* from;
    node_expr* to;
    node_statement* s;
    node_for_statement(std::string* ident, node_expr* from_expression, 
    node_expr* to_expression, node_statement* for_body) :
        id(ident), from(from_expression), to(to_expression), s(for_body) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_while_statement : public node_statement {
public:
    node_expr* e;
    node_statement* s;
    node_while_statement(node_expr* while_guard, node_statement* while_body) 
    : e(while_guard), s(while_body) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_return_statement : public node_statement {
public:
    node_expr* e;
    node_return_statement(node_expr* return_expression) : e(return_expression) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_break_statement : public node_statement {
public:
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_continue_statement : public node_statement {
public:
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_skip_statement : public node_statement {
public:
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};
/*
    EXPRESSIONS
*/
class node_expr : public node {
};

class node_literal : public node_expr {  
};  

class node_int_literal : public node_literal { 
public:
    long long  value;
    node_int_literal(long long val) : value(val) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_float_literal : public node_literal {
public:
    double value;
    node_float_literal(double val) : value(val) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_bool_literal : public node_literal { 
public:
    bool value;
    node_bool_literal(bool val) : value(val) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);    
};

class node_string_literal : public node_literal { 
public:
    std::string value;
    node_string_literal(const std::string val) : value(val) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_binary_operation_expr : public expr {
public:
    char op;
    expr* lhs_expr;
    expr* rhs_expr;
    node_binary_operation_expr(char binary_operator, expr* left_expression, expr* right_expression) :
        op(binary_operator), lhs_expr(left_expression), rhs_expr(right_expression) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_location : public node_expr {
public:
    std::vector<std::string*> ids;
    node_expr* e;
    location(std::vector<std::string*> location_ids) : ids(location_ids) { }
    location(std::vector<std::string*> location_ids, node_expr* index) :
        ids(location_ids), e(index) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_negate_expr : public node_expr {
public:
    node_expr e;
    node_negate_expr(node_expr expression) : e(expression) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_minus_expr : public node_expr {
public:
    node_expr e;
    node_minus_expr(node_expr expression) : e(expression) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};

class node_parenthesis_expr : public node_expr {
public:
    node_expr e;
    node_parenthesis_expr(node_expr expression) : e(expression) { }
    //virtual llvm::Value* codeGen(CodeGenContext& context);
};


#endif
