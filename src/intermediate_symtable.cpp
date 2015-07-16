#include "intermediate_symtable.h"

/*  ----------------------------------------------------------------------------------------------------    
                                            ids_info                                                 
    ----------------------------------------------------------------------------------------------------    */

ids_info::ids_info(void) {

}

bool ids_info::id_exists(std::string key) {

}

id_kind ids_info::get_kind(std::string key) {

}

unsigned int ids_info::get_offset(std::string key) {

}

unsigned int ids_info::get_local_vars(std::string key) {

}

std::string ids_info::get_owner_class(std::string key) {

}

std::list<std::string> ids_info::get_list_attributes(std::string key) {

}


/*  ----------------------------------------------------------------------------------------------------    
                                            intermediate_symtable                                                 
    ----------------------------------------------------------------------------------------------------    */

intermediate_symtable::intermediate_symtable() {

}

ids_info* intermediate_symtable::get_ids_info() {

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