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
        Other attributes will be inferred from this information.             */

    /*  To be returned when an entry is not found in a symbols table.
        Precondition: the argument is NOT_FOUND                              */
    symtable_element(id_class);

    /*  Basic type variable.                                                 */
    symtable_element(id_type);

    /*  Basic type array.                                                    */
    symtable_element(id_type, unsigned int);

    /*  Object.                                                              */
    symtable_element(std::string*); 

    /*  Objects array.                                                       */
    symtable_element(std::string*, unsigned int);

    /*  Function.                                                            */
    symtable_element(id_type, std::list<std::string>*); 

    /*  Class.                                                               */
    symtable_element(std::string*, std::list<std::string>*);


    /*  Getters.                                                             */

    /*  Returns the corresponding enum id_class value.                       */
    id_class get_class ();

    /*  Returns the corresponding enum id_type value.                        */
    id_type get_type ();

    /*  Precondition: get_class() == (T_OBJ || T_OBJ_ARRAY || T_CLASS).      */
    std::string* get_class_type ();

    /*  Precondition: get_class() = (T_ARRAY || T_OBJ_ARRAY).                */
    unsigned int get_dimension ();

    /*  Returns list of function formal parameters identifiers.
        Precondition: get_class() == T_FUNCTION.                             */
    std::list<std::string>* get_func_params ();

    /*  Returns list of class fields and methods identifiers.
        Precondition: get_class() == T_CLASS.                                */
    std::list<std::string>* get_class_fields ();

private:
    id_class c_id;

    id_type t_id;

    /*  Identifier type when it is an object.                                */
    std::string* class_type;

    /*  Identifier dimension. Only for arrays.                               */
    unsigned int dim = 0;

    /*  List of formal arguments identifiers (meant for when the symbols 
        table element is a function). Note that every element in the list 
        should be of a basic type.                                           */
    std::list<std::string>* func_params = NULL;

    /*  List of attributes and methods identifiers (meant for when the 
        symbols table element is a class).                                   */
    std::list<std::string>* class_fields = NULL;
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
    symtable table;
    scopes_list *prev;

public:
    /*  This constructor is meant only for the empty stack.                  */
    scopes_list(void);

    /*  The scopes_list* parameter should represent the previous top of the
        symbol tables stack.                                                 */
    scopes_list(scopes_list*);
    
    symtable_element get (std::string);

    void put(std::string, symtable_element);

    ~scopes_list();
};

#endif