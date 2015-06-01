#ifdef _SYMTABLE_H_
#define _SYMTABLE_H_

#include <cassert>


class symtable_element{
private:
    /*  Identifier class: function, variable or array.  */
    enum id_class { T_FUNCTION, T_VAR, T_ARRAY };
    id_class c_id;
    /*  Identifier type. It always has one.             */
    enum id_type { INTEGER, BOOLEAN, FLOAT, VOID, ID };
    id_type t_id;
    /*  Identifier dimension. Only for arrays.          */
    unsigned int dim = 0;
    /*  List of formal arguments if the identifier is 
        a function. Note that every element in the list
        must be of a basic type.                        */
    std::list<symtable_element> func_params = NULL;

public:

    symtable_element(id_type t) : c_id(T_VAR), t_id(t) { }
    symtable_element(id_type t, std::list<symtable_element> f) : 
        c_id(T_FUNCTION), t_id(t), func_params(f) { }
    symtable_element(id_type t, unsigned int d) : 
        c_id(T_ARRAY), t_id(t), dim(d) { }
};

class symtable { 
private:
    std::unordered_map<std::string, symtable_element> hashtable;

public:
    bool elem_exists (std::string key)) {
        return (hashtable.find(key) != hashtable.end());
    }

    symtable_element* get(std::string key) {
        assert(elem_exists(key));
        return (hashtable[key]);
    }

    void put (std::string key, symtable_element value) {
        hashtable[key] = value;
    }
} ;

class scopes_list {
private:
    symtable table;
    scopes_list *prev;

public:
    scopes_list() : prev(NULL) { }
    
    symtable_element* get (std::string key) {
        for(scopes_list *it = this; this != NULL; this = this->prev) {
            if((it->table).elem_exists(key))
                return ((it->table).get(key));
        }
        return NULL;
    }

    void put(std::string key, symtable_element value) {
        table.put(key, value);
    }
};

#endif _SYMTABLE_H_