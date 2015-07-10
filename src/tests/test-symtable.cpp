#include <iostream>
#include <cassert>
#include "../symtable.h"

void test_symtable_element_constructors() {
    std::cout << "\t\t* Not found element: ";
    symtable_element *elem_not_found = new symtable_element(symtable_element::NOT_FOUND);
    assert(elem_not_found->get_class() == symtable_element::NOT_FOUND);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Basic type variable: ";
    std::string key1 = std::string("first var");
    symtable_element *basic_type_var = new symtable_element(key1, symtable_element::BOOLEAN);
    assert(basic_type_var->get_class() == symtable_element::T_VAR);
    assert(basic_type_var->get_type() == symtable_element::BOOLEAN);
    assert((basic_type_var->get_key()).compare(key1) == 0);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Basic type array: ";
    unsigned int dim1 = 6;
    std::string key2 = std::string("second var");
    symtable_element *basic_type_arr = new symtable_element(key2, symtable_element::FLOAT, dim1);
    assert(basic_type_arr->get_class() == symtable_element::T_ARRAY);
    assert(basic_type_arr->get_type() == symtable_element::FLOAT);
    assert((basic_type_arr->get_key()).compare(key2) == 0);
    assert(basic_type_arr->get_dimension() == dim1);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Object: "; 
    std::string key3 = std::string("third var");
    std::string class_id1 = std::string("class");
    symtable_element *obj = new symtable_element(key3, &class_id1);
    assert(obj->get_class() == symtable_element::T_OBJ);
    assert(obj->get_type() == symtable_element::ID);
    assert((obj->get_class_type())->compare(class_id1) == 0);
    assert((obj->get_key()).compare(key3) == 0);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Objects array: ";
    std::string key4 = std::string("fourth var");
    std::string* class_id2 = new std::string("class");
    unsigned int dim2 = 11;
    symtable_element *obj_arr = new symtable_element(key4, class_id2, dim2);
    assert(obj_arr->get_class() == symtable_element::T_OBJ_ARRAY);
    assert(obj_arr->get_type() == symtable_element::ID);
    assert((obj_arr->get_class_type())->compare(*class_id2) == 0);
    assert((obj_arr->get_key()).compare(key4) == 0);
    assert(obj_arr->get_dimension() == dim2);
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Function: ";
    std::string key5 = std::string("first function");
    std::list<symtable_element>* parameters = new std::list<symtable_element>();
    parameters->push_front(*basic_type_var);
    symtable_element *function_elem = new symtable_element(key5, symtable_element::VOID, parameters);
    assert(function_elem->get_class() == symtable_element::T_FUNCTION);
    assert(function_elem->get_type() == symtable_element::VOID);
    assert((function_elem->get_key()).compare(key5) == 0);
    assert(parameters == (function_elem->get_func_params()));
    std::cout << "OK." << std::endl;

    std::cout << "\t\t* Class: ";
    std::string *key6 = new std::string("first class");
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    fields->push_front(*function_elem);
    symtable_element *class_elem = new symtable_element(key6, fields);
    assert(class_elem->get_class() == symtable_element::T_CLASS);
    assert(class_elem->get_type() == symtable_element::ID);
    assert((class_elem->get_key()).compare(*key6) == 0);
    assert(fields == (class_elem->get_class_fields()));
    std::cout << "OK." << std::endl;
}

void test_symtable_element_putters () {
    std::cout << "\t\t* Put function parameters: ";

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


    std::cout << "\t\t* Put class fields: ";
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    std::string *name = new std::string("class");
    symtable_element class_element(name, fields);
    std::list<symtable_element>* fields_got = class_element.get_class_fields();

    std::string *s = new std::string("class it belongs to");
    symtable_element first_field(std::string("object"), s);
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
    std::cout << "\t\t* Symbols table of a class or function: ";
    std::string name("class");
    symtable table1(name,false); // Symbols table of a function
    assert((table1.get_id()).compare(name) == 0);
    std::cout << "OK." << std::endl;
}

void test_symtable_putters() {
    std::cout << "\t\t* Put elems: ";

    symtable table1(std::string("function"), false);

    symtable_element elem1(std::string("variable"), symtable_element::INTEGER);
    assert(table1.put(std::string("variable"), elem1));

    std::list<symtable_element>* fields = new std::list<symtable_element>();
    symtable_element func1(std::string("function"), symtable_element::BOOLEAN, fields);
    assert(table1.put(std::string("function"), func1));

    assert(!(table1.put(std::string("function"), func1)));
    assert(!(table1.put(std::string("function"), elem1)));

    std::cout << "OK." << std::endl;
}

void test_symtable_getters() {
    std::cout << "\t\t* Get elems: ";
    
    symtable table1(std::string("class"), true);
    
    symtable_element elem1(std::string("variable"), symtable_element::INTEGER);
    table1.put(std::string("variable"), elem1);
    assert(table1.get_elem(elem1.get_key()));
    
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    symtable_element func1(std::string("function"), symtable_element::BOOLEAN, fields);
    table1.put(std::string("function"), func1);
    assert(table1.get_elem(func1.get_key()));

    std::cout << "OK." << std::endl;
}

void test_symtable_checkers() {
    std::cout << "\t\t* id_exists: ";

    symtable table1;

    std::string *s1 = new std::string("class");
    symtable_element elem1(std::string("object"), s1);
    table1.put(elem1.get_key(), elem1);

    assert(table1.id_exists(std::string("object")));
    assert(!table1.id_exists(std::string("blablabla")));

    std::cout << "OK." << std::endl;

    std::cout << "\t\t* is_recursive: ";

    symtable table2(std::string("table1"), true);
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    std::string *name = new std::string("function name");
    symtable_element elem2(name, fields);
    std::list<symtable_element>* parameters = new std::list<symtable_element>();
    std::string *s2 = new std::string("other class name");
    symtable_element elem3(std::string("object name"), s2);
    std::string *s3 = new std::string("table1");
    symtable_element elem4(std::string("object name"), s3);

    /*  elem1 is recursive 
        iff
        it is an object, table1 is a symbols table of a class and 
        elem1.get_id() is the same as table1's constructor string. 
        Hence, elem1 is not recursive because it is not an object, nor is
        elem2, since it does not have table1's name as the class_type, even 
        though it is an object. But elem3 is, because its class_type is 
        pricesily table1's name.                                             */
    assert(!(table2.is_recursive(elem2))); 
    assert(!(table2.is_recursive(elem3)));
    assert(table2.is_recursive(elem4));

    std::cout << "OK." << std::endl;
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
    symtables_stack stack1;
    assert(stack1.size() == 0);
    std::cout << "OK." << std::endl;
}
 
void test_symtables_stack_pushs() {
    std::cout << "\t\t* Push a block symtable: ";

    symtables_stack stack1;
    stack1.push_symtable();

    assert(stack1.size() == 1);

    std::cout << "OK." << std::endl;


    std::cout << "\t\t* Push a class symtable: ";

    symtable_element elem1(std::string("name1"), symtable_element::INTEGER, 8);
    std::list<symtable_element>* parameters = new std::list<symtable_element>();
    std::string *name = new std::string("name2");
    symtable_element func1(name, parameters);
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    fields->push_front(func1);
    fields->push_front(elem1);
    std::string *name2 = new std::string("class name");
    symtable_element class1(name2, fields);
    symtables_stack stack2;
    stack2.push_symtable(class1);

    assert(stack2.size() == 1);
    assert((stack2.get("name1"))->get_class() == symtable_element::T_ARRAY);
    assert((stack2.get("name2"))->get_class() != symtable_element::T_FUNCTION);
    assert((stack2.get("unexistent name"))->get_class() == symtable_element::NOT_FOUND);
    stack2.pop_symtable();
    assert(stack2.size() == 0);
    assert((stack2.get("name1"))->get_class() == symtable_element::NOT_FOUND);
    assert((stack2.get("name2"))->get_class() == symtable_element::NOT_FOUND);
    assert((stack2.get("unexistent name"))->get_class() == symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;
}

void test_symtables_stack_size() {
    symtables_stack stack1;
    assert(stack1.size() == 0);
    stack1.push_symtable();
    assert(stack1.size() == 1);
    stack1.push_symtable();
    assert(stack1.size() == 2);
    stack1.pop_symtable();
    assert(stack1.size() == 1);
    stack1.push_symtable();
    stack1.push_symtable();
    assert(stack1.size() == 3);

    std::cout << "OK." << std::endl;
}

void test_symtables_stack_put() {
    std::cout << "\t\t* Putting a function into the stack, finding it"
        ", but not finding its arguments: ";

    symtables_stack stack1;
    stack1.push_symtable();
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    fields->push_front(symtable_element(std::string("name2"), symtable_element::FLOAT));
    fields->push_front(symtable_element(std::string("name3"), symtable_element::BOOLEAN));
    symtable_element elem1(std::string("func1"), symtable_element::VOID, fields);
    stack1.put(elem1.get_key(), elem1);

    /*  When a function is meant to be put inside the symbols tables stack, 
        none of its parameters should be inserted along with it.             */
    assert((stack1.get(std::string("func1")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("name2")))->get_class() == symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("name3")))->get_class() == symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;
}

void test_symtables_stack_pop() {
    std::cout << "\t\t* Putting an element into the stack, popping the top "
        "of the stack and then not finding the element: ";

    symtables_stack stack1;
    stack1.push_symtable();
    std::string *name = new std::string("class name");
    symtable_element elem1(std::string("name1"), name, 2);
    stack1.put(std::string("name1"), elem1);
    assert((stack1.get(std::string("name1")))->get_class() != symtable_element::NOT_FOUND);
    stack1.pop_symtable();
    assert((stack1.get(std::string("name1")))->get_class() == symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;
}

void test_symtables_stack_get() {
    std::cout << "\t\t* Getting an element after pushing a new symbols table: ";

    symtables_stack stack1;
    
    stack1.push_symtable();
    symtable_element elem1(std::string("x"), symtable_element::BOOLEAN);
    stack1.put(elem1.get_key(), elem1);
    assert((stack1.get(std::string("x")))->get_class() != symtable_element::NOT_FOUND);

    stack1.push_symtable();
    symtable_element elem2(std::string("y"), symtable_element::INTEGER);
    stack1.put(elem2.get_key(), elem2);
    assert((stack1.get(std::string("x")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("y")))->get_class() != symtable_element::NOT_FOUND);
    
    stack1.push_symtable();
    symtable_element elem3(std::string("x"), symtable_element::FLOAT);
    stack1.put(elem3.get_key(), elem3);
    assert((stack1.get(std::string("x")))->get_type() == symtable_element::FLOAT);
    assert((stack1.get(std::string("y")))->get_class() != symtable_element::NOT_FOUND);

    stack1.pop_symtable();
    assert((stack1.get(std::string("x")))->get_type() == symtable_element::BOOLEAN);
    assert((stack1.get(std::string("y")))->get_class() != symtable_element::NOT_FOUND);

    stack1.pop_symtable();
    assert((stack1.get(std::string("x")))->get_type() == symtable_element::BOOLEAN);
    assert((stack1.get(std::string("y")))->get_class() == symtable_element::NOT_FOUND);
    
    stack1.pop_symtable();
    assert((stack1.get(std::string("x")))->get_class() == symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;
}

void test_symtables_stack_functions() {
    std::cout << "\t\t* put_func: ";

    symtables_stack stack1;
    stack1.push_symtable();
    assert(stack1.size() == 1);
    std::list<symtable_element>* parameters = new std::list<symtable_element>();
    symtable_element func1(std::string("func1"), symtable_element::BOOLEAN, parameters);
    stack1.put_func(std::string("func1"), func1);
    assert(stack1.size() == 2);
    assert((stack1.get(std::string("func1")))->get_class() != symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;


    std::cout << "\t\t* put_func_param: ";

    std::string *s1 = new std::string("classA");
    symtable_element elem1(std::string("objA"), s1);
    stack1.put_func_param(std::string("objA"), elem1);
    assert((stack1.get(std::string("objA")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("x")))->get_class() == symtable_element::NOT_FOUND);
    symtable_element elem2(std::string("x"), symtable_element::INTEGER);
    stack1.put_func_param(std::string("x"), elem2);
    assert((stack1.get(std::string("objA")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("x")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("func1")))->get_class() != symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;


    std::cout << "\t\t* finish_func_analysis: ";

    stack1.finish_func_analysis();
    assert(stack1.size() == 1);
    assert((stack1.get(std::string("objA")))->get_class() == symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("x")))->get_class() == symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("func1")))->get_class() != symtable_element::NOT_FOUND);
    stack1.pop_symtable();
    assert((stack1.get(std::string("func1")))->get_class() == symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;
}
    
void test_symtables_stack_classes() {
    std::cout << "\t\t* put_class: ";

    symtables_stack stack1;
    stack1.push_symtable();
    assert(stack1.size() == 1);
    std::list<symtable_element>* fields = new std::list<symtable_element>();
    std::string *name = new std::string("class1");
    symtable_element class1(name, fields);
    stack1.put_class(std::string("class1"), class1);
    assert(stack1.size() == 2);
    assert((stack1.get(std::string("class1")))->get_class() != symtable_element::NOT_FOUND);
    
    std::cout << "OK." << std::endl;


    std::cout << "\t\t* put_class_field: ";

    symtable_element elem1(std::string("x"), symtable_element::INTEGER);
    stack1.put_class_field(std::string("x"), elem1);
    assert((stack1.get(std::string("x")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("y")))->get_class() == symtable_element::NOT_FOUND);
    std::string *s1 = new std::string("classB");
    symtable_element elem2(std::string("y"), s1);
    stack1.put_class_field(std::string("y"), elem2);
    assert((stack1.get(std::string("x")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("y")))->get_class() != symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("class1")))->get_class() != symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;


    std::cout << "\t\t* finish_class_analysis: ";

    stack1.finish_class_analysis();
    assert(stack1.size() == 1);
    assert((stack1.get(std::string("x")))->get_class() == symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("y")))->get_class() == symtable_element::NOT_FOUND);
    assert((stack1.get(std::string("class1")))->get_class() != symtable_element::NOT_FOUND);
    stack1.pop_symtable();
    assert((stack1.get(std::string("class1")))->get_class() == symtable_element::NOT_FOUND);

    std::cout << "OK." << std::endl;
}

void test_symtables_stack () {
    std::cout << "\t1) Constructors: ";
    test_symtables_stack_constructors();

    std::cout << "\t2) push and push_symtable: " << std::endl;
    test_symtables_stack_pushs();

    std::cout << "\t3) size: ";
    test_symtables_stack_size();

    std::cout << "\t4) put: " << std::endl;
    test_symtables_stack_put();

    std::cout << "\t5) pop: " << std::endl;
    test_symtables_stack_pop();

    std::cout << "\t1) get: " << std::endl;
    test_symtables_stack_get();

    std::cout << "\t1) put_func, put_func_params and finish_func_analysis: " << std::endl;
    test_symtables_stack_functions();

    std::cout << "\t1) put_class, put_class_field and finish_class_analysis: " << std::endl;
    test_symtables_stack_classes();
}

void test_sample_program() {
    /*  The program to be tested is 
            class Program { 
                void method(){
                    x = 1;
                }
            }

            class main {
                void Main(){}
            }
    */

    /*  Create a symbols tables stack, and push the global symbols table into 
        it.                                                                  */ 
    symtables_stack stack1;
    stack1.push_symtable();

    /*  Create the Program class, along with the list of fields that is to be 
        filled later when every method is met.
        Also, put Program in the global symbols table and begin its analysis.*/
    std::list<symtable_element>* fields1 = new std::list<symtable_element>();
    std::string* class_name1 = new std::string("Program");
    symtable_element class1(class_name1, fields1);
    assert(stack1.put_class(class1.get_key(), class1) == symtables_stack::CLASS_PUT);
    assert(stack1.size() == 2);

    /*  The method function is met, and its representation created and then 
        pushed into Program's fields list. Its analysis is begun (its 
        identifier pushed into Program's symbols table, and new symbols table 
        is created for its analysis, etc) via put_func, but first its previous
        nonexistence is checked.                                            */
    std::list<symtable_element>* parameters1 = new std::list<symtable_element>();
    symtable_element method1(std::string("method"), symtable_element::VOID, parameters1);
    fields1->push_front(method1);
    assert((stack1.get(method1.get_key()))->get_class() == symtable_element::NOT_FOUND);
    assert(stack1.put_func(method1.get_key(), method1) == symtables_stack::FUNC_PUT);
    assert(stack1.size() == 3);

    /*  Here would go method's inner analysis.                               */

    /*  Finish method's and Program's analysis.                              */
    stack1.finish_func_analysis();
    assert(stack1.size() == 2);
    stack1.finish_class_analysis();
    assert(stack1.size() == 1);

    /*  Create the main class, along with the list of fields that is to be
        filled later when every method is met.
        Also, put main in the global symbols table and begin its analysis.   */
    std::list<symtable_element>* fields2 = new std::list<symtable_element>();
    std::string* class_name2 = new std::string("main");
    symtable_element class2(class_name2, fields2);
    assert(stack1.put_class(class2.get_key(), class2) == symtables_stack::CLASS_PUT);
    assert(stack1.size() == 2);

    /*  The Main function is met, and its representation created and then 
        pushed into main's fields list. Its analysis is begun (its identifier
        pushed into main's symbols table, and new symbols table is created for 
        its analysis, etc) via put_func, but first its previous nonexistence is
        checked.                                                             */
    std::list<symtable_element>* parameters2 = new std::list<symtable_element>();
    symtable_element method2(std::string("Main"), symtable_element::VOID, parameters2);
    fields2->push_front(method2);
    assert((stack1.get(std::string("Main")))->get_class() == symtable_element::NOT_FOUND);
    assert(stack1.put_func(method1.get_key(), method1) == symtables_stack::FUNC_PUT);
    assert(stack1.size() == 3);

    /*  Here would go Main's inner analysis, if it had one.                  */

    /*  Finish Main's and main's analysis.                                   */
    stack1.finish_func_analysis();
    assert(stack1.size() == 2);
    stack1.finish_class_analysis();
    assert(stack1.size() == 1);

    std::cout << "OK." << std::endl;
}

int main(int argc, const char* argv[]) {
    std::cout << "symtable_element: " << std::endl;
    test_symtable_element();

    std::cout << std::endl << "symtable: " << std::endl;
    test_symtable();

    std::cout << std::endl << "symtables_stack: " << std::endl;
    test_symtables_stack();

    std::cout << std::endl << "Analyzing a sample program: ";
    test_sample_program();

    

    return 0;
}
