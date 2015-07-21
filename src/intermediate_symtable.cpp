#include "intermediate_symtable.h"

/*  ----------------------------------------------------------------------------------------------------    
                                            ids_info                                                 
    ----------------------------------------------------------------------------------------------------    */

std::string get_next_internal(std::string key) {
    std::string internal_rep_key;
    if ((this->internal).find(key) != (this->internal).end()) {
        return (key + to_string((this->internal).find(key) + 1));
    }
    else {
        (this->internal).insert(std::pair<std::string, unsigned int>(key, 0));
        return(key + "0");   /* The internal representation ranges
                                            key0, key1, key2, ...            */
    }
}

std::string ids_info::register_var(std::string key, unsigned int offset) {
    std::string internal_key = get_next_internal(key);

    entry_info information;
    information.entry_kind = ids_info::VAR;
    information.data.var_info.offset = offset;

    (this->info_map).insert(std::pair<std::string, t_info>(key, information));

    return internal_key;
}

std::string ids_info::register_obj(std::string key, unsigned int offset, std::string owner) {
    std::string internal_key = get_next_internal(key);

    entry_info information;
    information.entry_kind = ids_info::OBJECT;
    information.data.obj_info.offset = offset;
    information.data.obj_info.owner = owner;

    (this->info_map).insert(std::pair<std::string, t_info>(key, information));

    return internal_key;
}

std::string ids_info::register_method(std::string key, unsigned int locals, std::string owner) {
    std::string internal_key = get_next_internal(key);

    entry_info information;
    information.entry_kind = ids_info::K_METHOD;
    information.data.method_info.local_vars = locals;
    information.data.method_info.owner = owner;

    (this->info_map).insert(std::pair<std::string, t_info>(key, information));

    return internal_key;
}

std::string ids_info::register_class(std::string key, std::list<std::string> attributes) {
    std::string internal_key = get_next_internal(key);

    entry_info information;
    information.entry_kind = ids_info::K_CLASS;
    information.data.class_info.l_atts = attributes;

    (this->info_map).insert(std::pair<std::string, t_info>(key, information));

    return internal_key;
}

void unregister(std::string key) {
    assert((this->internal).find(key) != (this->internal).end());
    assert(((this->internal).find(key))->second != 0);

    ((this->internal).find(key))->second = ((this->internal).find(key))->second - 1;
}

bool ids_info::id_exists(std::string key) {
    return ((this->internal).find(key) != (this->internal).end());
}

std::string ids_info::get_id_rep(std::string key) {
    assert((this->info_map).find(key) != (this->info_map).end());
    assert((this->internal).find(key) != (this->internal).end());

    return (key + to_string(((this->internal).find(key))->second));
}

id_kind ids_info::get_kind(std::string key) {
    assert((this->info_map).find(key) != (this->info_map).end());

    return ((((this->info_map).find(key))->second).entry_kind);
}

unsigned int ids_info::get_offset(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_VAR
        || (((this->info_map).find(key))->second).entry_kind == ids_info::K_OBJECT);

    if ((((this->info_map).find(key))->second).entry_kind == ids_info::K_VAR)
        return (((this->info_map).find(key))->second).data.var_info.offset;
    else
        return (((this->info_map).find(key))->second).data.obj_info.offset;
}

unsigned int ids_info::get_local_vars(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_METHOD);

    return ((((this->info_map).find(key))->second).data.method_info.local_vars);
}

std::string ids_info::get_owner_class(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_METHOD
        || (((this->info_map).find(key))->second).entry_kind == ids_info::K_OBJECT);

    if ((((this->info_map).find(key))->second).entry_kind == ids_info::K_METHOD)
        return (((this->info_map).find(key))->second).data.method_info.owner;
    else
        return (((this->info_map).find(key))->second).data.obj_info.owner;
}

std::list<std::string> ids_info::get_list_attributes(std::string key) {
    assert((((this->info_map).find(key))->second).entry_kind == ids_info::K_CLASS);

    return ((((this->info_map).find(key))->second).data.class_info.l_atts);
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
    assert(((this->scopes).get(key)).get_class() != symtable_element::NOT_FOUND);
    assert((this->information)->id_exists(key));

    return((this->information).get_id_rep(key));
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

pair<symtables_stack::put_results, std::string*> 
    intermediate_symtable::put_var(symtable_element e, std::string key, unsigned int offset) {
        /*  Besides putting the new variable into the symbols tables stack, it 
            is also necessary to register its new key.                       */
        symtables_stack::put_results res = (this->scopes).put(key, e);
        if (res != symtables_stack::ID_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_var(key, offset));
        
        return(make_pair(res, rep));
}

pair<symtables_stack::put_func_results, std::string>
    intermediate_symtable::put_obj(symtable_element& e, std::string key, unsigned int offset, std::string type) {
        /*  Besides putting the new object into the symbols tables stack, it 
            is also necessary to register its key.                           */
        symtables_stack::put_results res = (this->scopes).put(key, e);
        if (res != symtables_stack::ID_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_obj(key, offset));
        
        return(make_pair(res, rep));
}

pair<symtables_stack::put_func_results, std::string>
    put_func(symtable_element& e, std::string key, unsigned int local_vars, std::string class_name) {
        symtables_stack::put_func_results res = (this->scopes).put(key, e);
        if (res != symtables_stack::FUNC_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_method(key, local_vars, class_name));

        return(make_pair(res, rep));
}

pair<symtables_stack::put_param_results, std::string> 
    put_var_param(symtable_element& e, std::string key, unsigned int offset) {
        assert(key.compare(e.get_key()) == 0);
        assert(e.get_class() == symtable_element::T_VAR);

        symtables_stack::put_param_results res = (this->scopes).put(key, e);
        if(res != symtables_stack::PARAM_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_var(key, e));

        return(make_pair(res, rep));
}

pair<symtables_stack::put_param_results, std::string> 
    put_obj_param(symtable_element&, std::string, unsigned int, std::string) {
        assert(key.compare(e.get_key()) == 0);
        assert(e.get_class() == symtable_element::T_OBJ);

        symtables_stack::put_param_results res = (this->scopes).put_func(key, e);
        if(res != symtables_stack::PARAM_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_obj(key, e));

        return(make_pair(res, rep));
}

void intermediate_symtable::finish_func_analysis() {
    (this->scopes).finish_func_analysis();
}

pair<symtables_stack::put_class_results, std::string> 
    put_class(symtable_element& e, std::string key, std::list<std::string> l_atts) {
        symtables_stack::put_class_results res = (this->scopes).put_class(key, e);
        if(res != symtables_stack::CLASS_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_class(key, l_atts));

        return(make_pair(res, rep));
}

pair<symtables_stack::put_field_results, std::string> 
    put_var_field(symtable_element& e, std::string key, unsigned int offset) {
        symtables_stack::put_field_results res = (this->scopes).put_class_field(key, e);
        if(res != symtables_stack::FIELD_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_var(key, offset));

        return(make_pair(res, rep));
}

pair<symtables_stack::put_field_results, std::string> 
    put_obj_field(symtable_element& e, std::string key, unsigned int offset, std::string class_name) {
        symtables_stack::put_field_results res = (this->scopes).put_class_field(key, e);
        if(res != symtables_stack::FIELD_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_obj(key, offset, class_name));

        return(make_pair(res, rep));
}

pair<symtables_stack::put_field_results, std::string> 
    put_func_field(symtable_element& e, std::string key, unsigned int local_vars, std::string class_name) {
        symtables_stack::put_field_results res = (this->scopes).put_class_field(key, e);
        if(res != symtables_stack::FIELD_PUT)
            return(make_pair(res, NULL));

        std::string* rep = new std::string((this->information).register_method(key, local_vars, class_name));

        return(make_pair(res, rep));
}

void intermediate_symtable::finish_class_analysis() {
    (this->scopes).finish_class_analysis();
}

unsigned int intermediate_symtable::size() {
    return((this->scopes).size());
}