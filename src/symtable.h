#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#include <cassert>
#include <map>
#include <string>
#include <list>


/*  This class represents the information stored for each identifier.        */
class symtable_element {
public:
    /*  Identifier class. It will tell what kind of identifier it is.        */
    enum id_class { T_FUNCTION
                  , T_CLASS
                  , T_VAR
                  , T_OBJ
                  , T_ARRAY
                  , T_OBJ_ARRAY
                  , NOT_FOUND };
    /*  Identifier type.   It will represent the type of the identifier.
        Objects and array of objects will have the type of the class they 
        belong to.                                                           */
    enum id_type { INTEGER
                 , BOOLEAN
                 , FLOAT
                 , VOID
                 , ID };

    /*  For the following constructors, only necessary information is given. 
        Other attributes will be inferred from this information.
        The first parameter in each one of them, except for the first one,
        is the key in the symbol table associated with this element.         */

    /*  To be returned when an entry is not found in a symbols table.
        Precondition: the argument is NOT_FOUND                              */
    symtable_element(id_class);

    /*  Basic type variable.                                                 */
    symtable_element(std::string, id_type);

    /*  Basic type array.                                                    */
    symtable_element(std::string, id_type, unsigned int);

    /*  Object.                                                              */
    symtable_element(std::string, std::string); 

    /*  Objects array.                                                       */
    symtable_element(std::string, std::string, unsigned int);

    /*  Function.                                                            */
    symtable_element(std::string, id_type, std::list<symtable_element>*); 

    /*  Class.                                                               */
    symtable_element(std::string, std::list<symtable_element>*);


    /*  Getters.                                                             */

    std::string get_key(void);

    /*  Returns the corresponding enum id_class value.                       */
    id_class get_class (void);

    /*  Returns the corresponding enum id_type value.                        */
    id_type get_type (void);

    /*  Precondition: get_class() == (T_OBJ || T_OBJ_ARRAY || T_CLASS).      */
    std::string get_class_type (void);

    /*  Precondition: get_class() = (T_ARRAY || T_OBJ_ARRAY).                */
    unsigned int get_dimension (void);

    /*  Returns list of function formal parameters identifiers.
        Precondition: get_class() == T_FUNCTION.                             */
    std::list<symtable_element>* get_func_params (void);

    /*  Returns list of class fields and methods identifiers.
        Precondition: get_class() == T_CLASS.                                */
    std::list<symtable_element>* get_class_fields (void);

private:
    /*  key is meant to store the same key that appears in a symbols table
        associated with this symtable_element. key will be needed, e.g., when
        inserting a function or class in a symbols table: every function 
        parameter or class field or method should be inserted in the symbols
        table next to their corresponding identifiers.                       */
    std::string key;

    id_class c_id;

    id_type t_id;

    /*  Identifier type when it is an object, an object array or a class.    */
    std::string class_type;

    /*  Identifier dimension. Only for arrays.                               */
    unsigned int dim = 0;

    /*  List of formal arguments identifiers (meant for when the symbols 
        table element is a function). Note that every element in the list 
        should be of a basic type.                                           */
    std::list<symtable_element>* func_params = NULL;

    /*  List of attributes and methods identifiers (meant for when the 
        symbols table element is a class).                                   */
    std::list<symtable_element>* class_fields = NULL;
};


/*  This class represents the symbol table associated 
    with the current class/block/function.              */
class symtable { 
public:
    bool elem_exists (std::string);

    /*  Precondition: the formal argument given to get should already exist
        in the symtable; i.e., elem_exists(...) == TRUE.                     */
    symtable_element get(std::string);

    void put (std::string, symtable_element);

private:
    std::map<std::string, symtable_element> hashtable;
} ;


/*  This class represents the stack of symbol tables that conform the current
    scope.                                                                   */
class scopes_list {
private:
    symtable* table;
    scopes_list *prev;

public:
    /*  This constructor is meant only for the empty stack.                  */
    scopes_list(void);

    /*  Create a new symbols table related to a block.                       */
    void push_symtable(void);

    /*  Create a new symbols table related to a function or class.
        PRECONDITION: get_class_type() == (T_CLASS || T_FUNCTION)            */
    void push_symtable(symtable_element);

    void pop_symtable(void);
    
    symtable_element get (std::string);

    void put(std::string, symtable_element);

    ~scopes_list(void);
};

#endif