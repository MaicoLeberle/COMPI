#include "intermediate_symtable.h"


/*  ----------------------------------------------------------------------------------------------------    
                                            ids_info                                                 
    ----------------------------------------------------------------------------------------------------    */

std::string ids_info::get_next_internal(std::string key) {
    /*  The internal representations range key-0, key-1, key-2, ...          */
    if ((this->internal).find(key) == (this->internal).end()) 
        (this->internal).insert(std::pair<std::string, unsigned int>(key, 0));
    

    return ((key + '-') + std::to_string(((this->internal).find(key))->second));
}

std::string* ids_info::new_temp(unsigned int o) {
    std::string* ret = new std::string("t-" + std::to_string(this->temp_number));
    assert((this->info_map).find(*ret) == (this->info_map).end());

    entry_info information;

    information.entry_kind = ids_info::K_TEMP;
    information.offset = new unsigned int(o);

    (this->info_map).insert(std::pair<std::string, entry_info>(*ret, information));
    
    (this->temp_number)++;
    return ret;
}

std::string ids_info::register_var(std::string key, unsigned int offset) {
    entry_info information;
    
    information.entry_kind = ids_info::K_VAR;
    information.offset =  new unsigned int(offset);

    std::string internal_key = this->get_next_internal(key);
    ++((this->internal).find(key))->second;
    information.rep = internal_key;

    (this->info_map).insert(std::pair<std::string, entry_info>(key, information));

    return internal_key;
}

std::string ids_info::register_obj(std::string key, unsigned int offset, std::string owner, std::string address) {
    entry_info information;
    
    information.entry_kind = ids_info::K_OBJECT;
    information.offset = new unsigned int(offset);
    information.owner = new std::string(owner);
    information.begin_address = new std::string(address);

    std::string internal_key = this->get_next_internal(key);
    ++((this->internal).find(key))->second;
    information.rep = internal_key;

    (this->info_map).insert(std::pair<std::string, entry_info>(key, information));

    return internal_key;
}

std::string ids_info::register_method(std::string key, unsigned int locals, std::string owner) {
    entry_info information;

    information.entry_kind = ids_info::K_METHOD;
    information.local_vars = new unsigned int(locals);
    information.owner = new std::string(owner);

    std::string internal_key = key + "::" + owner;
    information.rep = internal_key;

    (this->info_map).insert(std::pair<std::string, entry_info>(key, information));

    return internal_key;
}

std::string ids_info::register_class(std::string key, std::list<std::string> attributes) {
    entry_info information;
    
    information.entry_kind = ids_info::K_CLASS;
    information.l_atts = new std::list<std::string>(attributes);
    
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
    return ((this->internal).find(key) != (this->internal).end());
}

std::string ids_info::get_id_rep(std::string key) {
    assert((this->info_map).find(key) != (this->info_map).end());
    
    if ((((this->info_map).find(key))->second).entry_kind == ids_info::K_METHOD) {
        return (key + "::" + *((((this->info_map).find(key))->second).owner));
    } else {
        assert((this->internal).find(key) != (this->internal).end());
        return (key + '-' + std::to_string(((this->internal).find(key))->second - 1));
    }
}

ids_info::id_kind ids_info::get_kind(std::string key) {
    assert((this->info_map).find(key) != (this->info_map).end());

    return ((((this->info_map).find(key))->second).entry_kind);
}

unsigned int ids_info::get_offset(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_VAR
        || (((this->info_map).find(key))->second).entry_kind == ids_info::K_OBJECT
        || (((this->info_map).find(key))->second).entry_kind == ids_info::K_TEMP);

    return *((((this->info_map).find(key))->second).offset);
}

unsigned int ids_info::get_local_vars(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_METHOD);

    return *((((this->info_map).find(key))->second).local_vars);
}

std::string ids_info::get_owner_class(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_METHOD
        || (((this->info_map).find(key))->second).entry_kind == ids_info::K_OBJECT);
    
    return *((((this->info_map).find(key))->second).owner);
}

std::list<std::string> ids_info::get_list_attributes(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_CLASS);

    return *((((this->info_map).find(key))->second).l_atts);
}

void ids_info::set_number_vars(std::string key, unsigned int number) {
    assert((this->info_map).find(key) != (this->info_map).end());
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_METHOD);

    if(!(((this->info_map).find(key))->second).local_vars)
        (((this->info_map).find(key))->second).local_vars = new unsigned int(number);
    else
        *((((this->info_map).find(key))->second).local_vars) = number;
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

symtable_element* intermediate_symtable::get (std::string key) {
    return((this->scopes).get(key));
}

std::pair<intermediate_symtable::put_results, std::string*> intermediate_symtable::new_temp(unsigned int offset) {
    return (std::pair<intermediate_symtable::put_results, std::string*>
            (intermediate_symtable::ID_PUT,(this->information)->new_temp(offset)) );
}

std::pair<intermediate_symtable::put_results, std::string*> 
    intermediate_symtable::put_var(symtable_element e, std::string key, unsigned int offset) {
        /*  Besides putting the new variable into the symbols tables stack, it 
            is also necessary to register its new key.                       */
        symtables_stack::put_results res = (this->scopes).put(key, e);

        if (res == symtables_stack::IS_RECURSIVE)
            return(std::pair<intermediate_symtable::put_results, std::string*>(intermediate_symtable::IS_RECURSIVE, NULL));

        if (res == symtables_stack::ID_EXISTS)
            return(std::pair<intermediate_symtable::put_results, std::string*>(intermediate_symtable::ID_EXISTS, NULL));

        std::string* rep = new std::string((this->information)->register_var(key, offset));
        
        return(std::pair<intermediate_symtable::put_results, std::string*>(intermediate_symtable::ID_PUT, rep));
}

std::pair<intermediate_symtable::put_results, std::string*>
    intermediate_symtable::put_obj(symtable_element& e, std::string key, unsigned int offset, std::string type, std::string address) {
        /*  Besides putting the new object into the symbols tables stack, it 
            is also necessary to register its key.                           */
        symtables_stack::put_results res = (this->scopes).put(key, e);

        if (res == symtables_stack::IS_RECURSIVE)
            return(std::pair<intermediate_symtable::put_results, std::string*>(intermediate_symtable::IS_RECURSIVE, NULL));

        if (res == symtables_stack::ID_EXISTS)
            return(std::pair<intermediate_symtable::put_results, std::string*>(intermediate_symtable::ID_EXISTS, NULL));

        std::string* rep = new std::string((this->information)->register_obj(key, offset, type, address));
        
        return(std::pair<intermediate_symtable::put_results, std::string*>(intermediate_symtable::ID_PUT, rep));
}

std::pair<intermediate_symtable::put_func_results, std::string*>
    intermediate_symtable::put_func(symtable_element& e, std::string key, unsigned int local_vars, std::string class_name) {
        symtables_stack::put_func_results res = (this->scopes).put_func(key, e);

        if (res == symtables_stack::NOT_FUNC)
            return(std::pair<intermediate_symtable::put_func_results, std::string*>(intermediate_symtable::NOT_FUNC, NULL));
        if (res == symtables_stack::FUNC_EXISTS)
            return(std::pair<intermediate_symtable::put_func_results, std::string*>(intermediate_symtable::FUNC_EXISTS, NULL));
        if (res == symtables_stack::FUNC_ERROR)
            return(std::pair<intermediate_symtable::put_func_results, std::string*>(intermediate_symtable::FUNC_ERROR, NULL));

        std::string* rep = new std::string((this->information)->register_method(key, local_vars, class_name));

        return(std::pair<intermediate_symtable::put_func_results, std::string*>(intermediate_symtable::FUNC_PUT, rep));
}

std::pair<intermediate_symtable::put_param_results, std::string*> 
    intermediate_symtable::put_var_param(symtable_element& e, std::string key, unsigned int offset) {
        assert(key.compare(e.get_key()) == 0);
        assert(e.get_class() == symtable_element::T_VAR);

        symtables_stack::put_param_results res = (this->scopes).put_func_param(key, e);

        if(res == symtables_stack::NO_PREV_FUNC)
            return(std::pair<intermediate_symtable::put_param_results, std::string*>(intermediate_symtable::NO_PREV_FUNC, NULL));
        if(res == symtables_stack::PARAM_TYPE_ERROR)
            return(std::pair<intermediate_symtable::put_param_results, std::string*>(intermediate_symtable::PARAM_TYPE_ERROR, NULL));

        std::string* rep = new std::string((this->information)->register_var(key, offset));

        return(std::pair<intermediate_symtable::put_param_results, std::string*>(intermediate_symtable::PARAM_PUT, rep));
}

std::pair<intermediate_symtable::put_param_results, std::string*> 
    intermediate_symtable::put_obj_param(symtable_element& e, std::string key, unsigned int offset, std::string owner, std::string address) {
        assert(key.compare(e.get_key()) == 0);
        assert(e.get_class() == symtable_element::T_OBJ);

        symtables_stack::put_param_results res = (this->scopes).put_func_param(key, e);

        if(res == symtables_stack::NO_PREV_FUNC)
            return(std::pair<intermediate_symtable::put_param_results, std::string*>(intermediate_symtable::NO_PREV_FUNC, NULL));

        if(res == symtables_stack::PARAM_TYPE_ERROR)
            return(std::pair<intermediate_symtable::put_param_results, std::string*>(intermediate_symtable::PARAM_TYPE_ERROR, NULL));

        std::string* rep = new std::string((this->information)->register_obj(key, offset, owner, address));

        return(std::pair<intermediate_symtable::put_param_results, std::string*>(intermediate_symtable::PARAM_PUT, rep));
}

void intermediate_symtable::set_number_vars(std::string key, unsigned int number) {
    (this->information)->set_number_vars(key, number);
}

void intermediate_symtable::finish_func_analysis() {
    (this->scopes).finish_func_analysis();
}

std::pair<intermediate_symtable::put_class_results, std::string*> 
    intermediate_symtable::put_class(symtable_element& e, std::string key, std::list<std::string> l_atts) {
        symtables_stack::put_class_results res = (this->scopes).put_class(key, e);

        if(res == symtables_stack::NOT_CLASS)
            return(std::pair<intermediate_symtable::put_class_results, std::string*> (intermediate_symtable::NOT_CLASS, NULL));

        if(res == symtables_stack::CLASS_EXISTS)
            return(std::pair<intermediate_symtable::put_class_results, std::string*> (intermediate_symtable::CLASS_EXISTS, NULL));

        if(res == symtables_stack::CLASS_ERROR)
            return(std::pair<intermediate_symtable::put_class_results, std::string*> (intermediate_symtable::CLASS_ERROR, NULL));

        std::string* rep = new std::string((this->information)->register_class(key, l_atts));

        return(std::pair<intermediate_symtable::put_class_results, std::string*> (intermediate_symtable::CLASS_PUT, rep));
}

std::pair<intermediate_symtable::put_field_results, std::string*> 
    intermediate_symtable::put_var_field(symtable_element& e, std::string key, unsigned int offset) {
        symtables_stack::put_field_results res = (this->scopes).put_class_field(key, e);

        if(res == symtables_stack::FIELD_TYPE_ERROR)
            return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::FIELD_TYPE_ERROR, NULL));

        if(res == symtables_stack::NO_PREV_CLASS)
            return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::NO_PREV_CLASS, NULL));

        std::string* rep = new std::string((this->information)->register_var(key, offset));

        return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::FIELD_PUT, rep));
}

std::pair<intermediate_symtable::put_field_results, std::string*> 
    intermediate_symtable::put_obj_field(symtable_element& e, std::string key, unsigned int offset, std::string class_name, std::string address) {
        symtables_stack::put_field_results res = (this->scopes).put_class_field(key, e);
        
        if(res == symtables_stack::FIELD_TYPE_ERROR)
            return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::FIELD_TYPE_ERROR, NULL));

        if(res == symtables_stack::NO_PREV_CLASS)
            return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::NO_PREV_CLASS, NULL));

        std::string* rep = new std::string((this->information)->register_obj(key, offset, class_name, address));

        return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::FIELD_PUT, rep));
}

std::pair<intermediate_symtable::put_field_results, std::string*> 
    intermediate_symtable::put_func_field(symtable_element& e, std::string key, unsigned int local_vars, std::string class_name) {
        symtables_stack::put_field_results res = (this->scopes).put_class_field(key, e);

        if(res == symtables_stack::FIELD_TYPE_ERROR)
            return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::FIELD_TYPE_ERROR, NULL));

        if(res == symtables_stack::NO_PREV_CLASS)
            return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::NO_PREV_CLASS, NULL));

        std::string* rep = new std::string((this->information)->register_method(key, local_vars, class_name));

        return(std::pair<intermediate_symtable::put_field_results, std::string*> (intermediate_symtable::FIELD_PUT, rep));
}

void intermediate_symtable::finish_class_analysis() {
    (this->scopes).finish_class_analysis();
}

std::string intermediate_symtable::new_label() {
    std::string ret = 'L' + std::to_string(this->number_label);
    (this->number_label)++;
    return ret;
}

unsigned int intermediate_symtable::size() {
    return((this->scopes).size());
}