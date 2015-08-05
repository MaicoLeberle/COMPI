#include <iostream>
#include <cassert> 
#include "../intermediate_symtable.h"

using namespace std;

void test_ids_info();
void test_register_var();
void test_register_obj();
void test_register_method();
void test_register_class();
void test_ids_info_id_exists();
void test_get_next_internal();
void test_unregister();
void test_ids_info_get_id_rep();
void test_get_kind();
void test_get_offset();
void test_set_number_vars();
void test_get_local_vars();
void test_get_owner_class();
void test_get_list_attributes();

void test_intermediate_symtable();
void test_intermediate_symtable_constructor();
void test_intermediate_symtable_get_ids_info();

void test_ids_info() {
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

    cout << "\t7) unregister: ";
    test_unregister();

    cout << "\t8) get_id_rep: ";
    test_ids_info_get_id_rep();

    cout << "\t9) get_kind: ";
    test_get_kind();

    cout << "\t10) get_offset: ";
    test_get_offset();

    cout << "\t11) set_number_vars: ";
    test_set_number_vars();

    cout << "\t12) get_local_vars: ";
    test_get_local_vars();

    cout << "\t13) get_owner_class: ";
    test_get_owner_class();

    cout << "\t14) get_list_attributes: ";
    test_get_list_attributes();
}

void test_register_var() {
    ids_info information;

    /*  Next, 0 is used as an offset everytime the identifier "variable" is 
        registered. This has no particular meaning, just random.             */
    string res = information.register_var(string("variable"), 10);
    assert(res.compare("variable-0") == 0);
    string res2 = information.register_var(string("variable"), 0);
    assert(res2.compare("variable-1") == 0);
    string res3 = information.register_var(string("variable"), 5);
    assert(res3.compare("variable-2") == 0);
}

void test_register_obj() {
    ids_info information;

    string res = information.register_obj(string("object"), 0, string("classA"), string("object.x-0"));
    assert(res.compare("object-0") == 0);
    string res2 = information.register_obj(string("object"), 15, string("classB"), string("object.y-0"));
    assert(res2.compare("object-1") == 0);
    string res3 = information.register_obj(string("object"), 2, string("classC"), string("object.x-1"));
    assert(res3.compare("object-2") == 0); 
}

void test_register_method() {
    ids_info information;

    string res = information.register_method(string("method"), 15, string("classA"));
    assert(res.compare("method-0") == 0);
    string res2 = information.register_method(string("method"), 6, string("classB"));
    assert(res2.compare("method-1") == 0);
    string res3 = information.register_method(string("method"), 2, string("classC"));
    assert(res3.compare("method-2") == 0);
}

void test_register_class() {
    ids_info information;

    list<string> l;
    string res = information.register_class(string("class"), l);
    assert(res.compare("class-0") == 0);

    list<string> l2;
    l2.append(string("first"));
    l2.append(string("second"));
    string res2 = information.register_class(string("class"), l2);
    assert(res2.compare("class-1") == 0);

    list<string> l3;
    l3.append(string("one"));
    string res3 = information.register_class(string("class"), l3);
    assert(res3.compare("class-2") == 0);
}

void test_ids_info_id_exists() {
    ids_info information;

    information.register_var(string("variable"), 16);
    assert(information.id_exists(string("variable")));
}

void test_get_next_internal() {
    ids_information;
    information.register_var(string("variable"), 0);
    information.register_var(string("variable"), 34);

    assert(information.get_next_internal(string("variable")) == 2);
}

void test_unregister() {
    ids_information;
    information.register_var(string("variable"), 0);
    assert(information.get_next_internal(string("variable")) == 1);
    information.register_var(string("variable"), 99);
    assert(information.get_next_internal(string("variable")) == 2);
    information.unregister(string("variable"));
    assert(information.get_next_internal(string("variable")) == 1);
    information.unregister(string("variable"));
    assert(information.get_next_internal(string("variable")) == 0);
}

void test_ids_info_get_id_rep() {
    ids_info information;

    information.register_var(string("variable"), 1);
    assert(information.get_id_rep(string("variable")) == string("variable-0"));
    information.register_var(string("variable"), 9);
    assert(information.get_id_rep(string("variable")) == string("variable-1"));
}

void test_get_kind() {
    ids_info information;

    information.register_var(string("variable"), 15);
    assert(information.get_kind(get_id_rep(string("variable"))) == ids_info::K_VAR);

    information.register_obj(string("object"), 15, string("classA"), string("object.x-0"));
    assert(information.get_kind(get_id_rep(string("object"))) == ids_info::K_OBJECT);

    information.register_method(string("method"), 43, string("classB"));
    assert(information.get_kind(get_id_rep(string("method"))) == ids_info::K_METHOD);

    list<string> l;
    l.append(string("one"));
    l.append(string("two"));
    information.register_class(string("class"), l);
    assert(information.get_kind(get_id_rep(string("class"))) == ids_info::K_CLASS);
}

void test_get_offset() {
    ids_info information;

    information.register_var(string("variable"), 120);
    assert(information.get_offset(get_id_rep(string("variable"))) == 120);

    information.register_obj(string("object"), 32, string("classD"), string("object.firstAtt-0"));
    assert(information.get_offset(get_id_rep(string("object"))) == 32);
}

void test_get_local_vars() {
    ids_info information;

    information.register_method(string("method"), 9, string("classE"));
    assert(information.get_local_vars(get_id_rep(string("method"))) == 9);
}

void test_set_number_vars() {
    ids_info information;
    
    information.register_method(string("method"), 12, string("classE"));
    assert(information.get_local_vars(get_id_rep(string("method"))) == 12);

    information.set_number_vars(get_id_rep(string("method"))), 14);
    assert(information.get_local_vars(get_id_rep(string("method"))) == 14);
}

void test_get_owner_class() {
    ids_info information;

    information.register_method(string("method"), 15, string("classF"));
    assert((information.get_owner_class(get_id_rep(string("method")))).compare("classF") == 0);

    information.register_obj(string("object"), 0, string("classG"), string("object.objA-15"));
    assert((information.get_owner_class(get_id_rep(string("method")))).compare("classG") == 0);
}

void test_get_list_attributes() {
    ids_info information;

    list<string> original_list;
    original_list.append(string("1"));
    original_list.append(string("2"));
    original_list.append(string("three"));
    original_list.append(string("extra string"));
    information.register_class(string("class"), original_list);

    list<string> returned_list = information.get_list_attributes(get_id_rep(string("class")));

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
}

void test_intermediate_symtable() {
    cout << "\t1) get_ids_info: ";
    test_intermediate_symtable_get_ids_info();
}

void test_intermediate_symtable_get_ids_info() {
    intermediate_symtable table;
    assert(table.get_ids_info() != NULL);
}


int main() {
    cout << "ids_info: " << endl;
    test_ids_info();

    cout << "intermediate_symtable: " << endl;
    test_symtables_stack();

    return 0;
}