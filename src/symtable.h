#ifdef _SYMTABLE_H_
#define _SYMTABLE_H_

#include <cassert>


/*  This class represents the information stored for each identifier.        */
class symtable_element {
public:
    /*  Identifier class: function, variable or array.                       */
    enum id_class { T_FUNCTION
                  , T_CLASS
                  , T_VAR
                  , T_OBJ
                  , T_ARRAY
                  , T_OBJ_ARRAY };
    id_class c_id;
    /*  Identifier type. It always has one.                                  */
    enum id_type { INTEGER
                 , BOOLEAN
                 , FLOAT
                 , VOID
                 , ID };
    /*  Basic type variable.                            */
    symtable_element(id_type t);
    /*  Basic type array.                               */
    symtable_element(id_type t, unsigned int d);
    /*  Object.                                         */
    symtable_element(std::string* s);  
    /*  Objects array.                                  */
    symtable_element(std::string* s, unsigned int d);
    /*  Function.                                       */
    symtable_element(id_type t, std::list<std::string>* f); 
    /*  Class.                                          */
    symtable_element(std::string* s, std::list<std::string>* f);

    /*  Getters.                                        */
    id_class get_class ();
    id_type get_type ();
    std::string* get_class_type ();
    unsigned int get_dimension ();
    std::list<std::string>* get_func_params ();
    std::list<std::string>* get_class_fields ();
private:
    id_type t_id;
    /*  Identifier type when it is an object.                                */
    std::string* class_type;
    /*  Identifier dimension. Only for arrays.                               */
    unsigned int dim = 0;
    /*  List of formal arguments (keys to the hashtable) if the identifier 
        is a function. Note that every element in the list should be of a 
        basic type.                                                          */
    std::list<std::string>* func_params = NULL;
    /*  List of attributes and methods (keys to the hastable, actually) if 
        the identifier is a class. Every element in the list should be 
        avalid entry in some symbols table. Therefore, recursion can be 
        done. This information will be useful when checking method calling.  */
    std::list<std::string>* class_fields = NULL;
};


/*  This class represents the symbol table associated 
    with the current class/block/function.              */
class symtable { 
private:
    std::unordered_map<std::string, symtable_element> hashtable;

public:
    bool elem_exists (std::string key);

    symtable_element get(std::string key);

    void put (std::string key, symtable_element value);
} ;


/*  This class represents the chain of symbols table
    that conform the current scope.                      */
class scopes_list {
private:
    symtable table;
    scopes_list *prev;

public:
    scopes_list() : prev(NULL) { }
    
    symtable_element* get (std::string key);

    void put(std::string key, symtable_element value);

    ~scopes_list();
};

#endif _SYMTABLE_H_