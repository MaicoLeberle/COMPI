#include "symtable.h"


/*  symtable_entry methods.                                                  */

/*  Each constructor receives the minimum amount of information necessary to
    resolve the id_class, id_type, etc., that the identifier has.            */

symtable_element::symtable_element(id_class not_found_class) :
    c_id(not_found_class) {
    assert(not_found_class == NOT_FOUND);
}

symtable_element::symtable_element(std::string k, id_type t) :
    key(k), c_id(T_VAR), t_id(t) { }

symtable_element::symtable_element(std::string k, id_type t, unsigned int d) : 
    key(k), c_id(T_ARRAY), t_id(t), dim(d) { }

symtable_element::symtable_element(std::string k, std::string s) : 
    key(k), c_id(T_OBJ), t_id(ID), class_type(s) { }  

symtable_element::symtable_element(std::string k, std::string s, unsigned int d) :
    key(k), c_id(T_OBJ_ARRAY), t_id(ID), class_type(s) { }

symtable_element::symtable_element(std::string k, id_type t, std::list<symtable_element>* f) :
    key(k), c_id(T_FUNCTION), t_id(t), func_params(f) { } 

symtable_element::symtable_element(std::string k, std::list<symtable_element>* f) :
    key(k), c_id(T_CLASS), class_fields(f), class_type (k) { }

std::string symtable_element::get_key() {
    return (this->key);
}

symtable_element::id_class symtable_element::get_class () { 
    return c_id; 
}

symtable_element::id_type symtable_element::get_type () { 
    return t_id; 
}

std::string symtable_element::get_class_type () {
    assert(c_id == T_OBJ || c_id == T_OBJ_ARRAY || c_id == T_CLASS);
    return (class_type); 
}

unsigned int symtable_element::get_dimension () { 
    assert(c_id == T_ARRAY || c_id == T_OBJ_ARRAY); 
    return dim; 
}

std::list<symtable_element>* symtable_element::get_func_params () {
    assert(c_id == T_FUNCTION); 
    return (func_params); 
}

std::list<symtable_element>* symtable_element::get_class_fields () {
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

scopes_list::scopes_list(void) {
    this->prev = NULL;
    this->table = new symtable();
}

void scopes_list::push_symtable(void) {
    this->prev = this;
    this->table = new symtable();
}
void scopes_list::push_symtable(symtable_element s) {
    assert ((s.get_class() == symtable_element::T_CLASS)
         || (s.get_class() == symtable_element::T_FUNCTION));
    
    /*  First, a new (block) symbols table must be pushed.                   */
    this->push_symtable();

    /*  Second, each element in the function parameters list or class 
        attributes and methods list must be added to the new symbols table.  */
    std::list<symtable_element>* l;
    if (s.get_class() == symtable_element::T_FUNCTION) 
        l = s.get_func_params();
    if (s.get_class() == symtable_element::T_CLASS) 
        l = s.get_class_fields();

    for(std::list<symtable_element>::iterator it = l->begin(); it != l->end(); it++) 
        (this->table)->put(it->get_key(), *it);
}

void scopes_list::pop_symtable(void) {
    if (this->prev == NULL) {
        delete (this->table);
        this->table = new symtable();
    } else {
        delete (this->table);
        (this->table) = (this->prev)->table;
        (this->prev) = (this->prev)->prev;
    }
}


symtable_element scopes_list::get(std::string key) {
     /*  Must search from the top of the stack and downwards.        */
    for(scopes_list *it = this; it != NULL; it = it->prev) 
        if((it->table)->elem_exists(key))
            return ((it->table)->get(key));

    /*  If the key has not been found in any of the symbols 
        tables, then it has not been found in the current scope.     */
    return (symtable_element(symtable_element::NOT_FOUND));
}

void scopes_list::put(std::string key, symtable_element value) {
    /*  Always insert a new identifier's information in the top
        of the stack.                                               */
    (this->table)-> put(key, value);
}

scopes_list::~scopes_list() {
    if (prev != NULL)
        delete prev;
}