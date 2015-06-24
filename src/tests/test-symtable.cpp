#include <iostream>
#include <cassert>
#include "../symtable.h"

void test_symtable_element_constructors() {
    symtable_element *elem_not_found
                   , *basic_type_var;
                   , *basic_type_arr;
                   , *obj;
                   , *obj_arr;
                   , *function_elem;
                   , *class_elem;
                   , *copy_constructor_elem;


    std::cout << "\t\t* Not found element: ";
    elem_not_found = new symtable_element(symtable_element::NOT_FOUND);
    assert(elem_not_found->get_class() == symtable_element::NOT_FOUND);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Basic type variable: ";
    std::string key1 = std::string("first var");
    basic_type_var = new symtable_element(key1, symtable_element::BOOLEAN);
    assert(basic_type_var->get_class() == symtable_element::T_VAR);
    assert(basic_type_var->get_type(symtable_element::BOOLEAN));
    assert((basic_type_var->get_key()).compare(key1) == 0);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Basic type array: ";
    unsigned int dim1 = 6;
    std::string key2 = std::string("second var");
    basic_type_arr = new symtable_element(key2, symtable_element::FLOAT, dim1);
    assert(basic_type_arr->get_class() == symtable_element::T_ARRAY);
    assert(basic_type_arr->get_type() == symtable_element::FLOAT);
    assert((basic_type_arr->get_key()).compare(key2) == 0);
    assert(basic_type_arr->get_dimension() == dim1);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Object: "; 
    std::string key3 = std::string("third var");
    std::string class_id1 = std::string("class");
    obj = new symtable_element(key3, class_id1);
    assert(obj->get_class() == symtable_element::T_OBJ);
    assert(obj->get_type() == symtable_element::ID);
    assert((obj->get_class_type()).compare(class_id1) == 0);
    assert((obj->get_key()).compare(key3) == 0);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Objects array: ";
    std::string key4 = std::string("fourth var");
    std::string class_id2 = std::string("class");
    unsigned int dim2 = 11;
    obj_arr = new symtable_element(key4, class_id2, dim2);
    assert(obj_arr->get_class() == symtable_element::T_OBJ_ARRAY);
    assert(obj_arr->get_type() == symtable_element::ID);
    assert((obj_arr->get_class_type()).compare(class_id2) == 0);
    assert((obj_arr->get_key()).compare(key4) == 0);
    assert(obj_arr->get_dimension() == dim2);
    cout << "OK." << std::endl;

    std::cout << "\t\t* Function: ";
    std::string key5 = std::string("first function");
    std::list<symtable_element>* parameters = new std::list<symtable_element>();
    parameters->push_front(basic_type_var);
    function_elem = new symtable_element(key5, symtable_element::VOID, parameters);
    assert(function_elem->get_class() == symtable_element::T_FUNCTION);
    assert(function_elem->get_type() == symtable_element::VOID);
    assert((function_elem->get_key()).compare(key5) == 0);
    assert(parameters == (function_elem->get_func_params()));
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Class: ";
    std::string key6 = std::string("first class");
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    fields->push_front(function_elem);
    class_elem = new symtable_element(key6, fields);
    assert(class_elem->get_class() == symtable_element::T_CLASS);
    assert(class_elem->get_type() == symtable_element::ID);
    assert((class_elem->get_key()).compare(key6) == 0);
    assert(fields == (class_elem->get_class_fields()));
    std::cout << "OK." << std::endl;
}

void test_symtable_element_putters () {
    std::cout << "\t\tPut function parameters: ";

    std::list<symtable_element>* parameters = new std::list<symtable_element>();
    symtable_element function_elem(std::string("function"), symtable_element::FLOAT, parameters);
    std::list<symtable_element>* parameters_got = function_elem.get_func_params();

    symtable_element first_parameter(std::string("basic type variable 1"), symtable_element::INTEGER);
    parameters->push_front(first_parameter);
    parameters_got->push_front(first_parameter);
    symtable_element second_parameter(std::string("basic type variable 2"), symtable_element::BOOLEAN);
    parameters->push_front(second_parameter);
    parameters_got->push_front(second_parameter);

    assert(parameters == (function_elem.get_func_params()));
    std::cout << "OK." << std::endl;


    std::cout << "\t\tPut class fields: ";
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    symtable_element class_element(std::string("class"), fields);
    std::list<symtable_element>* fields_got = class_element.get_class_fields();

    symtable_element first_field(std::string("object"), std::string("class it belongs to"));
    fields->push_front(first_field);
    fields_got->push_front(first_field);
    symtable_element second_field(function_elem); // Re-using function_elem as a class field.
    fields->push_front(second_field);
    fields_got->push_front(second_field);

    assert(fields == (class_element.get_class_fields()));
    std::cout << "OK." << std::endl;
}

void test_symtable_element() {
    std::cout << "\t1) Constructors: " << std::endl;
    test_symtable_element_constructors();

    std::cout << "\t2) Putters and getters: " << std::endl;
    test_symtable_element_putters();
}

void test_symtable_constructors() {
    std::cout << "\t\tNo arguments: ";
    symtable table1();
    assert(table1.get_id() == NULL);
    assert((table1.hashtable).size() == 0); /*  Check if the map has been 
                                                correctly created.           */
    std::cout << "OK." << std::endl;

    std::cout << "\t\tSymbols table of a class or function: " << std::endl;
    std::string name("class");
    symtable table2(name,false); // Symbols table of a function
    assert((table2.get_id()).compare(name) == 0);
    assert((table2.hashtable).size() == 0);
    std::cout << "OK." << std::endl;
}

void test_symtable_putters() {
    std::cout << "\t\tPut elems: ";

    symtable table1(std::string("function"), false);

    sytamble_element elem1(std::string("variable"), symtable_element::INTEGER);
    assert(table1.put(std::string("variable"), elem1));

    std::list<symtable_element>* fields = new std::list<symtable_element>();
    symtable_element func1(std::string("function"), symtable_element::BOOLEAN, fields);
    assert(table1.put(std::string("function"), func1));

    assert(!(table1.put(std::string("function"), func1)));
    assert(!(table1.put(std::strin("function"), elem1)));

    std::cout << "OK." << std::endl;
}

void test_symtable_getters() {
    std::cout << "\t\tGet elems: ";
    
    symtable table1(std::string("class"), true);
    
    symtable_element elem1(std::string("variable"), symtable_element::INTEGER);
    table1.put(std::string("variable"), elem1);
    assert(table1.get_elem(elem1.get_key()));
    
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    symtable_element func1(std::string("function"), symtable_element::BOOLEAN, fields);
    table2.put(std::string("function"), func1);
    assert(table1.get_elem(func1.get_key()));

    std::cout << "OK." << std::endl;
}

void test_symtable_checkers() {
    std::cout << "\t\tid_exists: ";

    symtable table1();

    symtable_element elem1(std::string("object"), std::string("class"));
    table1.put(elem1.get_key(), elem1);

    assert(table1.id_exists(std::string("object")));
    assert(table1.id_exists(std::string("blablabla")));

    std::cout << "OK." << std::endl;

    std::cout << "\t\tis_recursive: ";

    symtable table1(std::string("name"), true);
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    symtable_element elem1(std::string("name"), fields);
    std::list<symtable_element>* parameters = new std::list<symtable_element>();
    symtable_element elem2(std::string("na"), symtable_element::BOOLEAN, parameters);

    assert(table1.is_recursive(elem1));
    assert(!(table1.is_recursive(elem2)));

    std::cout << "OK." << std:endl;
}

void test_symtable() {
    std::cout << "\t1) Constructors: " << std::endl;
    test_symtable_constructors();

    std::cout << "\t2) Putters: " << std::endl;
    test_symtable_putters();

    std::cout << "\t3) Getters: " << std::endl;
    test_symtable_getters();

    std::cout << "\t4) Checkers: " << std::endl;
    test_symtable_checkers();
}

void test_symtables_stack_constructors() {

}
 
void test_symtables_stack_pushs() {

}

void test_symtables_stack_size() {

}

void test_symtables_stack_pop() {

}

void test_symtables_stack_put() {

}

void test_symtables_stack_get() {

}

void test_symtables_stack_functions() {

}
    
void test_symtables_stack_classes() {

}

void test_symtables_stack () {
    test_symtables_stack_constructors();
    test_symtables_stack_pushs();
    test_symtables_stack_size();
    test_symtables_stack_pop();
    test_symtables_stack_put();
    test_symtables_stack_get();
    test_symtables_stack_functions();
    test_symtables_stack_classes();
}

int main(int argc, const char* argv[]) {
    std::cout << "symtable_element is going to be checked now: " << std::endl;
    test_symtable_element();

    std::cout << "symtable is going to be checked now: " << std::endl;
    test_symtable();

    std::cout << "symtables_stack is going to be checked now: " << std::endl;
    test_symtables_stack();

    return 0;
}