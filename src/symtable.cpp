#include "symtable.h"


/*  ----------------------------------------------------------------------------------------------------    
                                            symtable_entry                                                
    ----------------------------------------------------------------------------------------------------    */

/*  START OF CONSTRUCTORS.   */

symtable_element::symtable_element(id_class not_found_class) : 
    c_id(not_found_class), dim(0), func_params(NULL), class_fields(NULL), class_type(NULL) {
        assert(not_found_class == NOT_FOUND);
}

symtable_element::symtable_element(std::string k, id_type t) : 
    key(k), c_id(T_VAR), t_id(t), dim(0), func_params(NULL), class_fields(NULL), class_type(NULL) {
    assert((t != symtable_element::VOID) && (t != symtable_element::ID));
}

symtable_element::symtable_element(std::string k, id_type t, unsigned int d) : 
    key(k), c_id(T_ARRAY), t_id(t), dim(d), func_params(NULL), class_fields(NULL), class_type(NULL)  { }

symtable_element::symtable_element(std::string k, std::string* s) : 
    key(k), c_id(T_OBJ), t_id(ID), class_type(s), dim(0), func_params(NULL), class_fields(NULL)  { }  

symtable_element::symtable_element(std::string k, std::string* s, unsigned int d) :
    key(k), c_id(T_OBJ_ARRAY), t_id(ID), class_type(s), dim(d), func_params(NULL), class_fields(NULL)  { 
        assert(d != 0);
}

symtable_element::symtable_element(std::string k, id_type t, std::list<symtable_element>* f) :
    key(k), c_id(T_FUNCTION), t_id(t), func_params(f), dim(0), class_fields(NULL), class_type(NULL)  { } 

symtable_element::symtable_element(std::string* k, std::list<symtable_element>* f) :
    key(*k), c_id(T_CLASS), class_fields(f), class_type(k), dim(0), func_params(NULL) { 
    }

symtable_element::symtable_element(symtable_element* from) :
    key(from->get_key()), c_id(from->get_class()), t_id(from->get_type())
  , dim(from->get_dimension()), func_params(from->get_func_params()), class_fields(from->get_class_fields()) { 
        std::string *s = from->get_class_type();
        this->class_type = s;
}

/*  END OF CONSTRUCTORS.    */

/*  START OF GETTERS.   */

std::string symtable_element::get_key() {
    return (this->key);
}

symtable_element::id_class symtable_element::get_class () { 
    return (this->c_id); 
}

symtable_element::id_type symtable_element::get_type () { 
    return (this->t_id); 
}

std::string* symtable_element::get_class_type () {
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

std::list<symtable_element>* symtable_element::get_class_fields () {
    assert(c_id == T_CLASS);
    return (class_fields); 
}

/*  END OF GETTERS. */

/*  START OF PUTTERS.   */

void symtable_element::put_func_param(symtable_element new_elem) {
    assert(this->c_id == T_FUNCTION);
    assert(this->func_params);
    (this->func_params)->push_back(new_elem);
}

void symtable_element::put_class_field(symtable_element new_elem) {
    assert(this->c_id == T_CLASS);
    assert(this->class_fields);
    (this->class_fields)->push_back(new_elem);
}

/*  END OF PUTTERS. */



/*  ----------------------------------------------------------------------------------------------------    
                                            symtable                                                 
    ----------------------------------------------------------------------------------------------------    */

/*  START OF CONSTRUCTORS.  */
symtable::symtable() : id(NULL) {}

symtable::symtable(std::string identifier, bool b) 
    : id(new std::string(identifier)),  class_or_function (b) { }

/*  END OF CONSTRUCTORS.    */

/*  START OF GETTERS.   */

symtable_element* symtable::get_elem(std::string key) {
    assert(id_exists(key));
    return (&((hashtable.find(key))->second));
}

std::string symtable::get_id() {
    assert(this->id);
    return(*(this->id));
}

/*  END OF GETTERS. */

/*  START OF PUTTERS.   */

bool symtable::put (std::string key, symtable_element value) {
    if (id_exists(key) || is_recursive(value))
        return false;
    if (this->id != NULL && value.get_class() == symtable_element::T_CLASS)
        /*  A class cannot be defined inside another class or function.      */
        return false;
    hashtable.insert(std::pair<std::string, symtable_element>(key, value));
    return true;
}

/*  END OF PUTTERS. */

/*  START OF CHECKERS.  */

bool symtable::id_exists(std::string key) {
    return ((this->hashtable).find(key) != (this->hashtable).end());
}
    
bool symtable::is_recursive(symtable_element elem) {
    if (!this->id || elem.get_class() != symtable_element::T_OBJ || !(this->class_or_function))
        return false;
    return((*elem.get_class_type()) == *(this->id));
}

/*  END OF CHECKERS.    */



/*  ----------------------------------------------------------------------------------------------------    
                                            symtables_stack                                                  
    ----------------------------------------------------------------------------------------------------    */

symtables_stack::symtables_stack() : last_class(NULL), last_func(NULL) { }

/*  START OF SYMBOLS TABLES PUSHING AND POPPING METHODS.    */

void symtables_stack::push_symtable() {
    symtable* new_table = new symtable();
    (this->stack).push_front(new_table);
}
void symtables_stack::push_symtable(symtable_element& s) {
    assert ((s.get_class() == symtable_element::T_CLASS)
         || (s.get_class() == symtable_element::T_FUNCTION));

    /*  First, create the new symbols table. There is no need to insert the 
        parameter element because it should have been done BEFORE wanting to
        create a new symbols table to analize this element (be it class or 
        function).                                                           */
    symtable* new_table = new symtable(s.get_key()
                            , ((s.get_class() == symtable_element::T_CLASS)? true : false));

    /*  Second, each element in the function parameters list, or class 
        attributes and methods list, must be added to the new symbols table. */
    std::list<symtable_element>* l;
    if (s.get_class() == symtable_element::T_FUNCTION) 
        l = s.get_func_params();
    if (s.get_class() == symtable_element::T_CLASS) 
        l = s.get_class_fields();

    for(std::list<symtable_element>::iterator it = l->begin(); it != l->end(); it++) 
        new_table->put(it->get_key(), *it);

    (this->stack).push_front(new_table);
}

void symtables_stack::pop_symtable() {
    assert(this->size() != 0);
    (this->stack).pop_front();
}

/*  END OF SYMBOLS TABLES PUSHING AND POPPING METHODS.  */

/*  START OF VARIABLE TREATMENT METHODS.    */

symtable_element* symtables_stack::get(std::string key) {
    assert(this->size() != 0);
    symtable* current = (this->stack).front();

     /*  Must search from the top of the stack and downwards.        */
    for(std::list<symtable*>::iterator it = (this->stack).begin(); it != (this->stack).end(); ++it) 
        if((*it)->id_exists(key))
            return ((*it)->get_elem(key));

    /*  If the key has not been found in any of the symbols 
        tables, then it has not been found in the current scope.     */
    symtable_element* not_found_elem = new symtable_element(symtable_element::NOT_FOUND);
    return (not_found_elem);
}

symtables_stack::put_results symtables_stack::put(std::string key, symtable_element value) {
    assert((value.get_class() != symtable_element::T_FUNCTION) && (value.get_class() != symtable_element::T_CLASS));

    /*  Always insert a new identifier's information in the top
        of the stack; i.e., in the symbols table inserted last.       */
    symtable* current = (this->stack).front();

    if(current-> put(key, value))                                                 
        return symtables_stack::ID_PUT;

    /*  If putting the symbol into the symbols table did not succeeded, there
        are two possible reasons:                                            */
    if (current->id_exists(key))
        return symtables_stack::ID_EXISTS;
    if (current->is_recursive(value))
        return symtables_stack::IS_RECURSIVE;
}

/*  END OF VARIABLE TREATMENT METHODS. */

/*  START FUNCTION ANALYSIS METHODS. */

symtables_stack::put_func_results symtables_stack::put_func(std::string key, symtable_element& value) {
    assert(this->last_func == NULL);
    if(value.get_class() != symtable_element::T_FUNCTION) 
        return symtables_stack::NOT_FUNC;

    if((this->get(key))->get_class() != symtable_element::NOT_FOUND)
        return symtables_stack::FUNC_EXISTS;

    symtable* current = (this->stack).front();
    if(current -> put(key, value)) {
        /*  The function has been added to its class's symbols table.
            Next, it has to be analysed; hence, a new symbols table for it is
            created.                                                         */
        this->push_symtable(value);
        this->last_func = &value;
        return symtables_stack::FUNC_PUT;
    }
    return symtables_stack::FUNC_ERROR;
}

symtables_stack::put_param_results symtables_stack::put_func_param(std::string key, symtable_element& value) {
    assert(this->last_func);
    assert(key.compare(value.get_key()) == 0);

    if((!(this->last_func)) || ((this->last_func)->get_class() == symtable_element::NOT_FOUND))
        return symtables_stack::NO_PREV_FUNC;
    
    if((value.get_class() == symtable_element::T_FUNCTION) || (value.get_class() == symtable_element::T_CLASS))
        return symtables_stack::PARAM_TYPE_ERROR;

    ((this->last_func)->get_func_params())->push_front(value);
    this->put(key, value);
    return symtables_stack::PARAM_PUT;
}

void symtables_stack::finish_func_analysis() {
    assert(this->last_func);
    (this->stack).pop_front();
    this->last_func = NULL;
}

/*  END OF FUNCTION ANALYSIS METHODS.   */

/* START OF CLASS ANALYSIS METHODS. */

symtables_stack::put_class_results symtables_stack::put_class(std::string key, symtable_element& value) {
    assert(this->last_class == NULL);
    if(value.get_class() != symtable_element::T_CLASS)
        return symtables_stack::NOT_CLASS;

    if((this->get(key))->get_class() != symtable_element::NOT_FOUND)
        return symtables_stack::CLASS_EXISTS;

    symtable* current = (this->stack).front();
    if(current -> put(key, value)) {
        /*  The class has been added to the top of the symbols tables stack.
            Next, it has to be analysed; hence, a new symbols table for it is
            created.                                                         */
        this->push_symtable(value);
        this->last_class = &value;
        return symtables_stack::CLASS_PUT;
    }
    return symtables_stack::CLASS_ERROR;
}

symtables_stack::put_field_results symtables_stack::put_class_field(std::string key, symtable_element& value) {
    assert(this->last_class);
    assert(key.compare(value.get_key()) == 0);

    if((this->last_class)->get_class() == symtable_element::NOT_FOUND)
        return symtables_stack::NO_PREV_CLASS;
    
    if(value.get_class() == symtable_element::T_CLASS)
        return symtables_stack::FIELD_TYPE_ERROR;

    ((this->last_class)->get_class_fields())->push_front(value);
    this->put(key, value);
    return symtables_stack::FIELD_PUT;
}

void symtables_stack::finish_class_analysis() {
    assert(this->last_class);
    (this->stack).pop_front();
    this->last_class = NULL;
}

/*  END OF CLASS ANALYSIS METHODS.  */    

unsigned int symtables_stack::size() {
    return ((this->stack).size());
}

symtables_stack::~symtables_stack() {
    for (auto c = (this->stack).begin(); c != (this->stack).end(); c++)
        delete (*c);
}