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

    /*  Copy constructor.                                                    */
    symtable_element(symtable_element*);


    /*  Getters.                                                             */

    std::string get_key(void);

    /*  Returns the corresponding enum id_class value.                       */
    id_class get_class (void);

    /*  Returns the corresponding enum id_type value.                        */
    id_type get_type (void);

    /*  Returns the class_type value.                                        */ 
    std::string get_class_type (void);

    /*  Returns the dimension. It will be 0 if it is not an array.           */
    unsigned int get_dimension (void);

    /*  Returns list of function formal parameters identifiers.              */
    std::list<symtable_element>* get_func_params (void);

    /*  Appends (to the end of the list) a new symtable_element to the list 
        of function parameters.
        Precondition: get_class() == T_FUNCTION.                             */
    void put_func_param(symtable_element);

    /*  Returns list of class fields and methods identifiers.                */
    std::list<symtable_element>* get_class_fields (void);

    /*  Appends (to the end of the list) a new symtable_element to the list
        of class fields.
        Precondition: get_class() == T_CLASS.                                */
    void put_class_field(symtable_element);

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
    /*  Constructor for a block symtable.                                    */
    symtable(void);

    /*  Constructor for class or function symtable. First parameter should be 
        the identifier of the class or function. Second parameter should be
        TRUE if the current symtable represents a class, and FALSE if it 
        represents a function.                              
        Precondition: get_class_type() == (symtable_element::T_FUNCTION || 
        symtable_element::T_CLASS)                                           */
    symtable(std::string, bool);

    bool elem_exists (std::string);

    /*  Precondition: the formal argument given to get should already exist
        in the symtable; i.e., elem_exists(...) == TRUE.                     */
    symtable_element* get_elem(std::string);

    /*  Precondition: the constructor used was symtable(std::string).        */
    std::string get_id(void);

    /*  Returns TRUE if the identifier has not yet been inserted in this 
        current symtable. Otherwise, returns FALSE.                          */
    bool id_exists(std::string);

    /*  Returns TRUE if the symtable_element parameter is of the same type of
        the class whose symtable is the current one. Otherwise, returns FALSE.
        Precondition: the constructor used was symtable(std::string, 
        symtable_element::id_class).                                         */
    bool is_recursive(symtable_element);

    /*  Returns TRUE if id_exists(...) == FALSE && 
        is_recursive(...) == FALSE. Otherwise, returns false.                */
    bool put (std::string, symtable_element); 

private:
    /*  id will be useful when checking the return type of a function, or 
        when checking the imposibility of defining an object of some class 
        type in the definition of that class.                                */
    std::string* id;
    bool class_or_function;
    std::map<std::string, symtable_element> hashtable;
} ;


/*  This class represents the stack of symbol tables that conform the current
    scope.                                                                   */
class symtables_stack {
private:
    //symtables_stack *prev;
    //unsigned int length;
    std::list<symtable*> stack;
    symtable_element* last_elem;

public:
    /*  This enumeration will be used as a way to inform the symtables_stack 
        user the result of calling put(...).                                 */
    enum put_results { IS_RECURSIVE
                     , ID_EXISTS
                     , NOT_FUNCTION
                     , NOT_CLASS 
                     , INSERTED };

    /*  This constructor is meant only for the empty stack.                  */
    symtables_stack(void);

    /*  Create a new symbols table related to a block.                       */
    void push_symtable(void);

    /*  Create a new symbols table related to a function or class.
        PRECONDITION: get_class_type() == (T_CLASS || T_FUNCTION)            */
    void push_symtable(symtable_element);

    /*  Precondition: There have been more calls to push_symtable(...) than
        to pop_symtable(); i.e., there is still another symbols table to pop.*/
    void pop_symtable(void);
    
    /*  Precondition: There have been more calls to push_symtable(...) than
        to pop_symtable(); i.e., there is still another symbols table to get 
        an element from.                                                     */ 
    symtable_element* get (std::string);

    /*  Inserts a new element into the symbols tables stack. This element is 
        later remembered as the lastly inserted symtable_element.
        Precondition: There have been more calls to push_symtable(...) than
        to pop_symtable(); i.e., there is still another symbols table to put 
        an element to.                                                       */
    symtables_stack::put_results put(std::string, const symtable_element&);

    /*  Inserts a new parameter to the lastly inserted symtable_element.
        Precondition: The lastly inserted symtable_element in the symbols 
        tables stack is a function.The element's key should be the same as 
        this function's first parameter.                                     */
    symtables_stack::put_results put_func_param(symtable_element);

    /*  Inserts a new class field to the lastly inserted symtable_element.
        Precondition: The lastly inserted symtable_element in the symbols
        tables stack is a class. The element's key should be the same as this
        function's first parameter.                                          */
    symtables_stack::put_results put_class_field(symtable_element);

    /*  Returns the quantity of symbols tables in the stack.                 */
    unsigned int get_length(void);

    bool is_empty(void);

    ~symtables_stack(void);
};

#endif