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

symtable_element::symtable_element(symtable_element* from) :
    key(from->get_key()), c_id(from->get_class()), t_id(from->get_type()), class_type(from->get_class_type())
  , dim(from->get_dimension()), func_params(from->get_func_params()), class_fields(from->get_class_fields()) { }

std::string symtable_element::get_key() {
    return (this->key);
}

symtable_element::id_class symtable_element::get_class () { 
    return (this->c_id); 
}

symtable_element::id_type symtable_element::get_type () { 
    return (this->t_id); 
}

std::string symtable_element::get_class_type () {
    assert(this->c_id == T_OBJ || this->c_id == T_OBJ_ARRAY || this->c_id == T_CLASS);
    return (this->class_type); 
}

unsigned int symtable_element::get_dimension () { 
    assert(c_id == T_ARRAY || c_id == T_OBJ_ARRAY); 
    return (this->dim); 
}

std::list<symtable_element>* symtable_element::get_func_params () {
    assert(this->c_id == T_FUNCTION); 
    return (this->func_params); 
}

void symtable_element::put_func_param(symtable_element new_elem) {
    assert(this->c_id == T_FUNCTION);
    assert(this->func_params);
    (this->func_params)->push_back(new_elem);
}

std::list<symtable_element>* symtable_element::get_class_fields () {
    assert(c_id == T_CLASS);
    return (class_fields); 
}

void symtable_element::put_class_field(symtable_element new_elem) {
    assert(this->c_id == T_CLASS);
    assert(this->class_fields);
    (this->class_fields)->push_back(new_elem);
}

/*  symtable methods.                                                        */

symtable::symtable() : id(NULL) {}

symtable::symtable(std::string identifier, bool b) 
    : id(new std::string(identifier)),  class_or_function (b) { }

bool symtable::elem_exists (std::string key) {
    return (hashtable.find(key) != hashtable.end());
}

symtable_element* symtable::get_elem(std::string key) {
    assert(elem_exists(key));
    return (&((hashtable.find(key))->second));
}

std::string symtable::get_id() {
    assert(this->id);
    return(*(this->id));
}

bool symtable::id_exists(std::string id) {
    return ((this->hashtable).find(id) != (this->hashtable).end());
}
    
bool symtable::is_recursive(symtable_element elem) {
    assert(this->id);
    if (elem.get_class() != symtable_element::T_OBJ || !(this->class_or_function))
        return false;
    return(elem.get_class_type() == *(this->id));
}


bool symtable::put (std::string key, symtable_element value) {
    if (id_exists(key) || is_recursive(value))
        return false;
    hashtable.insert(std::pair<std::string, symtable_element>(key, value));
    return true;
}

/*  symtables_stack methods.                                                  */

symtables_stack::symtables_stack(void) : last_elem(NULL) { }

void symtables_stack::push_symtable(void) {
    symtable* new_table = new symtable();
    (this->stack).push_front(new_table);
}
void symtables_stack::push_symtable(symtable_element s) {
    assert ((s.get_class() == symtable_element::T_CLASS)
         || (s.get_class() == symtable_element::T_FUNCTION));

    /*  First, create the new symbols table. There is no need to insert the 
        parameter element because it should have been done BEFORE wanting to
        create a new symbols table to analize this element (be it class or 
        function).                                                           */
    symtable* new_table = new symtable(s.get_key()
                            , ((s.get_class() == symtable_element::T_CLASS)? true : false));

    /*  Second, each element in the function parameters list or class 
        attributes and methods list must be added to the new symbols table.  */
    std::list<symtable_element>* l;
    if (s.get_class() == symtable_element::T_FUNCTION) 
        l = s.get_func_params();
    if (s.get_class() == symtable_element::T_CLASS) 
        l = s.get_class_fields();

    for(std::list<symtable_element>::iterator it = l->begin(); it != l->end(); it++) 
        new_table->put(it->get_key(), *it);

    (this->stack).push_front(new_table);
}

void symtables_stack::pop_symtable(void) {
    assert(!this->is_empty());
    (this->stack).pop_front();
}


symtable_element* symtables_stack::get(std::string key) {
    assert(!this->is_empty());
    symtable* current = (this->stack).front();

     /*  Must search from the top of the stack and downwards.        */
    for(std::list<symtable*>::iterator it = (this->stack).begin(); it != (this->stack).end(); ++it) 
        if((*it)->elem_exists(key))
            return ((*it)->get_elem(key));

    /*  If the key has not been found in any of the symbols 
        tables, then it has not been found in the current scope.     */
    symtable_element* not_found_elem = new symtable_element(symtable_element::NOT_FOUND);
    return (not_found_elem);
}

symtables_stack::put_results symtables_stack::put(std::string key, const symtable_element& value) {
    assert(!this->is_empty());

    /*  Always insert a new identifier's information in the top
        of the stack; i.e., in the symbols table inserted last.       */
    symtable* current = (this->stack).front();

    if(current-> put(key, value)) {
        /*  Update last_elem.                                                    */
        this->last_elem = new symtable_element(value);
        return symtables_stack::INSERTED;
    }

    /*  If putting the symbol into the symbols table did not succeeded, there
        are two possible reasons:                                            */
    if (current->id_exists(key))
        return symtables_stack::ID_EXISTS;
    if (current->is_recursive(value))
        return symtables_stack::IS_RECURSIVE;
}

symtables_stack::put_results symtables_stack::put_func_param(symtable_element value) {
    assert(this->last_elem);

    if ((this->last_elem)->get_class() != symtable_element::T_FUNCTION)
        return symtables_stack::NOT_FUNCTION;
    
    (this->last_elem)->put_func_param(value);
    return symtables_stack::INSERTED;
}

symtables_stack::put_results symtables_stack::put_class_field(symtable_element value) {
    assert(this->last_elem);

    if ((this->last_elem)->get_class() != symtable_element::T_CLASS)
        return symtables_stack::NOT_CLASS;

    (this->last_elem)->put_class_field(value);
    return symtables_stack::INSERTED;
}

unsigned int symtables_stack::get_length() {
    return ((this->stack).size());
}

bool symtables_stack::is_empty() {
    return ((this->stack).size() == 0);
}

symtables_stack::~symtables_stack() {
    for (auto c = (this->stack).begin(); c != (this->stack).end(); c++)
        delete (*c);
}