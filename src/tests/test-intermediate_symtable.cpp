#include <iostream>
#include <cassert> 
#include "../intermediate_symtable.h"

using namespace std;

void test_ids_info();

void test_register_temp();
void test_register_var();
void test_get_type();
void test_set_type();
void test_register_obj();
void test_register_method();
void test_register_class();
void test_ids_info_id_exists();
void test_get_next_internal();
void test_new_temp();
void test_unregister();
void test_ids_info_get_id_rep();
void test_get_kind();
void test_get_offset();
void test_set_number_vars();
void test_get_local_vars();
void test_get_owner_class();
void test_get_list_attributes();



void test_intermediate_symtable();

void test_intermediate_symtable_get_ids_info();
void test_intermediate_symtable_get_id_rep();
void test_intermediate_symtable_push_symtable();
void test_intermediate_symtable_pop_symtable();
void test_intermediate_symtable_put_var_and_get();
void test_intermediate_new_temp();
void test_intermediate_new_temp();
void test_intermediate_symtable_put_obj();
void test_intermediate_symtable_put_func();
void test_intermediate_symtable_put_var_param();
void test_intermediate_symtable_put_obj_param();
void test_intermediate_symtable_finish_func_analysis();
void test_intermediate_symtable_put_class();
void test_intermediate_symtable_put_var_field();
void test_intermediate_symtable_put_obj_field();
void test_intermediate_symtable_put_func_field();
void test_intermediate_symtable_finish_class_analysis();
void test_intermediate_new_label();




void test_ids_info() {
    cout << "\t1) new_temp: ";
    test_register_temp();

    cout << "\t1) register_var: ";
    test_register_var();

    cout << "\t2) register_obj: ";
    test_register_obj();

    cout << "\t3) register_method: ";
    test_register_method();

    cout << "\t4) register_class: ";
    test_register_class();

    cout << "\t5) id_exists: ";
    test_ids_info_id_exists();

    cout << "\t6) get_next_internal: ";
    test_get_next_internal();

    cout << "\t7) new_temp: ";
    test_new_temp();

    cout << "\t8) unregister: ";
    test_unregister();

    cout << "\t9) get_id_rep: ";
    test_ids_info_get_id_rep();

    cout << "\t10) get_kind: ";
    test_get_kind();

    cout << "\t11) get_offset: ";
    test_get_offset();

    cout << "\t12) set_number_vars: ";
    test_set_number_vars();

    cout << "\t13) get_local_vars: ";
    test_get_local_vars();

    cout << "\t14) get_owner_class: ";
    test_get_owner_class();

    cout << "\t15) get_list_attributes: ";
    test_get_list_attributes();
}

void test_register_temp() {
    ids_info information;

    string *res = information.new_temp(10, T_STRING);
    assert(res->compare("@t0") == 0);

    string *res2 = information.new_temp(0, T_UNDEFINED);
    assert(res2->compare("@t1") == 0);

    cout << "OK." << endl;
}

void test_register_var() {
    ids_info information;

    string res = information.register_var(string("variable"), 10, T_UNDEFINED);
    assert(res.compare("variable-0") == 0);
    string res2 = information.register_var(string("variable"), 0, T_UNDEFINED);
    assert(res2.compare("variable-1") == 0);
    string res3 = information.register_var(string("variable"), 5, T_UNDEFINED);
    assert(res3.compare("variable-2") == 0);

    cout << "OK." << endl;
}

void test_get_type() {
    ids_info information;

    information.register_var(string("variable"), 10, T_BOOL);
    assert(information.get_type(string("variable")) == T_BOOL);

    information.new_temp(15, T_FLOAT);
    assert(information.get_type(string("variable")) == T_FLOAT);

    cout << "OK." << endl;
}

void test_set_type() {
    ids_info information;

    information.register_var(string("variable"), 1, T_CHAR);
    assert(information.get_type(string("variable")) == T_CHAR);

    information.set_type(string("variable"), T_UNDEFINED);
    assert(information.get_type(string("variable")) == T_UNDEFINED);

    cout << "OK." << endl;
}

void test_register_obj() {
    ids_info information;

    string res = information.register_obj(string("object"), 0, string("classA"), string("object.x-0"));
    assert(res.compare("object-0") == 0);
    string res2 = information.register_obj(string("object"), 15, string("classB"), string("object.y-0"));
    assert(res2.compare("object-1") == 0);
    string res3 = information.register_obj(string("object"), 2, string("classC"), string("object.x-1"));
    assert(res3.compare("object-2") == 0); 

    cout << "OK." << endl;
}

void test_register_method() {
    ids_info information;

    string res = information.register_method(string("method"), 15, string("classA"));
    assert(res.compare("method::classA") == 0);
    string res2 = information.register_method(string("method"), 6, string("classB"));
    assert(res2.compare("method::classB") == 0);
    string res3 = information.register_method(string("method"), 2, string("classC"));
    assert(res3.compare("method::classC") == 0);

    cout << "OK." << endl;
}

void test_register_class() {
    ids_info information;

    list<string> l;
    string res = information.register_class(string("class"), l);
    assert(res.compare("class-0") == 0);

    list<string> l2;
    l2.push_back(string("first"));
    l2.push_back(string("second"));
    string res2 = information.register_class(string("class"), l2);
    assert(res2.compare("class-1") == 0);

    list<string> l3;
    l3.push_back(string("one"));
    string res3 = information.register_class(string("class"), l3);
    assert(res3.compare("class-2") == 0);

    cout << "OK." << endl;
}

void test_ids_info_id_exists() {
    ids_info information;

    information.register_var(string("variable"), 16, T_UNDEFINED);
    assert(information.id_exists(string("variable")));

    cout << "OK." << endl;
}

void test_get_next_internal() {
    ids_info information;
    information.register_var(string("variable"), 0, T_UNDEFINED);
    information.register_var(string("variable"), 34, T_UNDEFINED);

    assert((information.get_next_internal(string("variable"))).compare(string("variable-2")) == 0);

    cout << "OK." << endl;
}

void test_new_temp() {
    ids_info information;
    std::string* n = information.new_temp(89, T_UNDEFINED);
    std::string* m = information.new_temp(0, T_UNDEFINED);
    std::string* o = information.new_temp(89, T_UNDEFINED);
    
    assert(n);
    assert((*n).compare(std::string("@t0")) == 0);
    assert(m);
    assert((*m).compare(std::string("@t1")) == 0);
    assert(o);
    assert((*o).compare(std::string("@t2")) == 0);
    assert((*n).compare(std::string("@t0")) == 0);

    delete(n);
    delete(m);
    delete(o);
}

void test_unregister() {
    ids_info information;
    information.register_var(string("variable"), 0, T_UNDEFINED);
    assert((information.get_next_internal(string("variable"))).compare(string("variable-1")) == 0);
    information.register_var(string("variable"), 99, T_UNDEFINED);
    assert((information.get_next_internal(string("variable"))).compare(string("variable-2")) == 0);
    information.unregister(string("variable"));
    assert((information.get_next_internal(string("variable"))).compare(string("variable-1")) == 0);
    information.unregister(string("variable"));
    assert((information.get_next_internal(string("variable"))).compare(string("variable-0")) == 0);

    cout << "OK." << endl;
}

void test_ids_info_get_id_rep() {
    ids_info information;

    information.register_var(string("variable"), 1, T_UNDEFINED);
    assert(information.get_id_rep(string("variable")) == string("variable-0"));
    information.register_var(string("variable"), 9, T_UNDEFINED);
    assert(information.get_id_rep(string("variable")) == string("variable-1"));

    cout << "OK." << endl;
}

void test_get_kind() {
    ids_info information;
    information.register_var(string("variable"), 15, T_UNDEFINED);
    assert(information.get_kind(string("variable")) == K_VAR);

    information.register_obj(string("object"), 15, string("classA"), string("object.x-0"));
    assert(information.get_kind(string("object")) == K_OBJECT);
    information.register_method(string("method"), 43, string("classB"));
    assert(information.get_kind(string("method")) == K_METHOD);

    list<string> l;
    l.push_back(string("one"));
    l.push_back(string("two"));
    information.register_class(string("class"), l);
    assert(information.get_kind(string("class")) == K_CLASS);

    cout << "OK." << endl;
}

void test_get_offset() {
    ids_info information;

    information.register_var(string("variable"), 120, T_UNDEFINED);
    assert(information.get_offset(string("variable")) == 120);
    information.register_obj(string("object"), 32, string("classD"), string("object.firstAtt-0"));
    assert(information.get_offset(string("object")) == 32);

    cout << "OK." << endl;
}

void test_set_number_vars() {
    ids_info information;
    
    information.register_method(string("method"), 12, string("classE"));
    assert(information.get_local_vars(string("method")) == 12);

    information.set_number_vars(string("method"), 14);
    assert(information.get_local_vars(string("method")) == 14);

    cout << "OK." << endl;
}

void test_get_local_vars() {
    ids_info information;

    information.register_method(string("method"), 9, string("classE"));
    assert(information.get_local_vars(string("method")) == 9);
    cout << "OK." << endl;
}

void test_get_owner_class() {
    ids_info information;

    information.register_method(string("method"), 15, string("classF"));
    assert((information.get_owner_class(string("method"))).compare("classF") == 0);

    information.register_obj(string("object"), 0, string("classG"), string("object.objA-15"));
    assert((information.get_owner_class(string("object"))).compare("classG") == 0);

    cout << "OK." << endl;
}

void test_get_list_attributes() {
    ids_info information;

    list<string> original_list;
    original_list.push_back(string("1"));
    original_list.push_back(string("2"));
    original_list.push_back(string("three"));
    original_list.push_back(string("extra string"));
    information.register_class(string("class"), original_list);

    list<string> returned_list = information.get_list_attributes(string("class"));

    bool equal = true;
    while (original_list.size() != 0) {
        if (original_list.front() != returned_list.front()) {
            equal = false;
            break;
        } else {
            original_list.pop_front();
            returned_list.pop_front();
        }
    }
    assert(equal);

    cout << "OK." << endl;
}

void test_intermediate_symtable() {
    cout << "\t1) get_ids_info: ";
    test_intermediate_symtable_get_ids_info();

    cout << "\t2) get_id_rep: ";
    test_intermediate_symtable_get_id_rep();

    cout << "\t3) push_symtable: ";
    test_intermediate_symtable_push_symtable();

    cout << "\t4) pop_symtable: ";
    test_intermediate_symtable_pop_symtable();

    cout << "\t5) put_var and get: ";
    test_intermediate_symtable_put_var_and_get();

    cout << "\t6) new_temp: ";
    test_intermediate_new_temp();

    cout << "\t7) put_obj: ";
    test_intermediate_symtable_put_obj();

    cout << "\t8) put_func: ";
    test_intermediate_symtable_put_func();

    cout << "\t9) put_var_param: ";
    test_intermediate_symtable_put_var_param();

    cout << "\t10) put_obj_param: ";
    test_intermediate_symtable_put_obj_param();
    
    cout << "\t11) finish_func_analysis: ";
    test_intermediate_symtable_finish_func_analysis();

    cout << "\t12) put_class: ";
    test_intermediate_symtable_put_class();

    cout << "\t13) put_var_field: ";
    test_intermediate_symtable_put_var_field();

    cout << "\t14) put_obj_field: ";
    test_intermediate_symtable_put_obj_field();

    cout << "\t15) put_func_field: ";
    test_intermediate_symtable_put_func_field();

    cout << "\t16) finish_class_analysis: ";
    test_intermediate_symtable_finish_class_analysis();

    cout << "\t17) new_label: ";
    test_intermediate_new_label();
}

void test_intermediate_symtable_get_ids_info() {
    intermediate_symtable stack;
    assert(stack.get_ids_info() != NULL);
    
    cout << "OK." << endl;
}

void test_intermediate_symtable_get_id_rep() {
    intermediate_symtable stack;
    stack.push_symtable();

    symtable_element elem(string("variable"), symtable_element::INTEGER);
    pair<intermediate_symtable::put_results, string*> res = stack.put_var(elem, elem.get_key(), 17, T_UNDEFINED);
    assert(res.second != NULL);
    assert(string("variable-0").compare(stack.get_id_rep(string("variable"))) == 0);

    symtable_element elem2(string("variable2"), symtable_element::BOOLEAN);
    pair<intermediate_symtable::put_results, string*> res2 = stack.put_var(elem2, elem2.get_key(), 0, T_UNDEFINED);
    assert(res.second != NULL);
    assert(string("variable2-0").compare(stack.get_id_rep(string("variable2"))) == 0);

    cout << "OK." << endl;
}

void test_intermediate_symtable_push_symtable() {
    intermediate_symtable stack;
    
    cout << endl;
    cout << "\t\t* push_symtable(): ";
    assert(stack.size() == 0);
    stack.push_symtable();
    assert(stack.size() == 1);
    stack.push_symtable();
    assert(stack.size() == 2);
    cout << "OK." << endl;

    cout << "\t\t* push_symtable(symtable_element&): ";
    list<symtable_element>* l = new list<symtable_element>();
    symtable_element var(string("variable"), symtable_element::FLOAT);
    l->push_back(var);
    symtable_element method(string("method"), symtable_element::VOID, l);
    stack.push_symtable(method);
    assert(stack.size() == 3);
    cout << "OK." << endl;
}

void test_intermediate_symtable_pop_symtable() {
    intermediate_symtable stack;
    assert(stack.size() == 0);
    
    stack.push_symtable();
    assert(stack.size() == 1);

    list<symtable_element>* l = new list<symtable_element>();
    symtable_element method(string("method"), symtable_element::BOOLEAN, l);
    stack.push_symtable(method);
    assert(stack.size() == 2);

    stack.pop_symtable();
    assert(stack.size() == 1);

    stack.pop_symtable();
    assert(stack.size() == 0);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_var_and_get() {
    intermediate_symtable stack;
    stack.push_symtable();

    symtable_element elem(string("variable"), symtable_element::INTEGER);
    pair<intermediate_symtable::put_results, string*> res = stack.put_var(elem, elem.get_key(), 89, T_UNDEFINED);
    assert(res.first == intermediate_symtable::ID_PUT);
    assert(res.second != NULL);
    assert((*(res.second)).compare(string("variable-0")) == 0);

    list<symtable_element>* l = new list<symtable_element>();
    string* c = new string("classA");
    symtable_element obj(string("object"), c);
    l->push_back(obj);
    symtable_element method(string("method"), symtable_element::VOID, l);
    pair<intermediate_symtable::put_func_results, string*> res2 = stack.put_func(method, method.get_key(), 71, string("classB"));
    assert(res2.first == intermediate_symtable::FUNC_PUT);
    assert(res2.second != NULL);
    assert((*(res2.second)).compare(string("method::classB")) == 0);

    cout << "OK." << endl;
}

void test_intermediate_new_temp() {
    intermediate_symtable stack;
    std::pair<intermediate_symtable::put_results, std::string*> res1 = stack.new_temp(17);
    std::pair<intermediate_symtable::put_results, std::string*> res2 = stack.new_temp(0);
    std::pair<intermediate_symtable::put_results, std::string*> res3 = stack.new_temp(17);
    
    assert(res1.first == intermediate_symtable::ID_PUT);
    assert(res1.second != NULL);
    assert((*res1.second).compare(std::string("@t0")) == 0);
    assert(res2.first == intermediate_symtable::ID_PUT);
    assert(res2.second != NULL);
    assert((*res2.second).compare(std::string("@t1")) == 0);
    assert(res3.first == intermediate_symtable::ID_PUT);
    assert(res3.second != NULL);
    assert((*res3.second).compare(std::string("@t2")) == 0);

    delete(res1.second);
    delete(res2.second);
    delete(res3.second);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_obj() {
    intermediate_symtable stack;
    stack.push_symtable();

    symtable_element obj(string("object"), new string("classC"));
    pair<intermediate_symtable::put_results, string*> res = stack.put_obj(obj, obj.get_key(), 4, string("classC"), string("object.x-0"));
    assert(res.first == intermediate_symtable::ID_PUT);
    assert(res.second != NULL);
    assert((*(res.second)).compare(string("object-0")) == 0);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_func() {
    intermediate_symtable stack;
    stack.push_symtable();

    list<symtable_element>* l = new list<symtable_element>();
    symtable_element var(string("variable"), symtable_element::FLOAT);
    l->push_back(var);
    symtable_element method(string("method"), symtable_element::BOOLEAN, l);
    pair<intermediate_symtable::put_func_results, string*> res = stack.put_func(method, method.get_key(), 16, string("classD"));
    assert(res.first == intermediate_symtable::FUNC_PUT);
    assert(res.second != NULL);
    assert((*(res.second)).compare(string("method::classD")) == 0);
    assert(stack.size() == 2);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_var_param() {
    intermediate_symtable stack;
    stack.push_symtable();

    list<symtable_element>* l = new list<symtable_element>();
    symtable_element method(string("method"), symtable_element::VOID, l);
    stack.put_func(method, method.get_key(), 52, string("classE"));


    symtable_element var(string("variable"), symtable_element::BOOLEAN);
    pair<intermediate_symtable::put_param_results, string*> res = stack.put_var_param(var, var.get_key(), 8, T_UNDEFINED);
    assert(res.first == intermediate_symtable::PARAM_PUT);
    assert(res.second != NULL);
    assert((*(res.second)).compare(string("variable-0")) == 0);
    assert((stack.get(string("variable")))->get_class() == symtable_element::T_VAR);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_obj_param() {
    intermediate_symtable stack;
    stack.push_symtable();

    list<symtable_element>* l = new list<symtable_element>();
    symtable_element method(string("method"), symtable_element::VOID, l);
    stack.put_func(method, method.get_key(), 2, string("classF"));

    string* c = new string("classG");
    symtable_element obj(string("object"), c);
    pair<intermediate_symtable::put_param_results, string*> res = stack.put_obj_param(obj, obj.get_key(), 15, *c, string("object.a-0"));
    assert(res.first == intermediate_symtable::PARAM_PUT);
    assert(res.second != NULL);
    assert((*(res.second)).compare(string("object-0")) == 0);
    assert((stack.get(string("object")))->get_class() == symtable_element::T_OBJ);

    cout << "OK." << endl;
}

void test_intermediate_symtable_finish_func_analysis() {
    intermediate_symtable stack;
    stack.push_symtable();

    list<symtable_element>* l = new list<symtable_element>();
    symtable_element method(string("method"), symtable_element::VOID, l);
    stack.put_func(method, method.get_key(), 2, string("classF"));

    symtable_element obj(string("object"), new string("classG"));
    pair<intermediate_symtable::put_param_results, string*> res = stack.put_obj_param(obj, obj.get_key(), 15, string("classG"), string("object.a-0"));

    assert(stack.size() == 2);
    assert((stack.get(string("method")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack.get(string("object")))->get_class() != symtable_element::NOT_FOUND);
    stack.finish_func_analysis();
    assert(stack.size() == 1);
    assert(stack.get(string("method")) != NULL);
    assert((stack.get(string("object")))->get_class() == symtable_element::NOT_FOUND);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_class() {
    intermediate_symtable stack;
    stack.push_symtable();
    assert(stack.size() == 1);
    assert((stack.get(string("classG")))->get_class() == symtable_element::NOT_FOUND);

    symtable_element c(new string("classG"), new list<symtable_element>());
    stack.put_class(c, c.get_key(), list<string>());

    assert(stack.size() == 2);
    assert(stack.get(string("classG"))->get_class() == symtable_element::T_CLASS);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_var_field() {
    intermediate_symtable stack;
    stack.push_symtable();

    symtable_element c(new string("classG"), new list<symtable_element>());
    stack.put_class(c, c.get_key(), list<string>());

    symtable_element var(string("variable"), symtable_element::BOOLEAN);
    pair<intermediate_symtable::put_field_results, string*> res = stack.put_var_field(var, var.get_key(), 17, T_UNDEFINED);
    assert(res.first == intermediate_symtable::FIELD_PUT);
    assert(res.second != NULL);
    assert((*(res.second)).compare(string("variable-0")) == 0);
    assert((stack.get(string("variable")))->get_class() == symtable_element::T_VAR);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_obj_field() {
    intermediate_symtable stack;
    stack.push_symtable();

    symtable_element c(new string("classG"), new list<symtable_element>());
    stack.put_class(c, c.get_key(), list<string>());

    symtable_element obj(string("object"), new string("classH"));
    pair<intermediate_symtable::put_field_results, string*> res = stack.put_obj_field(obj, obj.get_key(), 17, string("classH"), string("object.y-0"));
    assert(res.first == intermediate_symtable::FIELD_PUT);
    assert(res.second != NULL);
    assert((*(res.second)).compare(string("object-0")) == 0);
    assert((stack.get(string("object")))->get_class() == symtable_element::T_OBJ);

    cout << "OK." << endl;
}

void test_intermediate_symtable_put_func_field() {
    intermediate_symtable stack;
    stack.push_symtable();

    symtable_element c(new string("classG"), new list<symtable_element>());
    stack.put_class(c, c.get_key(), list<string>());

    symtable_element method(string("method"), symtable_element::VOID, new list<symtable_element>());
    pair<intermediate_symtable::put_field_results, string*> res = stack.put_func_field(method, method.get_key(), 50, string("classI"));
    assert(res.first == intermediate_symtable::FIELD_PUT);
    assert(res.second != NULL);
    assert((*(res.second)).compare(string("method::classI")) == 0);
    assert((stack.get(string("method")))->get_class() == symtable_element::T_FUNCTION);

    cout << "OK." << endl;
}

void test_intermediate_symtable_finish_class_analysis() {
    intermediate_symtable stack;
    stack.push_symtable();

    symtable_element c(new string("classG"), new list<symtable_element>());
    stack.put_class(c, c.get_key(), list<string>());

    symtable_element method(string("method"), symtable_element::VOID, new list<symtable_element>());
    stack.put_func_field(method, method.get_key(), 50, string("classI"));
    assert(stack.size() == 3);
    stack.finish_func_analysis();
    assert(stack.size() == 2);
    assert((stack.get(string("method")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack.get(string("classG")))->get_class() != symtable_element::NOT_FOUND);
    stack.finish_class_analysis();
    assert(stack.size() == 1);
    assert((stack.get(string("method")))->get_class() == symtable_element::NOT_FOUND);
    assert((stack.get(string("classG")))->get_class() != symtable_element::NOT_FOUND);

    cout << "OK." << endl;
}

void test_intermediate_new_label() {
    intermediate_symtable stack;
    std::string res1 = stack.new_label();
    std::string res2 = stack.new_label();
    std::string res3 = stack.new_label();

    assert(res1.compare(std::string("L0")) == 0);
    assert(res2.compare(std::string("L1")) == 0);
    assert(res3.compare(std::string("L2")) == 0);

    cout << "OK." << endl;
}

int main() {
    cout << endl << "INTERMEDIATE_SYMTABLE TESTS" << endl << endl;
    cout << "ids_info: " << endl;
    test_ids_info();

    cout << endl << "intermediate_symtable: " << endl;
    test_intermediate_symtable();

    return 0;
}