#include <iostream>
#include <vector>
#include <llvm/Value.h>


class CodeGenContext;
class node {
public:
    virtual ~node() { }
    virtual llvm::Value* codeGen(CodeGenContext& context) { }
};

class class_decl;
class program : public node {
public:
    std::vector<class_decl*> classes;
    program(std::vector<class_decl*> list_classes) : classes(list_classes) { }
    virtual llvm::Value* codeGen(CodeGenContext& context) { }
};

class field_decl;
class method_decl;
class class_decl : public node {
public:
    std::string id;
    std::vector<declaration*> class_block;
    class_decl(std::string ident, std::vector<declaration*> c_block) :
        id(ident), class_block(c_block) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
}; 

class declaration : public node {
public:
    virtual llvm::Value* codeGen(CodeGenContext& context); 
};

class identifier_decl;
class field_decl : public declaration {
public:
    std::string f_type;
    std::vector<identifier_decl*> ids;
    field_decl(std::string t, std::vector<identifier_decl*> list_ids) : f_type(t), ids(list_ids) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class body;
class typed_identifier;
class method_decl : public declaration {
public:
    std::string m_type;
    std::string id;
    std::vector<typed_identifier*> formal_params;
    body m_body;
    method_decl(std::string method_type, std::string method_id, std::vector<typed_identifier*> method_formal_params, body method_body) :
        m_type(method_type), id(method_id), formal_params(method_formal_params), m_body(method_body);
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class block;
class body : public node {
public:
    bool extern_body;
    block* b_block;
    body(block* body_block) : extern_body(false), b_block(body_block) { }
    body() : extern_body(true), b_block(NULL) { }
    bool is_extern() { return extern_body; }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class statements;
class block : public node {
public:
    std::vector<statements*> block_content;
    block(std::vector<statements*> content) : block_content(content) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class statements : public node {
public:
};

class field_decl_statement : public statements, public field_decl{ 
public:
    field_decl_statement(std::string t, std::vector<identifier_decl*> list_ids) : field_decl(t, list_ids) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class location;
class expr;
class assignment_statement : public statements {
public:
    location* loc;
    expr* e;
    assignment_statement(location* assignment_location, expr* assignment_expression) : 
        loc(assignment_location), e(assignment_expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class method_call;
class method_call_statement : public statements {
public:
    method_call* m;
    method_call_statement(method_call* method) : m(method) { }
};

class if_statement : public statements {
public:
    expr* e;
    statement* true_statement;
    statement* false_statement;
    if_statement(expr* expression, statement* t) : 
        e(expression), true_statement(t), false_statement(NULL) {}
    if_statement(expr* expression, statement* t, statement* f) :
        e(expression), true_statement(t), false_statement(f) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class for_statement : public statements {
public:
    identifier* id;
    expr* from;
    expr* to;
    statement* s;
    for_statement(identifier* ident, expr* from_expression, expr* to_expression, statement* for_body) :
        id(ident), from(from_expression), to(to_expression), s(for_body) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class while_statement : public statements {
public:
    expr* e;
    statement* s;
    while_statement(expr* while_guard, statement* while_body) : e(while_guard), s(while_body) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class return_statement : public statements {
public:
    expr* e;
    return_statement(expr* return_expression) : e(return_expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class break_statement : public statements {
public:
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class continue_statement : public statements {
public:
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class skip_statement : public statements {
public:
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class block_statement : public statements {
public:
    statement* s;
    block_statement(statement* statement_block) : s(statement_block) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class assign_op {
public:
    std::string op;
    assign_op(std::string operator_symbol) : op(operator_symbol) { }
};

class method_call : public node {
public:
    std::vector<identifier*> ids;
    std::vector<expr*>* e;
    method_call(std::vector<identifier*> list_ids) : ids(list_ids) { }
    method_call(std::vector<identifier*> list_ids, std::vector<expr*>* expression) :
        ids(list_ids), e(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class location : public node {
public:
    std::vector<identifier*> ids;
    expr* e;
    location(std::vector<identifier*> location_ids) : ids(location_ids) { }
    location(std::vector<identifier*> location_ids, expr* index) :
        ids(location_ids), e(index) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class expr : public node {
};

class location_expr : public expr {
public:
    location l;
    location_expr(location loc) : l(loc) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class method_call_expr : public expr {
public:
    method_call m;
    method_call_expr(method_call m_call) : m(m_call) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class literal;
class literal_expr : public expr {
public:
    literal* l;
    literal_expr(literal* l_expression) : l(l_expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class bin_op;
class binary_operation_expr : public expr {
public:
    bin_op* op;
    expr* lhs_expr;
    expr* rhs_expr;
    binary_operation_expr(bin_op* binary_operator, expr* left_expression, expr* right_expression) :
        op(binary_operator), lhs_expr(left_expression), rhs_expr(right_expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class minus_expr : public expr {
public:
    expr e;
    minus_expr(expr expression) : e(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class negate_expr : public expr {
public:
    expr e;
    negate_expr(expr expression) : e(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class parenthesis_expr : public expr {
public:
    expr e;
    parenthesis_expr(expr expression) : e(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class bin_op {
public:
    std::string op;
    bin_op(std::string operator_symbol) : op(operator_symbol) { }
};

class arith_op {
public:
    std::string op;
    arith_op(std::string operator_symbol) : op(operator_symbol) { }
};

class rel_op {
public:
    std::string op;
    rel_op(std::string operator_symbol) : op(operator_symbol) { }
};

class eq_op {
public:
    std::string op;
    eq_op(std::string operator_symbol) : op(operator_symbol) { }
};

class cond_op {
public:
    std::string op;
    cond_op(std::string operator_symbol) : op(operator_symbol) { }
};

class literal : public node {
public:
};

class identifier {
public:
    std::string id;
    identifier(std::string s) : id(s) { }
};

class typed_identifier :public identifier {
public:
    std::string type;
    typed_identifier(std::string i_type, std::string i_value) : type(i_type), identifier(i_value) { }
};

class identifier_decl : public identifier {
public:
    int size;
    identifier_decl(std::string id) : identifier(id), size(1) { }
    identifier_decl(std::string id, int identifier_size) : identifier(id), size(identifier_size) { }
};

class int_literal : public literal { 
public:
    long long  value;
    int_literal(long long val) : value(val) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class float_literal : public literal {
public:
    double value;
    float_literal(double val) : value(val) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class bool_literal : public literal { 
public:
    bool value;
    bool_literal(bool val) : value(val) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);    
};

class string_literal : public literal { 
public:
    std::string value;
    string_literal(const std::string val) : value(val) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};