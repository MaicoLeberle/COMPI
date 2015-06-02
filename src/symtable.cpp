#include <cassert>
#include <string>
#include <list>
#include <unordered_map>

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

/*  symtable_entry methods.                                                  */

/*  Each constructor receives the minimum amount of information necessary to
    resolve the id_class, id_type, etc., that the identifier has.            */

symtable_element::symtable_element(id_type t) :
    c_id(T_VAR), t_id(t) { }

symtable_element::symtable_element(id_type t, unsigned int d) : 
    c_id(T_ARRAY), t_id(t), dim(d) { }

symtable_element::symtable_element(std::string* s) : 
    c_id(T_OBJ), t_id(ID), class_type(s) { }  

symtable_element::symtable_element(std::string* s, unsigned int d) :
    c_id(T_OBJ_ARRAY), t_id(ID), class_type(s) { }

symtable_element::symtable_element(id_type t, std::list<std::string>* f) :
     c_id(T_FUNCTION), t_id(t), func_params(f) { } 

symtable_element::symtable_element(std::string* s, std::list<std::string>* f) :
    c_id(T_CLASS), class_fields(f), class_type (s) { }

symtable_element::id_class symtable_element::get_class () { 
    return c_id; 
}

symtable_element::id_type symtable_element::get_type () { 
    return t_id; 
}

std::string* symtable_element::get_class_type () {
    assert(c_id == T_VAR && t_id == ID);
    return (class_type); 
}

unsigned int symtable_element::get_dimension () { 
    assert(c_id == T_ARRAY); 
    return dim; 
}

std::list<std::string>* symtable_element::get_func_params () {
    assert(c_id == T_FUNCTION); 
    return (func_params); 
}

std::list<std::string>* symtable_element::get_class_fields () {
    assert(c_id == T_CLASS);
    return (class_fields); 
}

/*  symtable methods.                                                        */

bool symtable::elem_exists (std::string key) {
    return (hashtable.find(key) != hashtable.end());
}

symtable_element symtable::get(std::string key) {
    assert(elem_exists(key));
    return (hashtable[key]);
}

void symtable::put (std::string key, symtable_element value) {
    hashtable[key] = value;
}

/*  scopes_list methods.                                                     */

symtable_element* scopes_list::get(std::string key) {
     /*  Must search from the top of the stack and downwards.        */
    for(scopes_list *it = this; it != NULL; it = it->prev) {
        if((it->table).elem_exists(key)) {
            symtable_element e = (it->table).get(key);
            symtable_element* ret_elem;
            switch (e.get_class()) {
                case (symtable_element::T_FUNCTION):
                    ret_elem = new symtable_element(e.get_type(), e.get_func_params());
                    break;
                case (symtable_element::T_CLASS):
                    ret_elem = new symtable_element(e.get_class_type(), e.get_class_fields());
                    break;
                case (symtable_element::T_VAR):
                    ret_elem = new symtable_element(e.get_type());
                    break;
                case (symtable_element::T_OBJ):
                    ret_elem = new symtable_element(e.get_class_type());
                    break;
                case (symtable_element::T_ARRAY):
                    ret_elem = new symtable_element(e.get_type(), e.get_dimension());
                    break;
                case (symtable_element::T_OBJ_ARRAY):
                    ret_elem = new symtable_element(e.get_class_type(), e.get_dimension());
                    break;
            }
            return (ret_elem);
        }
    }
    /*  If the key has not been found in any of the symbols 
        tables, then it has not been found in the current scope.     */
    return (NULL);
}

void scopes_list::put(std::string key, symtable_element value) {
    /*  Always insert a new identifier's information in the top
        of the stack.                                               */
    table.put(key, value);
}

scopes_list::~scopes_list() {
    if (prev != NULL)
        delete prev;
}

int main() { return 0; }