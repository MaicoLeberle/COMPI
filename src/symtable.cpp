#include "symtable.h"


/*  symtable_entry methods.                                                  */

/*  Each constructor receives the minimum amount of information necessary to
    resolve the id_class, id_type, etc., that the identifier has.            */

symtable_element::symtable_element(id_class not_found_class) 
    : c_id(not_found_class) {
        assert(not_found_class == NOT_FOUND);
}

symtable_element::symtable_element(id_type t) :
    c_id(T_VAR), t_id(t) { }

symtable_element::symtable_element(id_type t, unsigned int d) : 
    c_id(T_ARRAY), t_id(t), dim(d) { }

symtable_element::symtable_element(std::string* s) : 
    c_id(T_OBJ), t_id(ID), class_type(s) { }  

symtable_element::symtable_element(std::string* s, unsigned int d) :
    c_id(T_OBJ_ARRAY), t_id(ID), class_type(s) { }

symtable_element::symtable_element(id_type t, std::list<std::string>* f) :
     c_id(T_FUNCTION), t_id(t), func_params(f) { } 

symtable_element::symtable_element(std::string* s, std::list<std::string>* f) :
    c_id(T_CLASS), class_fields(f), class_type (s) { }

symtable_element::id_class symtable_element::get_class () { 
    return c_id; 
}

symtable_element::id_type symtable_element::get_type () { 
    return t_id; 
}

std::string* symtable_element::get_class_type () {
    assert(c_id == T_OBJ || c_id == T_OBJ_ARRAY || c_id == T_CLASS);
    return (class_type); 
}

unsigned int symtable_element::get_dimension () { 
    assert(c_id == T_ARRAY || c_id == T_OBJ_ARRAY); 
    return dim; 
}

std::list<std::string>* symtable_element::get_func_params () {
    assert(c_id == T_FUNCTION); 
    return (func_params); 
}

std::list<std::string>* symtable_element::get_class_fields () {
    assert(c_id == T_CLASS);
    return (class_fields); 
}

/*  symtable methods.                                                        */

bool symtable::elem_exists (std::string key) {
    return (hashtable.find(key) != hashtable.end());
}

symtable_element symtable::get(std::string key) {
    assert(elem_exists(key));
    return ((hashtable.find(key))->second);
}

void symtable::put (std::string key, symtable_element value) {
    hashtable.insert(std::pair<std::string, symtable_element>(key, value));
}

/*  scopes_list methods.                                                     */

symtable_element scopes_list::get(std::string key) {
     /*  Must search from the top of the stack and downwards.        */
    for(scopes_list *it = this; it != NULL; it = it->prev) 
        if((it->table).elem_exists(key))
            return ((it->table).get(key));

    /*  If the key has not been found in any of the symbols 
        tables, then it has not been found in the current scope.     */
    return (symtable_element(symtable_element::NOT_FOUND));
}

void scopes_list::put(std::string key, symtable_element value) {
    /*  Always insert a new identifier's information in the top
        of the stack.                                               */
    table.put(key, value);
}

scopes_list::~scopes_list() {
    if (prev != NULL)
        delete prev;
}