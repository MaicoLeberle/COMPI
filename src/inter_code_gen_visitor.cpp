#include "inter_code_gen_visitor.h"

/* TODO: cosas que faltan:

 	 	 _ definir una nueva instrucción de 3-direcciones que me indique
 	 	 que tengo que generar código para declarar un string.
*/
std::string printable_field_id(reference_list ids);

inter_code_gen_visitor::inter_code_gen_visitor() {
	this->inst_list = new instructions_list();
	this->offset = 0;
	this->into_method = false;
}

instructions_list* inter_code_gen_visitor::get_inst_list() {
	return inst_list;
}

intermediate_symtable* inter_code_gen_visitor::get_symtable(){
	return &s_table;
}

ids_info* inter_code_gen_visitor::get_ids_info(){
	return s_table.get_ids_info();
}

symtable_element* inter_code_gen_visitor::get_next_symtable_element(symtable_element
*actual_element, std::string id){
	#ifdef __DEBUG
		// PRE
		assert(actual_element->get_kind() == id_kind::K_CLASS);
	#endif

	symtable_element *ret = nullptr;

	std::list<symtable_element> *fields = actual_element->get_class_fields();
	for(std::list<symtable_element>::iterator it = fields->begin();
		it != fields->end(); ++it){

		if(it->get_key() == id){
			ret = &(*it);
			break;
		}
	}

	return ret;
}

// TODO: cuando pedimos los atributos de un método devuelve los ids originales!
// lo mismo con los atributos de una clase.
symtable_element* inter_code_gen_visitor::dereference(reference_list ids){
	#ifdef __DEBUG
		// PRE
		assert(s_table.size() > 0);
		std::cout << "Accessing reference " << ids[0] << std::endl;
	#endif

	symtable_element *ret = nullptr;
	symtable_element *aux = this->s_table.get(ids[0]);
	id_kind aux_kind;

	std::vector<int>::size_type i = (std::vector<int>::size_type) 1;
	std::vector<int>::size_type limit = ids.size();
	// INV : {aux is the symtable_element for the id number i-1 in ids}
	while(i < limit){
		#ifdef __DEBUG
			std::cout << "Accessing reference " << ids[i] << std::endl;
		#endif

		// Index the previous symtable_element, with the next id.
		aux_kind = aux->get_kind();
		// {aux_kind == id_kind::K_OBJECT}
		// Get the class of the object.
		std::string *object_class_name = aux->get_class_type();
		symtable_element *object_class = this->s_table.get(*object_class_name);
		#ifdef __DEBUG
			assert(object_class->get_kind() != id_kind::K_NOT_FOUND);
		#endif
		// Obtain the next element.
		aux = this->get_next_symtable_element(object_class, ids[i]);
		i++;
	}

	return ret;
}

// TODO: este método está repetido en semantic_analysis
id_type inter_code_gen_visitor::determine_type(Type::_Type type_ast){
	id_type ret;

	switch(type_ast){
		case Type::INTEGER:
			ret = id_type::T_INT;
			break;

		case Type::FLOAT:
			ret = id_type::T_FLOAT;
			break;

		case Type::BOOLEAN:
			ret = id_type::T_BOOL;
			break;

		case Type::VOID:
			ret = id_type::T_VOID;
			break;

		case Type::ID:
			ret = id_type::T_ID;
			break;

		case Type::STRING:
			ret = id_type::T_STRING;
			break;

		#ifdef __DEBUG
			default:
				assert(false);
		#endif
	}

	return ret;
}

std::string inter_code_gen_visitor::obtain_methods_label(reference_list ids){
	std::string method_name = ids[ids.size()-1];
	std::string class_name;

	if(ids.size() >= 2){
		// { It is a call of the form obj1. ... .objn.method() }
		std::string obj_name = ids[ids.size()-2];

		symtable_element *object = s_table.get(obj_name);
		class_name = *object->get_class_type();
	}
	else{
		// {ids.size() < 2}
		// {It is a call of the form method()}
		// The class to which the method belongs to, is the actual class.
		class_name = actual_class->get_key();
	}
	return class_name + "." + method_name;
}

/*	Determines the width of variable of a given type, being it a basic type
 *  or an instance.
 * */
unsigned int inter_code_gen_visitor::calculate_size(id_type type, 
                                                    std::string class_name){
	unsigned int size = 0;

	switch(type){
		case id_type::T_INT:
			size = INTEGER_WIDTH;
			break;

		case id_type::T_FLOAT:
			size = FLOAT_WIDTH;
			break;

		case id_type::T_BOOL:
			size = BOOLEAN_WIDTH;
			break;

		case id_type::T_ID:
			// TODO: esta información se tendría que almacenar directamente
			// en la tabla de símbolos.
			symtable_element *class_elem = s_table.get(class_name);
			std::list<symtable_element> *fields = class_elem->get_class_fields();

			for(std::list<symtable_element>::iterator it = fields->begin();
			it != fields->end(); it++){

				if((*it).get_kind() != id_kind::K_METHOD){
					if((*it).get_class_type() != nullptr){
						size += this->calculate_size((*it).get_type(),
													*(*it).get_class_type());
					}
					else{
						// {(*it).get_class_type() == nullptr}
						size += this->calculate_size((*it).get_type(),
													std::string(""));
					}
				}
			}

			break;
	}

	return size;
}

void inter_code_gen_visitor::array_initialization(
id_type type, std::string id, unsigned int array_length){

	address_pointer dest, constant = nullptr;
	// Register the array's identifier into the symbol's table.
	symtable_element variable(id, type, array_length);
	t_results pair = this->s_table.put_var(variable, id, this->offset);

	#ifdef __DEBUG
		assert(std::get<0>(pair) == ID_PUT);
	#endif
    std::string id_array = *std::get<1>(pair);

	// Initialize each position of the array with the corresponding
	// initial value.
	unsigned int arr_pos_width = this->calculate_size(type, std::string(""));
	unsigned int arr_size =  arr_pos_width*array_length;

	switch(type){
		case id_type::T_INT:
			constant = new_integer_constant(integer_initial_value);
			break;

		case id_type::T_FLOAT:
			constant = new_float_constant(float_initial_value);
			break;

		case id_type::T_BOOL:
			constant = new_boolean_constant(boolean_initial_value);

		#ifdef __DEBUG
			default:
				assert(false);
		#endif
	}

	dest = new_name_address(id_array);
	for(int arr_offset = 0;
		arr_offset < arr_size;
		arr_offset += arr_pos_width){

		inst_list->push_back(
				new_indexed_copy_to(dest,
									new_integer_constant(arr_offset),
									constant));
	}
	this->offset += arr_size;
}

/*	Translates a variable definition, registers its information into the
 * 	table of symbols and updates the offset.
 * 	PRE : {the actual value of offset indicates where, the new variable,
 * 			must be stored}
 * */

void inter_code_gen_visitor::translate_var_def(id_type type,
std::string id, std::string class_name, unsigned int array_length){
	address_pointer dest, constant = nullptr;

	if(type == id_type::T_ID){
		this->instance_initialization(class_name, id);
		// After instance_initialization, offset had been updated correctly.
	}
	else{
        // {type != id_type::T_ID}

		if (array_length > 0){
			this->array_initialization(type, id, array_length);
		}
		else {
			// {array_length <= 0}
			// Basic type
			switch(type){
				case id_type::T_INT:
					constant = new_integer_constant(integer_initial_value);
					break;

				case id_type::T_FLOAT:
					constant = new_float_constant(float_initial_value);
					break;

				case id_type::T_BOOL:
					constant = new_boolean_constant(boolean_initial_value);
			}
			symtable_element variable(id, type);

			if(class_name != std::string("")){
				// Then it is an instance's attribute.
                // By pre-condition, id is the unique identifier of the instance.
			    dest = new_name_address(id);
				inst_list->push_back(new_indexed_copy_to(dest,
									  new_integer_constant(this->offset),
									  constant));
			}
			else{
				// {class_name == std::string("")}
				// The variable defined is not an attribute of an instance.
                // By pre-condition, id is the original identifier of the variable.
				t_results pair = s_table.put_var(variable, id, this->offset);
				#ifdef __DEBUG
					assert(std::get<0>(pair) == ID_PUT);
				#endif
                dest = new_name_address(*std::get<1>(pair));
				inst_list->push_back(new_copy(dest, constant));
			}

			this->offset += calculate_size(type, std::string(""));
		}
	}
}

/* Returns the offset that corresponds to the object's attribute indicated by
 * ids, with respect to the beginning of the object indicated by the first
 * element in ids.
 * PRE : {ids.size() > 1} */
int inter_code_gen_visitor::get_attribute_offset(reference_list ids){
	int at_offset = 0;

	reference_list::iterator it = ids.begin();
	reference_list::iterator it_end = ids.end();
	// {it is the object}
	symtable_element *object = this->s_table.get(*it);
	symtable_element *element = nullptr;
	std::string *class_name = object->get_class_type();
	it++;

	// INV : {class_name is the name of the class to which "it" is and
	//			attribute of}
	while(it != it_end){
		at_offset += *this->s_table.get_offset(*it, *class_name);

		element = s_table.get(*it);

		if(element->get_kind() == id_kind::K_OBJECT){
			class_name = element->get_class_type();
		}
		it++;
	}

	return at_offset;
}

std::string inter_code_gen_visitor::instance_initialization(std::string id_class,
std::string id_instance){

	// Get information about the attributes of the instance.
	symtable_element *class_object = s_table.get(id_class);
	std::list<symtable_element> *fields = class_object->get_class_fields();
	std::string first_field("");

	// Update the symbol's table, with information about the instance.
	// The identifier of the instance will "point" to the first attribute.
	// It is placed before the attributes. Update the offset properly.
	symtable_element id(id_instance, new std::string(id_class));
	t_results pair_id_instance = s_table.put_obj(id,
									id_instance,
									this->offset,
									id_class,
									std::string(""));

	#ifdef __DEBUG
		assert(std::get<0>(pair_id_instance) == ID_PUT);
	#endif
    std::string unique_id_instance =  *std::get<1>(pair_id_instance);

    // Reset this->offset, to compute offsets of the attributes, with respect to the 
    // beginning of the instance.
	unsigned int prev_offset = this->offset;
	this->offset = 0;    
    
    // Add the size of the first field saved: the reference to the first attribute.
	this->offset += reference_width;

	// Extract the beginning
	std::list<symtable_element>::iterator it = fields->begin();

	if(it != fields->end() and
	(*it).get_kind() != id_kind::K_METHOD){
		// Register the first attribute, just to remember the position of the first attribute,
        // and to take its effective address later.
		symtable_element variable(first_field, (*it).get_class_type());
		t_results pair_first_field = this->s_table.put_var(variable, 
                                                            unique_id_instance + "." + (*it).get_key(), 
                                                            prev_offset + reference_width);
		#ifdef __DEBUG
			assert(std::get<0>(pair_first_field) == ID_PUT);
		#endif

        first_field = *std::get<1>(pair_first_field);
	}
	else{
		// {it == fields->end() or
		// 	(*it).get_kind() == id_kind::K_METHOD}

		// The class hasn't attributes.
		// TODO: cómo hacemos en este caso de clase sin atributos?
		first_field = unique_id_instance;
	}

	for(; it != fields->end(); ++it){
		if((*it).get_kind() != id_kind::K_METHOD){
			translate_var_def((*it).get_type(), unique_id_instance, id_class, 0);
		}
	}

	// Save the address of the first attribute, into a pointer.
    inst_list->push_back(new_unary_assign(new_name_address(unique_id_instance),
										new_name_address(first_field),
										quad_oper::ADDRESS_OF));

    // Restore and update the offset.
    this->offset += prev_offset;

	return first_field;
}

void inter_code_gen_visitor::visit(node_program& node){
	#ifdef __DEBUG
		std::cout << "Beginning translation." << std::endl;
	#endif
	// Define a new block: the global scope
	s_table.push_symtable();

	for(auto c : node.classes) {
		c->accept(*this);
	}
}

void inter_code_gen_visitor::visit(node_class_decl& node) {
	#ifdef __DEBUG
		std::cout << "Translating class " << node.id << std::endl;
	#endif

	// Translate the fields of the class.
	// We do this to allow mixed declarations of fields and methods.
	std::list<class_block_pointer> fields;
	std::list<class_block_pointer> methods;
	for(auto cb : node.class_block) {
		if (cb->is_node_field_decl()){
			fields.push_back(cb);
		}
		else{
			// {not cb->is_node_field_decl()}
			methods.push_back(cb);
		}
	}

	// Saving information of fields..
	symtable_element new_class(new std::string(node.id),
							   new std::list<symtable_element>());
    
    // Relax...this method, internally, calls all the others, so this 
    // stack frame is always present and this reference is always valid. 
	this->actual_class = &new_class;

	// Define a new scope
	this->s_table.push_symtable();
    // TODO: el segundo argumento tiene que ser el nombre de la clase, o un
    // id único?
	#ifdef __DEBUG
		t_class_results ret = this->s_table.put_class(new_class, new_class.get_key());
		assert(std::get<0>(ret) == CLASS_PUT);
	#else
		this->s_table.put_class(new_class, new_class.get_key());
	#endif

	// Gather information about fields. Set relative addresses with respect to the
	// beginning of this class definition.
	int prev_offset = this->offset;
	this->offset = 0;
	this->into_method = false;

	for(std::list<class_block_pointer>::iterator it = fields.begin();
		it != fields.end();
		++it){
			node_field_decl& aux = static_cast<node_field_decl&> (*(*it));
			aux.accept(*this);
	}
	// End of class declaration -> in offset we have the size of an instance.

	// Translation of methods.
	for(std::list<class_block_pointer>::iterator it = methods.begin();
		it != methods.end();++it){
			node_method_decl& aux = static_cast<node_method_decl&> (*(*it));
			aux.accept(*this);
	}

	// Restore offset value.
	this->offset = prev_offset;
	this->s_table.finish_class_analysis();
}

void inter_code_gen_visitor::visit(node_field_decl& node) {
	#ifdef __DEBUG
		std::cout << "Translating field declaration." << std::endl;
	#endif

	for(auto f : node.ids) {
		#ifdef __DEBUG
			std::cout << "Translating field " << f->id << std::endl;
		#endif

		// Translate each identifier. Add the identifier to the symbol table.
		if (node.type.type == Type::ID){
			// Declaration of instances.
			std::string *class_name = new std::string(node.type.id);
			symtable_element id(f->id, class_name);

			if(into_method){
				// Insert code that initializes the attributes of the object.
				std::string first_field = instance_initialization(node.type.id,
																f->id);
			}
			else{
				// {not into_method}
				// It is an attribute declaration. We just save the corresponding
				// data, for future uses.
				t_field_results pair =
						s_table.put_obj_field(id,
											f->id,
											this->offset,
											node.type.id,
											std::string(""));
				#ifdef __DEBUG
					assert(std::get<0>(pair) == FIELD_PUT);
				#endif

				this->offset += calculate_size(determine_type(node.type.type),
															node.type.id);
			}
		}
		else{
			// {node.type.type != Type::ID}
			// Basic type or array
			if (f->array_size > 0){
				// Array
				if(into_method){
					translate_var_def(determine_type(node.type.type), f->id,
					std::string(""), f->array_size);
				}
				else{
					// {not into_method}

					symtable_element id(f->id,
										determine_type(node.type.type),
										f->array_size);
					t_field_results pair = s_table.put_var_field(id,
																f->id,
																this->offset);
					#ifdef __DEBUG
						assert(std::get<0>(pair) == FIELD_PUT);
					#endif
					this->offset += calculate_size(
								determine_type(node.type.type), std::string(""))
								* f->array_size;
				}
			}
			else{
				// {f->array_size <= 0}
				// Basic type
				if(into_method){
					translate_var_def(determine_type(node.type.type), f->id,
										std::string(""), 0);
				}
				else{
					// {not into_method}
					symtable_element id(f->id, determine_type(node.type.type));
					t_field_results pair = this->s_table.put_var_field(id,
																        f->id,
																        this->offset);

					#ifdef __DEBUG
						assert(std::get<0>(pair) == FIELD_PUT);
					#endif

					this->offset += calculate_size(
										determine_type(node.type.type),
										std::string(""));
				}

			}
		}
	}
}

void inter_code_gen_visitor::visit(node_id& node) {
	// This visitor does not need to do some task into node_id.
}

void inter_code_gen_visitor::visit(node_method_decl& node){
	#ifdef __DEBUG
		std::cout << "Translating method " << node.id << std::endl;
	#endif

	symtable_element method(node.id,
							determine_type(node.type.type),
							new std::list<symtable_element>(),
							node.body->is_extern);

	t_func_results pair = s_table.put_func(method,
											node.id,
											0,
											actual_class->get_key());
	#ifdef __DEBUG
	    assert(std::get<0>(pair) == FUNC_PUT);
    #endif
	// Generate code for the method declaration.
	std::string *class_name = new std::string(this->actual_class->get_key());
	inst_list->push_back(new_label_inst(new_method_label_address(node.id,
																*class_name)));

	// Initialize offset.
	unsigned int offset_prev = this->offset;
	this->offset = 0;

	// Add the "this" reference as first parameter.
	std::string this_param("this");
	symtable_element obj_param(this_param, class_name);
    std::cout << "Llamando a put_obj_param" << std::endl;
	t_param_results pair1 = this->s_table.put_obj_param(obj_param,
												    this_param,
												    this->offset,
												    *class_name,
												    std::string(""));
    std::cout << "Listo..." << std::endl;
	#ifdef __DEBUG
		assert(std::get<0>(pair1) == PARAM_PUT);
	#endif

    this->this_unique_id = *std::get<1>(pair1);

	this->offset += reference_width;

	// Add the remaining parameters.
	for(auto p : node.parameters) {
		p->accept(*this);
	}

	this->offset = 0;
	into_method = true;
	// Save inst_list
	instructions_list *aux = inst_list;
	inst_list = new instructions_list();
	node.body->accept(*this);
	// Into offset we have the size of the sum of the local and temporal variables.
	// We add the corresponding enter instruction at the beginning of the procedure.
	aux->push_back(new_enter_procedure(this->offset));

	for (instructions_list::iterator it = inst_list->begin();
	it != inst_list->end(); ++it){
			aux->push_back(*it);
	}

	delete(inst_list);

	inst_list = aux;
	this->offset = offset_prev;
	s_table.finish_func_analysis();
	t_field_results pair2 = s_table.put_func_field(method,
												method.get_key(),
												0,
												actual_class->get_key());
	#ifdef __DEBUG
		assert(std::get<0>(pair2) == FIELD_PUT);
    #endif
}

void inter_code_gen_visitor::visit(node_parameter_identifier& node) {
	#ifdef __DEBUG
		std::cout << "Translating parameter " << node.id << std::endl;
	#endif

	switch(node.type.type){
		case Type::ID:{
				std::string *class_name = new std::string(node.type.id);
				// TODO: el último parámetro debería ser un string de la forma
				// node.id "." nombre del primer atributo de la clase
				symtable_element obj_param(node.id, class_name);

				t_param_results pair1 = s_table.put_obj_param(obj_param,
															 node.id,
															 this->offset,
															 node.type.id,
															 std::string(""));

				#ifdef __DEBUG
					assert(std::get<0>(pair1) == PARAM_PUT);
				#endif
				// Update the offset so, outside this method, this value is used to
				// calculate how much data is reserved into the stack.
				this->offset += reference_width;
			}
			break;

		default:{
				symtable_element var_param(node.id, determine_type(node.type.type));
				t_param_results pair2 = s_table.put_var_param(var_param,
															var_param.get_key(),
															this->offset);
				#ifdef __DEBUG
					assert(std::get<0>(pair2) == PARAM_PUT);
				#endif
				// Update the offset so, outside this method, this value is used to
				// calculate how much data is reserved into the stack.
				this->offset += calculate_size(determine_type(node.type.type),
												std::string(""));
			}
	}
}

void inter_code_gen_visitor::visit(node_body& node) {
	#ifdef __DEBUG
		std::cout << "Translating body of method" << std::endl;
	#endif

	if(node.is_extern){
		#ifdef __DEBUG
			std::cout << "The body is extern" << std::endl;
		#endif
	}
	else {
		#ifdef __DEBUG
			std::cout << "The body is declared" << std::endl;
		#endif

		node.block->accept(*this);
	}
}

void inter_code_gen_visitor::visit(node_block& node) {
#ifdef __DEBUG
	std::cout << "Translating a block." << std::endl;
#endif
	// A block defines a new scope.
	s_table.push_symtable();

	// Analyze the content of the block.
	for(auto s : node.content) {
		stm_call_appropriate_accept(s);
	}
	s_table.pop_symtable();
}

void inter_code_gen_visitor::visit(node_assignment_statement& node) {
	#ifdef __DEBUG
		std::cout << "Translating assignment statement." << std::endl;
	#endif
	this->expr_call_appropriate_accept(node.get_expression());
	address_pointer r_value = this->temp;
    address_pointer l_value = nullptr;
    address_pointer index = nullptr;
	location_pointer location = node.get_location();
    reference_list ids = location->get_ids();
    bool is_att_from_act_obj = location->get_is_attribute_from_actual_object();

    if(location->is_array_pos()){
        std::string array_name("");
        if(is_att_from_act_obj){
            // This is an attribute from the actual object. Complete the 
            // identifier with "this".
            ids = reference_list(ids);
            ids.insert(ids.begin(), this->this_unique_id);
        }

        // Evaluate the index.
        this->expr_call_appropriate_accept(location->get_array_idx_expr());
        // We have the value of the index in this->temp.
        index = this->temp;

        // Increment the index, by the size of the array.
        symtable_element *array = this->dereference(ids);
        unsigned int size = this->calculate_size(array->get_type(),
                                        std::string(""));
        
        inst_list->push_back(new_binary_assign(index,
                                            index, 
                                            new_integer_constant(size), 
                                            quad_oper::TIMES));
        
        if(location->is_object_field() or is_att_from_act_obj){
            // Get the offset of the array, into the instance.
            unsigned int at_offset = this->get_attribute_offset(ids);

            // Add this offset to the previous.
            inst_list->push_back(new_binary_assign(index,
	                                            index, 
                                                new_integer_constant(at_offset), 
                                                quad_oper::PLUS));

            l_value = new_name_address(this->this_unique_id);
        }
        else{
            // {not location->is_object_field() and not is_att_from_act_obj}
            l_value = new_name_address(this->s_table.get_id_rep(ids[0]));
        }
    }
    else{
        // {not location->is_array_pos()}

        if(location->is_object_field() or is_att_from_act_obj){
            std::string object_name("");

            if(is_att_from_act_obj){
                // This is an attribute from the actual object. Complete the 
                // identifier with "this".
                ids = reference_list(ids);
                ids.insert(ids.begin(), this->this_unique_id);
                object_name = this->this_unique_id;
            }
            else{
                object_name = ids[0];            
            }

            // Get the offset of the attribute, into the instance.
            unsigned int at_offset = this->get_attribute_offset(ids);
            l_value = new_name_address(this->s_table.get_id_rep(object_name));
            index = new_integer_constant(at_offset);
        }
        else{
            // {not location->is_object_field() and not is_att_from_act_obj}
            std::string name = location->get_ids().printable_field_id();
            l_value = new_name_address(this->s_table.get_id_rep(name));
        }        
    }

    switch(node.oper){
        case AssignOper::ASSIGN:
            if(location->is_array_pos() or location->is_object_field() 
            or is_att_from_act_obj){
                
                inst_list->push_back(
                            new_indexed_copy_to(l_value,
                                                index, 
                                                r_value));
            }
            else{
                // {not location->is_array_pos() and 
                // not location->is_object_field() and 
                // not is_att_from_act_obj}
                inst_list->push_back(new_copy(l_value, r_value));
            }
            break;

        default:
            // Extract the value represented by location.
            this->expr_call_appropriate_accept(location);
            // We have the value of location in this->temp.
            address_pointer location_value = this->temp;

            quad_oper op;
            
            switch(node.oper){
                case AssignOper::PLUS_ASSIGN:
                    op = quad_oper::PLUS;
                    break;

                default:
                    // {AssignOper::MINUS_ASSIGN}
                    op = quad_oper::MINUS;
            }
            
            if(location->is_array_pos() or location->is_object_field() 
            or is_att_from_act_obj){
                
                // Save the increment into a temporal variable.
                t_results pair =  this->s_table.new_temp(this->offset);

	            #ifdef __DEBUG
		            assert(std::get<0>(pair) == put_results::ID_PUT);
	            #endif
                address_pointer new_temp = new_name_address(*std::get<1>(pair));

                inst_list->push_back(
                            new_binary_assign(new_temp,
                                                location_value, 
                                                r_value,
                                                op));
                // TODO:Update the offset.
                //this->offset += 
                
                // Finally, put the result into location.
                inst_list->push_back(
                            new_indexed_copy_to(l_value,
                                                index, 
                                                new_temp));
                
            }
            else{
                // {not location->is_array_pos() and 
                // not location->is_object_field() and 
                // not is_att_from_act_obj}
                inst_list->push_back(
                            new_binary_assign(l_value,
                                                location_value, 
                                                r_value,
                                                op));
            }
    }
}

void inter_code_gen_visitor::visit(node_location& node) {
	#ifdef __DEBUG
		std::cout << "Translating location expression" << std::endl;
	#endif

    // TODO: agregar tests para esto que estamos haciendo!!!!
    bool is_att_from_act_obj = node.get_is_attribute_from_actual_object();
    address_pointer reference = nullptr;
    address_pointer index = nullptr;

    if(node.is_array_pos()){
        // Evaluate the index.
        this->expr_call_appropriate_accept(node.get_array_idx_expr());
        // We have the value of the index in this->temp.
        index = this->temp;

        // Compute the offset, by taking into the account the size
        // of each field of the array.
        // TODO: no tengo que sumarle el espacio de la referencia inicial?
        symtable_element *array = this->dereference(node.get_ids());
        int field_size = this->calculate_size(array->get_type(),
                                                std::string(""));
        address_pointer address_field_size = new_integer_constant(field_size);            
        inst_list->push_back(new_binary_assign(index,
		                                        index, 
                                                address_field_size, 
                                                quad_oper::TIMES));
        reference_list ids = node.get_ids();

        // Determine if it is an instance's attribute.
        if(node.is_object_field() or is_att_from_act_obj){
            if(is_att_from_act_obj){
                // This is an attribute from the actual object. Complete the 
                // identifier with "this".
                ids = reference_list(ids);
                ids.insert(ids.begin(), this->this_unique_id);
            }

	        std::string object_name = ids[0];
            // Get the offset of the array, into the instance.
	        unsigned int at_offset = this->get_attribute_offset(ids);
            
            // Add this offset to the previous.
            inst_list->push_back(new_binary_assign(index,
		                                        index, 
                                                new_integer_constant(at_offset), 
                                                quad_oper::PLUS));

            std::string object_unique_id = this->s_table.get_id_rep(object_name);
            reference = new_name_address(object_unique_id);
        }
        else{
            // {not node.is_object_field() and not is_att_from_act_obj}
            reference = new_name_address(this->s_table.get_id_rep(ids[0]));
        }
        // Save into this->temp the result of the array indexing.
        t_results pair =  this->s_table.new_temp(this->offset);

        #ifdef __DEBUG
	        assert(std::get<0>(pair) == put_results::ID_PUT);
        #endif
        this->temp = new_name_address(*std::get<1>(pair));
        // We use space to save a new temporal. Update this->offset.
        this->offset += field_size;
        
        inst_list->push_back(
			            new_indexed_copy_from(this->temp,
                                                reference,
								                index));
    }
    else{
        // {not node.is_array_pos()}
        if(node.is_object_field() or is_att_from_act_obj){
		    reference_list ids = node.get_ids();

            if(is_att_from_act_obj){
                // This is an attribute from the actual object. Complete the 
                // identifier with "this".
                ids = reference_list(ids);
                ids.insert(ids.begin(), this->this_unique_id);
            }
		    std::string object_name = ids[0];
		    unsigned int at_offset = this->get_attribute_offset(ids);
            t_results pair =  this->s_table.new_temp(this->offset);
	        #ifdef __DEBUG
		        assert(std::get<0>(pair) == put_results::ID_PUT);
	        #endif

            this->temp = new_name_address(*std::get<1>(pair));
            reference = new_name_address(this->s_table.get_id_rep(
                                                object_name));
            index = new_integer_constant(at_offset);
		    inst_list->push_back(
				    new_indexed_copy_from(this->temp,
                                            reference,
									    index));
            // We use space to save a new temporal. Update this->offset.
            symtable_element *element = this->dereference(node.get_ids());        
            this->offset += this->calculate_size(element->get_type(),
                                                std::string(""));
            
        }
        else{
            // {not node.is_object_field() and not is_att_from_act_obj}
            std::string name = node.ids.printable_field_id();
            this->temp = new_name_address(this->s_table.get_id_rep(name));        
        }
	}
}

void inter_code_gen_visitor::visit(node_int_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating int literal with value " << node.value << std::endl;
	#endif

	this->temp = new_integer_constant(node.value);
}

void inter_code_gen_visitor::visit(node_bool_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating bool literal with value " << node.value << std::endl;
	#endif

	this->temp = new_boolean_constant(node.value);
}

void inter_code_gen_visitor::visit(node_float_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating FLOAT literal with value " << node.value << std::endl;
	#endif

	this->temp = new_float_constant(node.value);
}

void inter_code_gen_visitor::visit(node_string_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating string literal of value " << node.value << std::endl;
	#endif

	this->temp = new_string_constant(node.value);
}

void inter_code_gen_visitor::visit(node_method_call_expr& node) {
	#ifdef __DEBUG
			std::cout << "Translating method call statement" << std::endl;
	#endif
	// First parameter: "this" reference
	address_pointer this_param = nullptr;
    symtable_element *method = this->dereference(node.method_call_data->ids);

	if(node.method_call_data->ids.size() >= 2){
		// It is a call of the form obj1. ... .objn.method()
		// Then, this = objn. Obtain the symtable_element that represents objn.
        reference_list ids_objn = reference_list(node.method_call_data->ids);
        ids_objn.pop_back();
        symtable_element *objn = this->dereference(ids_objn);
		this_param = new_name_address(this->s_table.get_id_rep(objn->get_key()));
	}
	else{
		// {node.method_call_data->ids.size() < 2}
		// It is a call of the form method()
		// Then, this = actual object.
		this_param = new_name_address(this->this_unique_id);
	}

	// Add the "this" reference as parameter.
	inst_list->push_back(new_parameter_inst(this_param));

	// Add the remaining parameters.
	for(auto r : node.method_call_data->parameters){
		this->expr_call_appropriate_accept(r);
		inst_list->push_back(new_parameter_inst(this->temp));
	}

    // Create a temporal variable to store the result.
    t_results temp_pair =  this->s_table.new_temp(this->offset);
    #ifdef __DEBUG
        assert(std::get<0>(temp_pair) == put_results::ID_PUT);
    #endif
	this->temp = new_name_address(*std::get<1>(temp_pair));

	std::string method_label = this->obtain_methods_label(
                                    node.method_call_data->ids);
	inst_list->push_back(
                new_function_call_inst(
                    this->temp, 
                    new_label_address(method_label),
                    new_integer_constant(
                        node.method_call_data->parameters.size())));

    // Update the offset.
    this->offset += this->calculate_size(method->get_type(), std::string(""));
}

void inter_code_gen_visitor::visit(node_method_call_statement& node) {
	#ifdef __DEBUG
			std::cout << "Translating method call statement" << std::endl;
	#endif

	// First parameter: "this" reference
	address_pointer this_param = nullptr;
	if(node.method_call_data->ids.size() >= 2){
		// It is a call of the form obj1. ... .objn.method()
		// Then, this = objn. Obtain the symtable_element that represents objn.
        reference_list ids_objn = reference_list(node.method_call_data->ids);
        ids_objn.pop_back();
        symtable_element *objn = this->dereference(ids_objn);
		this_param = new_name_address(this->s_table.get_id_rep(objn->get_key()));
	}
	else{
		// {node.method_call_data->ids.size() < 2}
		// It is a call of the form method()
		// Then, this = actual object.
		this_param = new_name_address(this->this_unique_id);
	}
	inst_list->push_back(new_parameter_inst(this_param));
	// Remaining parameters.
	for(auto r : node.method_call_data->parameters){
		this->expr_call_appropriate_accept(r);
		this->inst_list->push_back(new_parameter_inst(this->temp));
	}
	std::string method_label = this->obtain_methods_label(node.method_call_data->ids);
	inst_list->push_back(new_procedure_call_inst(new_label_address(method_label),
						new_integer_constant(node.method_call_data->parameters.size()+1)));
}

void inter_code_gen_visitor::visit(node_if_statement& node){
	#ifdef __DEBUG
			std::cout << "Translating if statement" << std::endl;
	#endif
	this->expr_call_appropriate_accept(node.expression);

    t_results label_else_pair =  s_table.new_temp(this->offset);
	std::string label_else(*std::get<1>(label_else_pair));
	inst_list->push_back(new_conditional_jump_inst(this->temp,
													new_label_address(label_else),
													quad_oper::IFFALSE));
	// Code for the "if" branch.
	stm_call_appropriate_accept(node.then_statement);
	// Unconditional jump to the final part of the translation.
    t_results label_end_pair =  s_table.new_temp(this->offset);
	std::string label_end(*std::get<1>(label_end_pair));	
	inst_list->push_back(new_unconditional_jump_inst(
												new_label_address(label_end)));

	// Code for the "else" branch.
	inst_list->push_back(new_label_inst(new_label_address(label_else)));
	if(node.else_statement != nullptr) {
		stm_call_appropriate_accept(node.else_statement);
	}
	// Ending.
	inst_list->push_back(new_label_inst(new_label_address(label_end)));
}

void inter_code_gen_visitor::visit(node_for_statement& node){
	#ifdef __DEBUG
			std::cout << "Translating for statement" << std::endl;
	#endif

	this->expr_call_appropriate_accept(node.from);
    // The variable node.id is local to the loop.
	address_pointer var = new_name_address(node.id);
	inst_list->push_back(new_copy(var, temp));

    t_results label_beg_pair =  s_table.new_temp(this->offset);
	std::string label_beginning(*std::get<1>(label_beg_pair));
	inst_list->push_back(new_label_inst(new_label_address(label_beginning)));

	/*inst_list->push_back("LTE x to z");
	inst_list->push_back("IFFALSE z L2");*/

    t_results label_ending_pair =  s_table.new_temp(this->offset);
	std::string label_ending(*std::get<1>(label_ending_pair));
	this->expr_call_appropriate_accept(node.to);
	// if temp < var goto label_ending
	inst_list->push_back(new_relational_jump_inst(this->temp, 
                                                    var, 
                                                    quad_oper::LESS,
						                            new_label_address(label_ending)));

	// Body code.
	stm_call_appropriate_accept(node.body);

	// Increment.
	address_pointer inc = new_integer_constant(1);
	inst_list->push_back(new_binary_assign(var, var, inc, quad_oper::PLUS));

	// Return to the beginning
	inst_list->push_back(new_unconditional_jump_inst(
											new_label_address(label_beginning)));

	// Ending.
	inst_list->push_back(new_label_inst(new_label_address(label_ending)));
}

void inter_code_gen_visitor::visit(node_while_statement& node) {
	#ifdef __DEBUG
		std::cout << "Translating while statement" << std::endl;
	#endif
	/*
	 *  LABEL L1, (EXPRESSION CODE, it returns t1), IFFALSE t1 L2, (BODY CODE),
	 *  GOTO L1, LABEL L2
	 * */
    t_results label_beg_pair =  s_table.new_temp(this->offset);
	std::string label_beginning(*std::get<1>(label_beg_pair));
	inst_list->push_back(new_label_inst(new_label_address(label_beginning)));

	// Evaluate the guard. Its value is in temp.
	this->expr_call_appropriate_accept(node.expression);

	// If the guard is false, jump to the end of the code.
    t_results label_end_pair =  s_table.new_temp(this->offset);
	std::string label_ending(*std::get<1>(label_end_pair));
	inst_list->push_back(new_conditional_jump_inst(temp,
										new_label_address(label_ending),
										quad_oper::IFFALSE));

	// Body.
	stm_call_appropriate_accept(node.body);

	// Return to the beginning.
	inst_list->push_back(new_unconditional_jump_inst(
										new_label_address(label_beginning)));

	// The end of the loop.
	inst_list->push_back(new_label_inst(new_label_address(label_ending)));
}

void inter_code_gen_visitor::visit(node_return_statement& node) {
	#ifdef __DEBUG
		std::cout << "Translating return statement" << std::endl;
	#endif

	// Evaluate the returned value. Its value is in temp.
	this->expr_call_appropriate_accept(node.expression);

	inst_list->push_back(new_return_inst(temp));
}

void inter_code_gen_visitor::visit(node_break_statement& node) {
	#ifdef __DEBUG
		std::cout << "Translating break statement" << std::endl;
	#endif

	// Break is translated as an unconditional jump to the end of the loop.
	inst_list->push_back(new_unconditional_jump_inst(
										new_label_address(std::string("L2"))));
}

void inter_code_gen_visitor::visit(node_continue_statement& node) {
	#ifdef __DEBUG
		std::cout << "Translating continue statement" << std::endl;
	#endif
	// Continue is translated as an unconditional jump to the beginning of the loop.
	inst_list->push_back(new_unconditional_jump_inst(
										new_label_address(std::string("L1"))));
}

void inter_code_gen_visitor::visit(node_skip_statement& node) {
	#ifdef __DEBUG
		std::cout << "Translating skip statement" << std::endl;
	#endif
}

void inter_code_gen_visitor::visit(node_binary_operation_expr& node) {
	quad_oper oper;
	address_pointer right_operand = nullptr;
	address_pointer left_operand = nullptr;
	address_pointer dest = nullptr;

	#ifdef __DEBUG
		std::cout << "Translating binary operation" << std::endl;
	#endif

	// Evaluate the operands.
	this->expr_call_appropriate_accept(node.left);
	left_operand = this->temp;

	switch(node.oper){
		case Oper::TIMES:
			oper = quad_oper::TIMES;
			break;

		case Oper::PLUS:
			oper = quad_oper::PLUS;
			break;

		case Oper::MINUS:
			oper = quad_oper::MINUS;
			break;

		case Oper::DIVIDE:
			oper = quad_oper::DIVIDE;
			break;

		case Oper::MOD:
			oper = quad_oper::MOD;
			break;

		case Oper::LESS:
			oper = quad_oper::LESS;
			break;

		case Oper::LESS_EQUAL:
			oper = quad_oper::LESS_EQUAL;
			break;

		case Oper::GREATER:
			oper = quad_oper::GREATER;
			break;

		case Oper::GREATER_EQUAL:
			oper = quad_oper::GREATER_EQUAL;
			break;

		case Oper::EQUAL:
			oper = quad_oper::EQUAL;
			break;

		case Oper::DISTINCT:
			oper = quad_oper::DISTINCT;
			break;

        default:{
            // {node.oper == Oper::AND or node.oper = Oper::OR}
            // This operators have a "short-circuit" semantics.
            t_results label_pair =  s_table.new_temp(this->offset);
	        #ifdef __DEBUG
		        assert(std::get<0>(label_pair) == put_results::ID_PUT);
	        #endif
			
            bool short_circuit_value;

            switch(node.oper){
                case Oper::AND:
                    short_circuit_value = false;
                    break;

                default:
                    // {node.oper == Oper::OR}
                    short_circuit_value = true;
            }

            // Determine if left_operand is short_circuit_value.
            inst_list->push_back(
                new_relational_jump_inst(left_operand,
                                        new_boolean_constant(short_circuit_value),
									    quad_oper::EQUAL,
									    new_label_address(*std::get<1>(label_pair))));
            // If that is not the case, evaluate the right operand.
            this->expr_call_appropriate_accept(node.right);
            inst_list->push_back(new_copy(left_operand, this->temp));

            inst_list->push_back(
                        new_label_inst(
                            new_label_address(*std::get<1>(label_pair))));
            
            this->temp = left_operand;
        }
	}

    if(node.oper != Oper::AND and node.oper != Oper::OR){
        this->expr_call_appropriate_accept(node.right);
	    right_operand = this->temp;
        // New temporal for the result.
	    t_results pair =  this->s_table.new_temp(this->offset);

	    #ifdef __DEBUG
		    assert(std::get<0>(pair) == put_results::ID_PUT);
	    #endif

	    dest = new_name_address(*std::get<1>(pair));
 
	    inst_list->push_back(new_binary_assign(dest,
			       left_operand, right_operand, oper));

        // Update offset.
	    this->offset += this->calculate_size(
			    this->determine_type(node.get_type().type),
			    std::string(""));
    }

	temp = dest;
}

void inter_code_gen_visitor::visit(node_negate_expr& node) {
	address_pointer dest = nullptr;

	#ifdef __DEBUG
		std::cout << "Translating negate expression" << std::endl;
	#endif

	// Evaluate the operand.
	this->expr_call_appropriate_accept(node.expression);

	// New temporal for the result.
	t_results pair =  s_table.new_temp(this->offset);

	#ifdef __DEBUG
		assert(std::get<0>(pair) == put_results::ID_PUT);
	#endif

	dest = new_name_address(*std::get<1>(pair));
	inst_list->push_back(new_unary_assign(dest, temp, quad_oper::NEGATION));

	// Update offset.
	this->offset += boolean_width;

	temp = dest;
}

void inter_code_gen_visitor::visit(node_negative_expr& node) {
	address_pointer dest = nullptr;

	#ifdef __DEBUG
		std::cout << "Translating negative expression" << std::endl;
	#endif
	// Evaluate the operand.
	this->expr_call_appropriate_accept(node.expression);

	// New temporal for the result.
	t_results pair =  s_table.new_temp(this->offset);

	#ifdef __DEBUG
		assert(std::get<0>(pair) == put_results::ID_PUT);
	#endif

	dest = new_name_address(*std::get<1>(pair));
	inst_list->push_back(new_unary_assign(dest, temp, quad_oper::NEGATIVE));

	// Update offset.
	this->offset += this->calculate_size(
						this->determine_type(node.get_type().type),
						std::string(""));

	temp = dest;
}

void inter_code_gen_visitor::visit(node_parentheses_expr& node) {
	address_pointer dest = nullptr;

	#ifdef __DEBUG
		std::cout << "Translating parentheses expression" << std::endl;
	#endif
	this->expr_call_appropriate_accept(node.expression);
}

