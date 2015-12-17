#include "symtable.h"


/*  ----------------------------------------------------------------------------------------------------    
                                            symtable_entry                                                
    ----------------------------------------------------------------------------------------------------    */

/*  START OF CONSTRUCTORS.   */

symtable_element::symtable_element(id_kind not_found_key) : 
    k_id(not_found_key), dim(0), func_params(NULL), class_fields(NULL), class_type(NULL), is_extern(false) {
        assert(not_found_key == K_NOT_FOUND);
}

symtable_element::symtable_element(std::string k, id_type t) : 
    key(k), k_id(K_VAR), t_id(t), dim(0), func_params(NULL), class_fields(NULL), class_type(NULL), is_extern(false) {
    assert((t != T_VOID) && (t != T_ID));
}

symtable_element::symtable_element(std::string k, id_type t, unsigned int d) : 
    key(k), k_id(K_ARRAY), t_id(t), dim(d), func_params(NULL), class_fields(NULL), class_type(NULL), is_extern(false)  { }

symtable_element::symtable_element(std::string k, std::string* s) : 
      key(k)
    , k_id(K_OBJECT)
    , t_id(T_ID)
    , class_type(s)
    , dim(0)
    , func_params(NULL)
    , class_fields(NULL)
    , is_extern(false)  { }  

symtable_element::symtable_element(std::string k, std::string* s, unsigned int d) :
      key(k)
    , k_id(K_OBJECT_ARRAY)
    , t_id(T_ID)
    , class_type(s)
    , dim(d)
    , func_params(NULL)
    , class_fields(NULL)
    , is_extern(false)  { 
        assert(d != 0);
}

symtable_element::symtable_element(std::string k, id_type t, std::list<symtable_element>* f, bool b) :
      key(k)
    , k_id(K_METHOD)
    , t_id(t)
    , func_params(f)
    , dim(0)
    , class_fields(NULL)
    , class_type(NULL)
    , is_extern(b)  { } 

symtable_element::symtable_element(std::string* k, std::list<symtable_element>* f) :
    key(*k), k_id(K_CLASS), t_id(T_ID), class_fields(f), class_type(k), dim(0), func_params(NULL) { 
    }

/*  END OF CONSTRUCTORS.    */

/*  START OF GETTERS.   */

std::string symtable_element::get_key() {
    return (this->key);
}

id_kind symtable_element::get_kind () { 
    return (this->k_id); 
}

id_type symtable_element::get_type () { 
    return (this->t_id); 
}

std::string* symtable_element::get_class_type () {
    return (this->class_type); 
}

unsigned int symtable_element::get_dimension () { 
    assert(k_id == K_ARRAY || k_id == K_OBJECT_ARRAY); 
    return (this->dim); 
}

std::list<symtable_element>* symtable_element::get_func_params () {
    assert(this->k_id == K_METHOD); 
    return (this->func_params); 
}

bool symtable_element::is_func_extern(void) {
    return (this->is_extern);
}

std::list<symtable_element>* symtable_element::get_class_fields () {
    assert(k_id == K_CLASS);
    return (class_fields); 
}

/*  END OF GETTERS. */

/*  START OF PUTTERS.   */

void symtable_element::put_func_param(symtable_element new_elem) {
    assert(this->k_id == K_METHOD);
    assert(this->func_params);
    (this->func_params)->push_back(new_elem);
}

void symtable_element::put_class_field(symtable_element new_elem) {
    assert(this->k_id == K_CLASS);
    assert(this->class_fields);
    (this->class_fields)->push_back(new_elem);
}

/*  END OF PUTTERS. */



/*  ----------------------------------------------------------------------------------------------------    
                                            symtable                                                 
    ----------------------------------------------------------------------------------------------------    */

/*  START OF CONSTRUCTORS.  */
symtable::symtable() : id(NULL), k_symtable(S_BLOCK) {}

symtable::symtable(std::string identifier, bool b) 
    : id(new std::string(identifier)),  k_symtable(b ? S_CLASS : S_METHOD) { }

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
    if (this->id_exists(key) || this->is_recursive(value))
        return false;
    if (this->id != NULL && value.get_kind() == K_CLASS)
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
    if (!this->id || elem.get_kind() != K_OBJECT || !(this->class_or_function))
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
    assert ((s.get_kind() == K_CLASS)
         || (s.get_kind() == K_METHOD));

    /*  First, create the new symbols table. There is no need to insert the 
        parameter element because it should have been done BEFORE wanting to
        create a new symbols table to analize this element (be it class or 
        function).                                                           */
    symtable* new_table = new symtable(s.get_key()
                            , ((s.get_kind() == K_CLASS)? true : false));

    /*  Second, each element in the function parameters list, or class 
        attributes and methods list, must be added to the new symbols table. */
    std::list<symtable_element>* l;
    if (s.get_kind() == K_METHOD) 
        l = s.get_func_params();
    if (s.get_kind() == K_CLASS) 
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
    if(this->size() == 0) {
        symtable_element* s = new symtable_element(K_NOT_FOUND);
        return s;
    }
    symtable* current = (this->stack).front();

     /*  Must search from the top of the stack and downwards.        */
    for(std::list<symtable*>::iterator it = (this->stack).begin(); it != (this->stack).end(); ++it) 
        if((*it)->id_exists(key))
            return ((*it)->get_elem(key));

    /*  If the key has not been found in any of the symbols 
        tables, then it has not been found in the current scope.     */
    symtable_element* not_found_elem = new symtable_element(K_NOT_FOUND);
    return (not_found_elem);
}

symtables_stack::put_results symtables_stack::put(std::string key, symtable_element value) {
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

bool symtables_stack::is_attribute_of_this(std::string key) {
    symtable_element* get_res = this->get(key);
}

/*  END OF VARIABLE TREATMENT METHODS. */

/*  START FUNCTION ANALYSIS METHODS. */

symtables_stack::put_func_results symtables_stack::put_func(std::string key, symtable_element& value) {
    assert(this->last_func == NULL);
    if(value.get_kind() != K_METHOD) 
        return symtables_stack::NOT_FUNC;

    if((this->get(key))->get_kind() != K_NOT_FOUND 
        && (this->get(key))->get_kind() != K_CLASS)
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

    if((!(this->last_func)) || ((this->last_func)->get_kind() == K_NOT_FOUND))
        return symtables_stack::NO_PREV_FUNC;
    
    if((value.get_kind() == K_METHOD) || (value.get_kind() == K_CLASS))
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
    if(value.get_kind() != K_CLASS)
        return symtables_stack::NOT_CLASS;

    if((this->get(key))->get_kind() != K_NOT_FOUND)
        return symtables_stack::CLASS_EXISTS;

    symtable* current = (this->stack).front();
    if(current->put(key, value)) {
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

    if((this->last_class)->get_kind() == K_NOT_FOUND)
        return symtables_stack::NO_PREV_CLASS;
    
    if(value.get_kind() == K_CLASS)
        return symtables_stack::FIELD_TYPE_ERROR;

    ((this->last_class)->get_class_fields())->push_front(value);

    if (value.get_kind() == K_METHOD) {
        symtables_stack::put_func_results res = this->put_func(key, value);
        if(res != symtables_stack::FUNC_PUT)
            return symtables_stack::FIELD_ERROR;
        else
            return symtables_stack::FIELD_PUT;
    } else {
        this->put(key, value);
        return symtables_stack::FIELD_PUT;
    }
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