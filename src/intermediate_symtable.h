#ifndef _INTERMEDIATE_SYMTABLE_
#define _INTERMEDIATE_SYMTABLE_

#include "symtable.h"
#include <string>
#include <list>
#include <map>
#include <utility>


/*  ---------------------------------------------------------------------    */
/*  Data types definitions related to ids_info class.                        */

enum id_type { T_STRING
             , T_INT
             , T_FLOAT
             , T_BOOL
             , T_CHAR
             , T_UNDEFINED };

enum id_kind { K_TEMP
             , K_VAR
             , K_OBJECT
             , K_CLASS
             , K_METHOD };

/*  End of data types definitions related to ids_info class.    */
/*  ---------------------------------------------------------------------    */


/*  ---------------------------------------------------------------------    */
/*  Data types definitions related to intermediate_symtable class.           */

enum put_results { IS_RECURSIVE
                 , ID_EXISTS
                 , ID_PUT };

typedef std::pair<put_results, std::string*> t_results;


enum put_func_results { NOT_FUNC
                          , FUNC_EXISTS
                          , FUNC_PUT
                          , FUNC_ERROR /* This last value is meant only for
                                          debugging purposes. */ };

typedef std::pair<put_func_results, std::string*> t_func_results;


enum put_param_results { PARAM_PUT
                      , PARAM_TYPE_ERROR
                      , NO_PREV_FUNC };

typedef std::pair<put_param_results, std::string*> t_param_results;


enum put_class_results { NOT_CLASS
                       , CLASS_EXISTS
                       , CLASS_PUT
                       , CLASS_ERROR /* This last value is meant only for
                                      debugging purposes. */ };

typedef std::pair<put_class_results, std::string*> t_class_results;


enum put_field_results { FIELD_PUT
                      , FIELD_TYPE_ERROR /* Classes cannot be fields of 
                                            another classes. */
                      , NO_PREV_CLASS /* A class should be under analysis 
                                         in order to put a new class 
                                         field in scope. */ 
                      , FIELD_ERROR /*  This last value is meant only for
                                        debugging purposes. */ };

typedef std::pair<put_field_results, std::string*> t_field_results;

typedef std::pair<std::string, int*> t_att;

typedef std::list<t_att> t_attributes;

typedef std::list<std::string> t_params;

/*  End of data types definitions related to intermediate_symtable class.    */
/*  ---------------------------------------------------------------------    */


class ids_info {
public:
    std::string get_next_internal(std::string);

    /*  At the end of the name, a '-' character followed by a number is 
        appended, such that the whole string is a unique identifier, has never
        been used before and will not be asigned to any other identifier in 
        the rest of the source code.                                          
        Parameters: offset inside the method
                  , type of the new temporary variable.                      */
    std::string* new_temp(int, id_type);

    /*  The following register_* methods register the id passed as parameter, 
        as long with their information, into info_map.
        They return the internal representation of the id just registered. It 
        is assured that right after calling this method, the id passed as 
        parameter's representation is unique inside info_map. This 
        representation is what should be used inside any intermediate 
        representaion instruction.                                           */

    /*  Parameters: id with which generate the internal representation id
                  , offset inside the method
                  , type of the new variable.                                */
    std::string register_var(std::string, int, id_type);

    /*  Precondition: the id has been registered, and it is of kind K_TEMP or 
        K_VAR.                                                               */
    id_type get_type(std::string);

    /*  This method is implemented only in case that it is needed in a future
        implementation of the compiler.
        Precondition: the id has been registered, and the content is of kind 
        K_TEMP or K_VAR.                                                     */
    void set_type(std::string, id_type);

    /*  Parameters: id with which generate the internal representation id
                  , offset inside the method
                  , name of the parameter id's type (its class)
                  , the string that represents the beginning address of the 
                    object.                                                  */
    std::string register_obj(std::string, int, std::string, std::string);

    /*  Parameters: id with which generate the internal representation id
                  , number of local variables in the method
                  , name of the class this method belongs to.                */
    std::string register_method(std::string, unsigned int, std::string);

    /*  Parameters: id with which generate the internal representation id
                  , list of attributes in the class (in the same order they 
                    have been declared in the source code).                  */
    std::string register_class(std::string, t_attributes);

    /*  Precondition: There have been more calls to register_* with the 
        parameter id than calls to unregister with the same id.              */
    void unregister(std::string);

    /*  Returns: true, if the parameter id has been registered previously, and
        false, if it has not.                                                */
    bool id_exists(std::string);

    /*  Precondition: the id has been registered.
        Returns: the internal representation of the id inside info_map.      */
    std::string get_id_rep(std::string);



    /*  Precondition: the id has been registered.                            */
    id_kind get_kind(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_VAR or 
        K_OBJECT.
        Returns: the id's offset inside its method's body.                   */
    int get_offset(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_METHOD.
        Returns: the number of local variables inside this method's body.    */
    unsigned int get_local_vars(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_METHOD or
        K_OBJ.
        Returns: the class name this object or method belongs to.             */
    std::string get_owner_class(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_CLASS.
        Returns: the list of all the attributes in the class, ordered as they
        were in the class's definition.                                      */
    t_attributes& get_list_attributes(std::string);

    /*  Precondition: the id has been registered, and it is of kind K_METHOD.
        Returns: the list of all parameters of the function, ordered as they
        were in the method's definition.                                     */
    t_params& get_list_params(std::string);

    /*  Updates the number of local variables (this includes the temporary 
        ones).
        Precondition: the id has been registered, and it is of kind K_METHOD.
        The number of variables passed as parameter should be greater than
        the size of the list of parameters already put in the method's list. */
    void set_number_vars(std::string, unsigned int);

    /*  Updates the offset.
        Precondition: the id has been registered, and it is not of kind 
        K_METHOD or kind K_CLASS.                                            */
    void set_offset(std::string, int);

private:
    struct entry_info {
        id_kind entry_kind;

        /*  For variables and temporaries. For the other entries, 
            entry_type == T_UNDEFINED.                                       */
        id_type entry_type;

        std::string rep;

        /*  For variables and objects.                                       */
        int* offset = NULL;

        /*  For objects (declaring the object's type) and methods (declaring
            the class this method belongs to).                               */
        std::string* owner = NULL;

        /*  For methods.                                                     */
        unsigned int* local_vars = NULL;

        /*  For methods. Represents the information needed to get the 
            beginning address of an object (an object interpreted as an array
            of variables of -possibly- different types; i.e., as an array of
            its attributes).                                                 */
        std::string* begin_address;

        /*  For classes.                                                     */
        t_attributes* l_atts = NULL;

        /*  For methods.                                                     */
        t_params* l_params = NULL;

    };
    

    std::map<std::string, entry_info> info_map;

    /*  The following contains the information necessary to create new internal
        representations out of identifier strings.                           */
    std::map<std::string, unsigned int> internal;

    unsigned int temp_number = 0;
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

    /*  Precondition: the parameter id has already been put in the current 
        scope.
        Returns: the internal representation of the parameter id; it should
        be used instead of the original name in the construction of any 
        intermediate representation instruction.                             */
    std::string get_id_rep(std::string);

    /*  Create a new symbols table related to a block.                       */
    void push_symtable(void);

    /*  Create a new symbols table related to a function or class.
        Precondition: get_class_type() == (T_CLASS || T_FUNCTION)            */
    void push_symtable(symtable_element&);

    /*  Precondition: There have been more calls to push_symtable than
        to pop_symtable; i.e., there is still another symbols table to pop.*/
    void pop_symtable(void);
    
    /*  Precondition: There have been more calls to push_symtable than
        to pop_symtable; i.e., there is still another symbols table to get 
        an element from.                                                     */ 
    symtable_element* get(std::string);

    /*  Parameres:  ID of the attribute
                  , name of the class that the attribute belongs to.
        Precondition: The ID of the attribute has been registered via put_obj*,
        and whose class is the same as the second parameter of this method.
        Return: Offset of the attribute inside its class. Return type is (int*)
        , in case that the parameters do not pass the preconditions.         */
    int* get_offset(std::string, std::string);

    /*  Creates a new temporary variable that will not be inserted into the 
        symbols tables stack, but will be registered in this->information.
        Parameters: the offset inside the temporary variable's method.       
        Returns: a pair containing the result of putting the variable into the
        current scope, along with its representation inside this->information
        (as long as putting the object in scope was succesful; otherwise, NULL
        is returned as second element of the pair). */
    t_results new_temp(int);

    /*  Inserts a new variable as an element into the symbols tables stack. 
        Parameters: the variable itself
                  , the key with which it is going to be registered
                  , the offset inside the variable's method
                  , the variable's type.
        Precondition: There have been more calls to push_symtable(...) than
        to pop_symtable(); i.e., there is still another symbols table to put 
        an element to. The element to be inserted is a variable.
        Returns: a pair containing the result of putting the variable into the
        current scope, along with its representation inside this->information 
        (as long as putting the object in scope was successful; otherwise, 
        NULL is returned as second element of the pair).                     */
    t_results put_var(symtable_element
                    , std::string
                    , int);

    /*  Inserts a new object as an element into the symbols tables stack. 
        Parameters: the object itself
                  , the key with which it is going to be registered
                  , the offset inside the object's method
                  , the object's type (its class)
                  , the string that represents the beginning address of the 
                    object.
        Precondition: There have been more calls to push_symtable(...) than
        to pop_symtable(); i.e., there is still another symbols table to put 
        an element to. The element to be inserted is an object.
        Returns: a pair containing the result of putting the object into the
        current scope, along with its representation inside this->information 
        (as long as putting the object in scope was successful; otherwise, 
        NULL is returned as second element of the pair).                     */
    t_results put_obj(symtable_element&
                    , std::string
                    , int
                    , std::string
                    , std::string);

    /*  Inserts a new function to the symbols tables stack. This function is 
        remembered for future calls of put_*_param. Also, a new symbols
        table is pushed on top of the stack, and every subsequent call to 
        put_func_param is performed to this function and this recently created 
        symbols table. Finally, the function's id is registered (with a unique
        representation, of course) inside this->information.
        Parameters: the function itself
                  , the key with which it is going to be registered
                  , its number of local variables
                  , the name of the class this function belongs to.
        Precondition: There have been more calls to push_symtable than
        to pop_symtable; i.e., there is still another symbols table to put 
        an element to. The element to be inserted is a function. Every 
        previous function in the current class under analysis has already 
        been fully analyzed.
        Returns: a pair containing the result of putting the function into
        the current scope (which implies pushing a new symbols table and other 
        things), along with its representation inside this->information (as 
        long as putting the function in scope was successful; otherwise, NULL
        is returned as second element of the pair).                          */
    t_func_results put_func(symtable_element&
                          , std::string
                          , unsigned int
                          , std::string);

    /*  Inserts a new variable to the lastly inserted function (via put_func) 
        as a parameter.
        Parameters: the variable itself
                  , the key with which it is going to be registered
                  , the variable's offset inside the method it belongs to
                  , the variable's type.
        Precondition: The element is a variable, and the string given to this
        method is equal to the variable's key.
        Returns: a pair containing the result of putting the variable into the
        current scope (as well as in the list of parameters of the function it
        is a parameter of), along with its representation inside 
        this->information (as long as putting the variable in scope was 
        successful; otherwise, NULL is returned as second parameter).        */
    t_param_results put_var_param(symtable_element&
                                , std::string
                                , int);

    /*  Inserts a new object to the lastly inserted function (via put_func) as
        a parameter.
        Parameters: the object itself
                  , the key with which it is going to be registered
                  , the object's offset inside the method it belongs to
                  , the object's type(its class).
                  , the string that represents the beginning address of the 
                    object.
        Precondition: The element is an object, and the second parameter 
        given to this method is equal to the object's key.
        Returns: a pair containing the result of putting the object into the
        current scope (as well as in the list of parameters of the function it
        is a parameter of), along with its representation inside 
        this->information (as long as putting the object in scope was 
        successful; otherwise, NULL is returned as second parameter).        */
    t_param_results put_obj_param(symtable_element&
                                , std::string
                                , int
                                , std::string
                                , std::string);

    /*  Updates the number of local variables (this includes the temporary 
        ones).
        Precondition: the id has been previously put.                        */
    void set_number_vars(std::string
                       , unsigned int);

    /*  Simply performs a pop operation on the stack and resets the value of
        last_func. Caution not to pop some other symbols table on top of the 
        stack is advised. Future calls to put_func_param should be preceded by 
        a put_func call. 
        Precondition: There have been more calls to put_func than to 
        finish_func_analysis; i.e., there is a function's symbols table to be 
        popped.                                                        */
    void finish_func_analysis(void);

    /*  Inserts a new class to the symbols tables stack. This class is 
        remembered for future calls of put_*_field. Also, a new symbols
        table is pushed on top of the stack, and every subsequent call to
        put_*_field is performed to this class and the recently created
        symbols table. Finally, the class's id is registered inside 
        this->information.
        Parameters: the class itself
                  , the key with which it is going to be registered
                  , the list of local attributes of the class.
        Precondition: There have been more calls to push_symtable than
        to pop_symtable; i.e., there is still another symbols table to put 
        an element to. The element to be inserted is a class. Every previous 
        class has already been fully analyzed.
        Returns: a pair contaiing the result of putting the class into the 
        current scope (which implies pushing a new symbols table, and other 
        things), along with its representation inside this->information (as
        long as putting the class in scope was succesful; otherwise, NULL is 
        returned as second element of the pair).                             */
    t_class_results put_class(symtable_element&
                            , std::string
                            , t_attributes);

    /*  Inserts a new variable as a field to the lastly inserted class (via 
        put_class).
        Parameters: the variable itself
                  , the key with which it is going to be registered
                  , the offset inside the method its belong to
                  , the variable's type.
        Precondition: the second parameter matches the variable's key, and 
        there is a class to push this field into.
        Returns: a pair containing the result of putting the variable into the
        current scope (which implies putting it in the class's list of fields
        as well, and other things), along with its representation inside 
        this->information (as long as putting the variable in scope was 
        successful; otherwise, NULL is returned as second element of the 
        pair).                                                               */
    t_field_results put_var_field(symtable_element&
                                , std::string
                                , int);

    /*  Inserts a new object as a field to the lastly inserted class (via 
        put_class).
        Parameters: the object itself
                  , the key with which it is going to be registered
                  , the offset inside the method its belong to
                  , the object's type (its class)
                  , the string that represents the beginning address of the 
                    object.
        Precondition: the second parameter matches the object's key, and 
        there is a class to push this field into.
        Returns: a pair containing the result of putting the object into the
        current scope (which implies putting it in the class's list of fields
        as well, and other things), along with its representation inside 
        this->information (as long as putting the object in scope was 
        successful; otherwise, NULL is returned as second element of the 
        pair).                                                                */
    t_field_results put_obj_field(symtable_element&
                                , std::string
                                , int
                                , std::string
                                , std::string);

    /*  Inserts a new function as a field to the lastly inserted class (via 
        put_class).
        Parameters: the function itself
                  , the key with which it is going to be registered
                  , the number of local variables in it
                  , the class it belongs to.
        Precondition: the second parameter matches the function's key, and 
        there is a class to push this field into.
        Returns: a pair containing the result of putting the function into the
        current scope (which implies putting it in the class's list of fields
        as well, and other things), along with its representation inside 
        this->information (as long as putting the  in scope was 
        successful; otherwise, NULL is returned as second element of the 
        pair.                                                                */
    t_field_results put_func_field(symtable_element&
                                 , std::string
                                 , unsigned int
                                 , std::string);

    /*  Simply performs a pop operation on the stack and resets the value of
        last_class. Caution not to pop some other symbols table on top of the 
        stack is advised. Future calls to put_class_field should be preceded by
        a put_class call. 
        Precondition: There has been exactly one more call to put_class than to 
        finish_class_analysis; i.e., there is a class's symbols table to be 
        popped.                                                        */
    void finish_class_analysis(void);

    std::string new_label(void);

    /*  Returns the quantity of symbols tables in the stack.                 */
    unsigned int size(void);

private:
    ids_info* information;

    symtables_stack scopes;

    std::string* class_name = NULL;

    std::string* func_name = NULL;

    unsigned int number_label = 0;
};

#endif
