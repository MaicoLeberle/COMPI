#ifndef _INTERMEDIATE_SYMTABLE_
#define _INTERMEDIATE_SYMTABLE_

#include "symtable.h"
#include <string>
#include <map>

class ids_info {
public:
    ids_info(void);

    bool id_exists(std::string);

    /*  TODO: every specific kind of register (for variables,objects, methods, classes)
        should be implemented.
    void register(std::string); */

    enum id_kind { K_VAR
              , K_OBJECT
              , K_CLASS
              , K_METHOD };

    id_kind get_kind(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_VAR or 
        K_OBJECT.
        Returns: the id's offset inside its method's body.                   */
    unsigned int get_offset(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_METHOD.
        Returns: the number of local variables inside this method's body.    */
    unsigned int get_local_vars(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_METHOD or
        K_OBJ.
        Returns: the class name this method belongs to.                      */
    std::string get_owner_class(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_CLASS.
        Returns: the list of all the attributes in the class, ordered as they
        were in the class's definition.                                      */
    std::list<std::string> get_list_attributes(std::string);

private:
    union t_info {
        /* For variables. */
        unsigned int offset;

        /* For objects. */
        struct obj_info {   
            unsigned int offset; /* Offset inside the function it belongs to. */
            std::string owner; /* Object's type. */
        };

        /* For methods. */
        struct func_info {
            unsigned int local_vars; /* Number of local variables in the method's 
                                        body. */
            std::string owner;  /* Class the method belongs to. */
        };

        /* For classes. */
        std::list<std::string> l_atts; /* List of attributes in a class, in the 
                                          order they appear in it. */
    };

    std::map<std::string, t_info> info_map;
};


/*  intermediate_symtable has every method (and semantic action, for that 
    matter) symtables_stack has, but also creates an ids_info object and 
    updates it accordingly to the actions performed to the symtables_stack
    object.                                                                  */

class intermediate_symtable {
public:
    intermediate_symtable(void);

    /*  After analysis, this method should be called and its return value 
        passed onto the next phase of the compilation: the object code 
        generation.                                                          */
    ids_info* get_ids_info(void);

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

    /*  Inserts a new class to the symbols tables stack. This class is 
        remembered for future calls of put_class_field. Also, a new symbols
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

private:
    ids_info* information;

    symbols_stack scopes;
};

#endif