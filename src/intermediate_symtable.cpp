#include <iostream>
#include "intermediate_symtable.h"


/*  ----------------------------------------------------------------------------------------------------    
                                            ids_info                                                 
    ----------------------------------------------------------------------------------------------------    */

std::string ids_info::get_next_internal(std::string key) {
    /*  The internal representations range key-0, key-1, key-2, ...          */
    if ((this->internal).find(key) == (this->internal).end()) 
        (this->internal).insert(std::pair<std::string, unsigned int>(key, 0));
    

    return ((key + '@') + std::to_string(((this->internal).find(key))->second));
}

std::string* ids_info::new_temp(int o
                              , id_type type) {
    std::string* ret = new std::string("@t" + std::to_string(this->temp_number));
    assert((this->info_map).find(*ret) == (this->info_map).end());

    entry_info information;

    information.entry_kind = K_TEMP;
    information.entry_type = type;
    information.offset = new int(o);

    (this->info_map).insert(std::pair<std::string, entry_info>(*ret, information));
    
    (this->temp_number)++;
    return ret;
}

std::string ids_info::register_var(std::string key
                                 , int offset
                                 , id_type type) {
    entry_info information;
    
    information.entry_kind = K_VAR;
    information.entry_type = type;
    information.offset =  new int(offset);

    std::string internal_key = this->get_next_internal(key);
    ++((this->internal).find(key))->second;
    information.rep = internal_key;

    (this->info_map).insert(std::pair<std::string, entry_info>(key, information));

    return internal_key;
}

id_type ids_info::get_type(std::string key) {
    assert((this->info_map).find(key) != (this->info_map).end());
    assert((this->get_kind(key) == K_TEMP) || 
           (this->get_kind(key) == K_VAR));

    return ((((this->info_map).find(key))->second).entry_type);
}

void ids_info::set_type(std::string key
                      , id_type type) {
    assert((this->info_map).find(key) != (this->info_map).end());
    assert((this->get_kind(key) == K_TEMP) || 
           (this->get_kind(key) == K_VAR));

    (((this->info_map).find(key))->second).entry_type = type;
}

std::string ids_info::register_obj(std::string key
                                 , int offset
                                 , std::string owner
                                 , std::string address) {
    entry_info information;
    
    information.entry_kind = K_OBJECT;
    information.offset = new int(offset);
    information.owner = new std::string(owner);
    information.begin_address = new std::string(address);

    std::string internal_key = this->get_next_internal(key);
    ++((this->internal).find(key))->second;
    information.rep = internal_key;

    (this->info_map).insert(std::pair<std::string, entry_info>(key, information));

    return internal_key;
}

std::string ids_info::register_method(std::string key
                                    , unsigned int locals
                                    , std::string owner) {
    entry_info information;

    information.entry_kind = K_METHOD;
    information.local_vars = new unsigned int(locals);
    information.owner = new std::string(owner);
    information.l_params = new std::list<std::string>;

    std::string internal_key = key + "::" + owner;
    information.rep = internal_key;

    (this->info_map).insert(std::pair<std::string, entry_info>(key, information));

    return internal_key;
}

std::string ids_info::register_class(std::string key
                                   , t_attributes attributes) {
    entry_info information;
    
    information.entry_kind = K_CLASS;
    information.l_atts = new t_attributes(attributes);
    
    std::string internal_key = this->get_next_internal(key);
    ++((this->internal).find(key))->second;
    information.rep = internal_key;

    (this->info_map).insert(std::pair<std::string, entry_info>(key, information));

    return internal_key;
}

void ids_info::unregister(std::string key) {
    assert((this->internal).find(key) != (this->internal).end());
    assert(((this->internal).find(key))->second != 0);

    --((this->internal).find(key))->second;
}

bool ids_info::id_exists(std::string key) {
    return ((this->info_map).find(key) != (this->info_map).end());
}

std::string ids_info::get_id_rep(std::string key) {
    assert((this->info_map).find(key) != (this->info_map).end());
    
    if ((((this->info_map).find(key))->second).entry_kind == K_METHOD) {
        return (key + "::" + *((((this->info_map).find(key))->second).owner));
    } else {
        assert((this->internal).find(key) != (this->internal).end());
        return (key + '@' + std::to_string(((this->internal).find(key))->second - 1));
    }
}

id_kind ids_info::get_kind(std::string key) {
    assert((this->info_map).find(key) != (this->info_map).end());

    return ((((this->info_map).find(key))->second).entry_kind);
}

int ids_info::get_offset(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == K_VAR
        || (((this->info_map).find(key))->second).entry_kind == K_OBJECT
        || (((this->info_map).find(key))->second).entry_kind == K_TEMP);

    return *((((this->info_map).find(key))->second).offset);
}

unsigned int ids_info::get_local_vars(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == K_METHOD);

    return *((((this->info_map).find(key))->second).local_vars);
}

std::string ids_info::get_owner_class(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == K_METHOD
        || (((this->info_map).find(key))->second).entry_kind == K_OBJECT);
    
    return *((((this->info_map).find(key))->second).owner);
}

t_attributes& ids_info::get_list_attributes(std::string key) {
    assert(this->id_exists(key));
    assert((((this->info_map).find(key))->second).entry_kind == K_CLASS);
    assert((((this->info_map).find(key))->second).l_atts);

    return *((((this->info_map).find(key))->second).l_atts);
}

t_params& ids_info::get_list_params(std::string key) {
    assert(this->id_exists(key));
    assert((((this->info_map).find(key))->second).entry_kind == K_METHOD);
    assert((((this->info_map).find(key))->second).l_params);

    return(*((((this->info_map).find(key))->second).l_params));
}

void ids_info::set_number_vars(std::string key,
                               unsigned int number) {
    assert((this->info_map).find(key) != (this->info_map).end());
    assert((((this->info_map).find(key))->second).entry_kind == K_METHOD);

    if(!(((this->info_map).find(key))->second).local_vars)
        (((this->info_map).find(key))->second).local_vars = new unsigned int(number);
    else
        *((((this->info_map).find(key))->second).local_vars) = number;
}

void ids_info::set_offset(std::string key, 
                          int number) {
    assert((this->info_map).find(key) != (this->info_map).end());
    assert((((this->info_map).find(key))->second).entry_kind != K_METHOD);
    assert((((this->info_map).find(key))->second).entry_kind != K_CLASS);

    if(!(((this->info_map).find(key))->second).offset)
        (((this->info_map).find(key))->second).offset = new int(number);
    else
        *((((this->info_map).find(key))->second).offset) = number;
}


/*  ----------------------------------------------------------------------------------------------------    
                                            intermediate_symtable                                                 
    ----------------------------------------------------------------------------------------------------    */

intermediate_symtable::intermediate_symtable() : information(new ids_info()) {

}

ids_info* intermediate_symtable::get_ids_info() {
    return (this->information);
}

std::string intermediate_symtable::get_id_rep(std::string key) {
    assert(((this->scopes).get(key))->get_class() != symtable_element::NOT_FOUND);
    assert((this->information)->id_exists(key));

    return((this->information)->get_id_rep(key));
}

void intermediate_symtable::push_symtable() {
    (this->scopes).push_symtable();
}


void intermediate_symtable::push_symtable(symtable_element& s) {
    (this->scopes).push_symtable(s);
}

void intermediate_symtable::pop_symtable() {
    (this->scopes).pop_symtable();
}

symtable_element* intermediate_symtable::get(std::string key) {
    return((this->scopes).get(key));
}

int* intermediate_symtable::get_offset(std::string key, std::string class_name) {
    int* ret = NULL;
    
    if ((this->information)->id_exists(key) 
        &&
        ((this->information)->get_kind(key) == K_VAR ||
         (this->information)->get_kind(key) == K_OBJECT) 
        &&
        ((this->information)->get_owner_class(key)).compare(class_name) == 0) 
            ret = new int((this->information)->get_offset(key));

    return(ret);
}

t_results intermediate_symtable::new_temp(int offset) {
    return (std::pair<put_results, std::string*>
            (ID_PUT,(this->information)->new_temp(offset, id_type::T_UNDEFINED)));
}

t_results intermediate_symtable::put_var(symtable_element e
                                       , std::string key
                                       , int offset) {
        /*  Besides putting the new variable into the symbols tables stack, it 
            is also necessary to register its new key.                       */
        symtables_stack::put_results res = (this->scopes).put(key, e);

        if (res == symtables_stack::IS_RECURSIVE)
            return(t_results(IS_RECURSIVE, NULL));

        if (res == symtables_stack::ID_EXISTS)
            return(t_results(ID_EXISTS, NULL));

        std::string *rep;
        if (e.get_type() == symtable_element::INTEGER)
            rep = new std::string((this->information)->register_var(key, offset, T_INT));
        else if (e.get_type() == symtable_element::BOOLEAN)
            rep = new std::string((this->information)->register_var(key, offset, T_BOOL));
        else if (e.get_type() == symtable_element::FLOAT)
            rep = new std::string((this->information)->register_var(key, offset, T_FLOAT));
        else if (e.get_type() == symtable_element::STRING)
            rep = new std::string((this->information)->register_var(key, offset, T_STRING));
        else if (e.get_type() == symtable_element::CHAR)
            rep = new std::string((this->information)->register_var(key, offset, T_CHAR));
        else {
            // VOID, ID or UNDEFINED
            rep = new std::string((this->information)->register_var(key, offset, T_UNDEFINED));
        }

        return(t_results(ID_PUT, rep));
}

t_results intermediate_symtable::put_obj(symtable_element& e
                                       , std::string key
                                       , int offset
                                       , std::string type
                                       , std::string address) {
        /*  Besides putting the new object into the symbols tables stack, it 
            is also necessary to register its key.                           */
        symtables_stack::put_results res = (this->scopes).put(key, e);

        if (res == symtables_stack::IS_RECURSIVE)
            return(t_results(IS_RECURSIVE, NULL));

        if (res == symtables_stack::ID_EXISTS)
            return(t_results(ID_EXISTS, NULL));

        std::string* rep = new std::string((this->information)->register_obj(key, offset, type, address));
        
        return(t_results(ID_PUT, rep));
}

t_func_results intermediate_symtable::put_func(symtable_element& e
                                             , std::string key
                                             , unsigned int local_vars
                                             , std::string class_name) {
        /*  Check that the key with which e is going to be registered is
            effectively e's key.                                             */
        assert(key.compare(e.get_key()) == 0);
        /*  Check that e is effectively a function.                          */
        assert(e.get_class() == symtable_element::T_FUNCTION);

        symtables_stack::put_func_results res = 
                                       (this->scopes).put_func(key, e);

        if (res == symtables_stack::NOT_FUNC)
            return(t_func_results(NOT_FUNC, NULL));
        if (res == symtables_stack::FUNC_EXISTS)
            return(t_func_results(FUNC_EXISTS, NULL));
        if (res == symtables_stack::FUNC_ERROR)
            return(t_func_results(FUNC_ERROR, NULL));

        this->func_name = new std::string(key);
        std::string* rep = new std::string((this->information)->register_method(key, local_vars, class_name));

        return(t_func_results(FUNC_PUT, rep));
}

t_param_results intermediate_symtable::put_var_param(symtable_element& e
                                                   , std::string key
                                                   , int offset) {
        assert(key.compare(e.get_key()) == 0);
        assert(e.get_class() == symtable_element::T_VAR);

        symtables_stack::put_param_results res = 
                                        (this->scopes).put_func_param(key, e);

        if(res == symtables_stack::NO_PREV_FUNC)
            return(t_param_results(NO_PREV_FUNC, NULL));
        if(res == symtables_stack::PARAM_TYPE_ERROR)
            return(t_param_results(PARAM_TYPE_ERROR, NULL));

        assert(this->func_name);
        assert((this->information)->id_exists(*(this->func_name)));
        ((this->information)->get_list_params(*(this->func_name))).push_back(key);

        std::string *rep;
        if (e.get_type() == symtable_element::INTEGER)
            rep = new std::string((this->information)->register_var(key, offset, T_INT));
        else if (e.get_type() == symtable_element::BOOLEAN)
            rep = new std::string((this->information)->register_var(key, offset, T_BOOL));
        else if (e.get_type() == symtable_element::FLOAT)
            rep = new std::string((this->information)->register_var(key, offset, T_FLOAT));
        else if (e.get_type() == symtable_element::STRING)
            rep = new std::string((this->information)->register_var(key, offset, T_STRING));
        else if (e.get_type() == symtable_element::CHAR)
            rep = new std::string((this->information)->register_var(key, offset, T_CHAR));
        else {
            // VOID, ID or UNDEFINED
            rep = new std::string((this->information)->register_var(key, offset, T_UNDEFINED));
        }

        return(t_param_results(PARAM_PUT, rep));
}

t_param_results intermediate_symtable::put_obj_param(symtable_element& e
                                                   , std::string key
                                                   , int offset
                                                   , std::string owner
                                                   , std::string address) {
        assert(key.compare(e.get_key()) == 0);
        assert(e.get_class() == symtable_element::T_OBJ);

        symtables_stack::put_param_results res = 
                                        (this->scopes).put_func_param(key, e);

        if(res == symtables_stack::NO_PREV_FUNC)
            return(t_param_results(NO_PREV_FUNC, NULL));

        if(res == symtables_stack::PARAM_TYPE_ERROR)
            return(t_param_results(PARAM_TYPE_ERROR, NULL));

        /*  Check that we are in the middle of a function analysis, and that 
            the name of the function being analyzed exists as an ID registered,
            whose kind is K_METHOD.                                          */
        assert(this->func_name);
        assert((this->information)->id_exists(*(this->func_name)));
        assert((this->information)->get_kind(*(this->func_name)) == K_METHOD);

        /*  Now that being within a function analysis has been checked, we can
            push the new parameter into the function's parameters list.      */
        ((this->information)->get_list_params(*(this->func_name))).push_back(key);

        std::string* rep = new std::string((this->information)->register_obj(key, offset, owner, address));

        return(t_param_results(PARAM_PUT, rep));
}

void intermediate_symtable::set_number_vars(std::string key
                                          , unsigned int number) {
    assert((this->information)->id_exists(key));
    assert((this->information)->get_local_vars(key) <= number);
    (this->information)->set_number_vars(key, number);
}

void intermediate_symtable::finish_func_analysis() {
    (this->scopes).finish_func_analysis();
    delete (this->func_name);
    this->func_name = NULL;
}

t_class_results intermediate_symtable::put_class(symtable_element& e
                                               , std::string key) {
        symtables_stack::put_class_results res = 
                                        (this->scopes).put_class(key, e);

        if(res == symtables_stack::NOT_CLASS)
            return(t_class_results(NOT_CLASS, NULL));

        if(res == symtables_stack::CLASS_EXISTS)
            return(t_class_results(CLASS_EXISTS, NULL));

        if(res == symtables_stack::CLASS_ERROR)
            return(t_class_results(CLASS_ERROR, NULL));

        this->class_name = new std::string(key);
        std::string* rep = new std::string((this->information)->register_class(key, t_attributes()));

        return(t_class_results(CLASS_PUT, rep));
}

t_field_results intermediate_symtable::put_var_field(symtable_element& e
                                                   , std::string key
                                                   , int offset) {
        symtables_stack::put_field_results res = 
                                        (this->scopes).put_class_field(key, e);

        if(res == symtables_stack::FIELD_TYPE_ERROR)
            return(t_field_results(FIELD_TYPE_ERROR, NULL));

        if(res == symtables_stack::NO_PREV_CLASS)
            return(t_field_results(NO_PREV_CLASS, NULL));

        assert(this->class_name);
        assert((this->information)->id_exists(*(this->class_name)));
        int* aux = new int(offset);
        ((this->information)->get_list_attributes(*(this->class_name))).push_front(t_att(key, aux));
        
        std::string *rep;
        if (e.get_type() == symtable_element::INTEGER)
            rep = new std::string((this->information)->register_var(key, offset, T_INT));
        else if (e.get_type() == symtable_element::BOOLEAN)
            rep = new std::string((this->information)->register_var(key, offset, T_BOOL));
        else if (e.get_type() == symtable_element::FLOAT)
            rep = new std::string((this->information)->register_var(key, offset, T_FLOAT));
        else if (e.get_type() == symtable_element::STRING)
            rep = new std::string((this->information)->register_var(key, offset, T_STRING));
        else if (e.get_type() == symtable_element::CHAR)
            rep = new std::string((this->information)->register_var(key, offset, T_CHAR));
        else {
            // VOID, ID or UNDEFINED
            rep = new std::string((this->information)->register_var(key, offset, T_UNDEFINED));
        }

        return(t_field_results(FIELD_PUT, rep));
}

t_field_results intermediate_symtable::put_obj_field(symtable_element& e
                                                   , std::string key
                                                   , int offset
                                                   , std::string class_name
                                                   , std::string address) {
        symtables_stack::put_field_results res = 
                                        (this->scopes).put_class_field(key, e);
        
        if(res == symtables_stack::FIELD_TYPE_ERROR)
            return(t_field_results(FIELD_TYPE_ERROR, NULL));

        if(res == symtables_stack::NO_PREV_CLASS)
            return(t_field_results(NO_PREV_CLASS, NULL));

        assert(this->class_name);
        assert((this->information)->id_exists(*(this->class_name)));
        ((this->information)->get_list_attributes(*(this->class_name))).push_front(t_att(key, new int(offset)));
        std::string* rep = new std::string((this->information)->register_obj(key, offset, class_name, address));

        return(t_field_results(FIELD_PUT, rep));
}

t_field_results intermediate_symtable::put_func_field(symtable_element& e
                                                   , std::string key
                                                   , unsigned int local_vars
                                                   , std::string class_name) {
        symtables_stack::put_field_results res = 
                                        (this->scopes).put_class_field(key, e);

        if(res == symtables_stack::FIELD_TYPE_ERROR)
            return(t_field_results(FIELD_TYPE_ERROR, NULL));

        if(res == symtables_stack::NO_PREV_CLASS)
            return(t_field_results(NO_PREV_CLASS, NULL));

        assert(this->class_name);
        assert((this->information)->id_exists(*(this->class_name)));
        ((this->information)->get_list_attributes(*(this->class_name))).push_front(t_att(key, NULL)); 
        std::string* rep = new std::string((this->information)->register_method(key, local_vars, class_name));

        return(t_field_results(FIELD_PUT, rep));
}

void intermediate_symtable::finish_class_analysis() {
    (this->scopes).finish_class_analysis();
    delete (this->class_name);
    this->class_name = NULL;
}

std::string intermediate_symtable::new_label() {
    std::string ret = 'L' + std::to_string(this->number_label);
    (this->number_label)++;
    return ret;
}

unsigned int intermediate_symtable::size() {
    return((this->scopes).size());
}