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

intermediate_symtable::intermediate_symtable() {

}

ids_info* intermediate_symtable::get_ids_info() {

}

std::string intermediate_symtable::get_id_rep(std::string key) {

}

void intermediate_symtable::push_symtable() {

}


void intermediate_symtable::push_symtable(symtable_element& s) {

}

void intermediate_symtable::pop_symtable() {

}

symtable_element* intermediate_symtable::get (std::string key) {

}

symtables_stack::put_results intermediate_symtable::put(std::string key, symtable_element value) {

}

symtables_stack::put_func_results intermediate_symtable::put_func(std::string key, symtable_element& value) {

}

symtables_stack::put_param_results intermediate_symtable::put_func_param(std::string key, symtable_element& value) {

}

void intermediate_symtable::finish_func_analysis() {

}

symtables_stack::put_class_results intermediate_symtable::put_class(std::string key, symtable_element& value) {

}

symtables_stack::put_field_results intermediate_symtable::put_class_field(std::string key, symtable_element& value) {

}

void intermediate_symtable::finish_class_analysis() {

}

unsigned int intermediate_symtable::size() {

}