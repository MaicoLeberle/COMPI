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

    /*  Basic type variable.
        Precondition: the type is neither VOID nor ID.                       */
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
        Precondition: get_class_type() of the symtable_element whose key is 
        this function's first parameter should be == 
        (symtable_element::T_FUNCTION || symtable_element::T_CLASS).         */
    symtable(std::string, bool);

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
    std::list<symtable*> stack;
    symtable_element* last_func;
    symtable_element* last_class;

public:
    /*  This enumeration will be used as a way to inform the symtables_stack 
        user the result of calling put(...).                                 */
    enum put_results { IS_RECURSIVE
                     , ID_EXISTS
                     , ID_PUT };
    enum put_func_results { NOT_FUNC
                          , FUNC_EXISTS
                          , FUNC_PUT
                          , FUNC_ERROR /* This last value is meant only for
                                          debugging purposes. */ };
    enum put_class_results { NOT_CLASS
                           , CLASS_EXISTS
                           , CLASS_PUT
                           , CLASS_ERROR /* This last value is meant only for
                                          debugging purposes. */ };
    enum put_param_results { PARAM_PUT
                          , PARAM_TYPE_ERROR
                          , NO_PREV_FUNC };

    enum put_field_results { FIELD_PUT
                          , FIELD_TYPE_ERROR /* Classes cannot be fields of 
                                                another classes. */
                          , NO_PREV_CLASS /* A class should be under analysis 
                                             in order to put a new class 
                                             field in scope. */ };

    /*  This constructor is meant only for the empty stack.                  */
    symtables_stack(void);

    /*  Create a new symbols table related to a block.                       */
    void push_symtable(void);

    /*  Create a new symbols table related to a function or class.
        PRECONDITION: get_class_type() == (T_CLASS || T_FUNCTION)            */
    void push_symtable(symtable_element&);

    /*  Precondition: There have been more calls to push_symtable than
        to pop_symtable; i.e., there is still another symbols table to pop.*/
    void pop_symtable(void);
    
    /*  Precondition: There have been more calls to push_symtable than
        to pop_symtable; i.e., there is still another symbols table to get 
        an element from.                                                     */ 
    symtable_element* get (std::string);

    /*  Inserts a new element into the symbols tables stack. 
        Precondition: There have been more calls to push_symtable(...) than
        to pop_symtable(); i.e., there is still another symbols table to put 
        an element to. The element to be inserted is not a class nor a 
        function.                                                            */
    symtables_stack::put_results put(std::string, symtable_element);

    /*  Inserts a new function to the symbols tables stack. This function is 
        remembered for future calls of put_func_param. Also, a new symbols
        table is pushed on top of the stack, and every subsequent call to 
        put_func_param is performed to this function and this recently created 
        symbols table.
        Precondition: There have been more calls to push_symtable than
        to pop_symtable; i.e., there is still another symbols table to put 
        an element to. The element to be inserted is a function. Every 
        previous function in the current class under analysis has already 
        been fully analyzed.                                                */
    symtables_stack::put_func_results put_func(std::string, symtable_element&);

    /*  Inserts a new parameter to the lastly inserted function (via put_func).
        Precondition: The element is not a function or a class, and the string 
        given to this put_func_param is equal to the element's key.          */
    symtables_stack::put_param_results put_func_param(std::string, symtable_element&);

    /*  Simply performs a pop operation on the stack and resets the value of
        last_func. Caution not to pop some other symbols table on top of the 
        stack is advised. Future calls to put_func_param should be preceded by 
        a put_func call. 
        Precondition: There have been more calls to put_func than to 
        finish_func_analysis; i.e., there is a function's symbols table to be 
        popped.                                                        */
    void finish_func_analysis(void);

    /*  Inserts a new classto the symbols tables stack. This class is 
        remembered for future calls of put_func_param. Also, a new symbols
        table is pushed on top of the stack, and every subsequent call to
        put_class_field is performed to this class and this recently created
        symbols table.
        Precondition: There have been more calls to push_symtable than
        to pop_symtable; i.e., there is still another symbols table to put 
        an element to. The element to be inserted is a class. Every previous 
        class has already been fully analyzed.                               */
    symtables_stack::put_class_results put_class(std::string, symtable_element&);

    /*  Inserts a new class field to the lastly inserted class (via put_class).
        Precondition: The element is not a class, and the string given to this
         put_class_field call is equal to the element's key.                 */
    symtables_stack::put_field_results put_class_field(std::string, symtable_element&);

    /*  Simply performs a pop operation on the stack and resets the value of
        last_class. Caution not to pop some other symbols table on top of the 
        stack is advised. Future calls to put_class_field should be preceded by
        a put_class call. 
        Precondition: There has been exactly one more call to put_class than to 
        finish_class_analysis; i.e., there is a class's symbols table to be 
        popped.                                                        */
    void finish_class_analysis(void);

    /*  Returns the quantity of symbols tables in the stack.                 */
    unsigned int size(void);

    ~symtables_stack(void);
};

#endif