#include "semantic_analysis.h"

semantic_analysis::semantic_analysis () {
	errors = 0;
	into_for_or_while = false;
	into_method = false;
	actual_method = nullptr;
}

void semantic_analysis::register_error(std::string error, error_id error_encountered){
	std::cerr << "\n" << error << std::endl;
	// The process is not interrupted. We just count the error.
	error += 1;
	last_error = error_encountered;
}

semantic_analysis::error_id semantic_analysis::get_last_error(){
	return last_error;
}

int semantic_analysis::get_errors(){
	return errors;
}

symtable_element::id_type semantic_analysis::get_wider_type(
symtable_element::id_type t1, symtable_element::id_type t2){

	symtable_element::id_type ret;

	#ifdef __DEBUG
		assert(t1 == symtable_element::FLOAT || t1 == symtable_element::INTEGER);
		assert(t2 == symtable_element::FLOAT || t2 == symtable_element::INTEGER);
	#endif

	switch(t1){
		case symtable_element::INTEGER:
			if (t2 == symtable_element::INTEGER)
				ret = symtable_element::INTEGER;
			else
				ret = t2;
			break;

		case symtable_element::FLOAT:
			ret = t1;
			break;
	}

	return ret;
}

symtable_element::id_type semantic_analysis::determine_type(Type::_Type type_ast){
	symtable_element::id_type ret;

	switch(type_ast){
		case Type::INTEGER:
			ret = symtable_element::INTEGER;
			break;

		case Type::FLOAT:
			ret = symtable_element::FLOAT;
			break;

		case Type::BOOLEAN:
			ret = symtable_element::BOOLEAN;
			break;

		case Type::VOID:
			ret = symtable_element::VOID;
			break;

		case Type::ID:
			ret = symtable_element::ID;
			break;

		/*case Type::STRING:
			ret = symtable_element::STRING;
			break;*/

		#ifdef __DEBUG
		default:
			assert(false);
		#endif
	}

	return ret;
}

symtable_element* semantic_analysis::get_next_symtable_element(symtable_element
*actual_element, std::string id){
	#ifdef __DEBUG
		// PRE
		assert(actual_element->get_class() == symtable_element::T_CLASS);
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

symtable_element* semantic_analysis::dereference(reference_list ids){
	#ifdef __DEBUG
		// PRE
		assert(s_table.size() > 0);
		std::cout << "Accessing reference " << ids[0] << std::endl;
	#endif

	symtable_element *ret = nullptr;
	symtable_element *aux = s_table.get(ids[0]);
	symtable_element::id_class aux_class;
	// Rule 2: declaration before use
	if (aux->get_class() == symtable_element::NOT_FOUND){
		register_error(std::string("Id "+ids[0]+" must be declared before use."),
				ERROR_2);
		well_formed = false;
	}
	else{
		// {aux->get_class() != symtable_element::NOT_FOUND}

		std::vector<int>::size_type i = (std::vector<int>::size_type) 1;
		std::vector<int>::size_type limit = ids.size();
		// INV : {aux is the symtable_element for the id number i-1 in ids}
		while(i < limit){
			#ifdef __DEBUG
				std::cout << "Accessing reference " << ids[i] << std::endl;
			#endif

			// Index the previous symtable_element, with the next id.
			aux_class = aux->get_class();
			if(aux_class != symtable_element::T_OBJ){
				register_error(std::string("Trying to access a field from id "+ids[i-1]
				                           +", which is not an object."),
								ERROR_21);
				well_formed = false;
				break;

			}
			else{
				// {aux_class == symtable_element::T_OBJ}
				// Get the class of the object.
				std::string *object_class_name = aux->get_class_type();
				symtable_element *object_class = s_table.get(*object_class_name);
				#ifdef __DEBUG
					assert(object_class->get_class() != symtable_element::NOT_FOUND);
				#endif
				// Obtain the next element.
				aux = get_next_symtable_element(object_class, ids[i]);
				// Rule 2: declaration before use
				if (aux == nullptr){
					register_error(std::string("Id "+ids[i]+" must be declared before use."),
							ERROR_2);
					well_formed = false;
					break;
				}
				i++;
			}
		}
	}
	// {aux is the symtable_element referenced by ids or nullptr.}
	if(aux != nullptr && aux->get_class() != symtable_element::NOT_FOUND){
		well_formed = true;
		ret = aux;
	}

	return ret;
}

void semantic_analysis::visit(const node_program& node) {
	#ifdef __DEBUG
	 	 std::cout << "Beginning analysis." << std::endl;
	#endif
	// Define a new block: the global scope
	s_table.push_symtable();
	for(auto c : node.classes) {
		c->accept(*this);
	}
	// {there must be 1 or 0 symbol tables in s_tables}

	// Rule 3: Every program has one class with name "main".
	if(node.classes.size() == 0 ||
	s_table.get(std::string("main"))->get_class() == symtable_element::NOT_FOUND){
		register_error(std::string("No \"main\" class declared."), ERROR_3);
	}
}

void semantic_analysis::visit(const node_class_decl& node) {
	#ifdef __DEBUG
		std::cout << "Accessing class " << node.id << std::endl;
	#endif
	symtable_element new_class(new std::string(node.id),
			new std::list<symtable_element>());
	actual_class = &new_class;

	// Define a new scope
	#ifdef __DEBUG
		symtables_stack::put_class_results ret = s_table.put_class(new_class.get_key(), new_class);
		assert(ret == symtables_stack::CLASS_PUT);
	#else
		s_table.put_class(new_class.get_key(), new_class);
	#endif

	for(auto cb : node.class_block) {
		if (cb->is_node_field_decl()){
			// TODO: por qué tiene que ser node_field_decl&?
			// TODO: utilizar static_pointer_cast:https://msdn.microsoft.com/es-AR/library/hh279669.aspx
			node_field_decl& aux = static_cast<node_field_decl&> (*cb);
			aux.accept(*this);
		}
		else{
			// {not cb->is_node_field_decl()}
			// TODO: por qué tiene que ser node_field_decl&?
			node_method_decl& aux = static_cast<node_method_decl&> (*cb);
			aux.accept(*this);
		}
	}
	// Rule 3: if the name of the class is main, then it should have
	// defined one method, with name main, with no parameters
	if (node.id == "main"){
		#ifdef __DEBUG
			std::cout << "Main class found." << std::endl;
		#endif
		symtable_element *element = s_table.get(std::string("main"));
		if(element->get_class() != symtable_element::T_FUNCTION){
			register_error(std::string("\"main\" class without a \"main\" method."),
					ERROR_3);
		}
	}
	// Close the scope introduced by the class
	#ifdef __DEBUG
		std::cout << "Finishing analysis of class " << node.id << std::endl;
	#endif
	s_table.finish_class_analysis();
	actual_class = nullptr;
}

void semantic_analysis::visit(const node_field_decl& node) {
	symtable_element *id = nullptr;

	#ifdef __DEBUG
		std::cout << "Accessing field declaration." << std::endl;
	#endif

	for(auto f : node.ids) {
		// Rule 1: this is the first time this identifier is declared in this
		// scope
		if(s_table.get(f->id)->get_class() != symtable_element::NOT_FOUND){
			register_error(std::string("Identifier "+f->id+" already declared in this scope."),
					ERROR_1);
			continue;
		}
		// Rule 4: if it is an array declaration => the length must be > 0
		if (f->array_size == 0){
			register_error(std::string("Array declaration with id " +
							f->id +" has dimension == 0."),
					ERROR_4);
			continue;
		}
		// Everything ok. Add the identifier to the symbol table.
		if (node.type.type == Type::ID){
			// Declaration of instances.
			// Create a new symtable_element object, that represents
			// an object, with the id and the name of the corresponding class.
			if(s_table.get(node.type.id)->get_class() != symtable_element::T_CLASS){
				register_error(std::string("Identifier "+f->id+" has unknown type "
								+ node.type.id + "."),
						ERROR_20);
				continue;
			}
			else{
				// {s_table.get(node.type.id)->get_class() ==
				//  symtable_element::T_CLASS}
				if (not into_method){
					// This is a field declaration.
					// Rule 24: the type of an attribute cannot be the class where
					// it belongs to.
					if(node.type.id == actual_class->get_key()){
						register_error(std::string("Attribute "+f->id+" has as type "
										"the class"+ node.type.id + "where it belongs to."),
												ERROR_24);
						continue;
					}

				}
				id = new symtable_element(f->id, new std::string(node.type.id));
			}
		}
		else{
			// {node.type.type != Type::ID}
			// Basic type or array
			if (f->array_size > 0){
				// Array
				id = new symtable_element(f->id, determine_type(node.type.type),
					  f->array_size);
			}
			else{
				// {f->array_size < 0}
				// Basic type
				id = new symtable_element(f->id, determine_type(node.type.type));
			}
		}
		// Add the symbol to s_table.
		if(not into_method){
			#ifdef __DEBUG
						assert(s_table.put_class_field(f->id, *id) == symtables_stack::FIELD_PUT);
			#else
						s_table.put_class_field(f->id, *id);
			#endif
		}
		else{
			#ifdef __DEBUG
						assert(s_table.put(f->id, *id) == symtables_stack::ID_PUT);
			#else
						s_table.put(f->id, *id);
			#endif
		}
	}
}

void semantic_analysis::visit(const node_id& node) {
	// This visitor does not need to do some task into node_id.
}

void semantic_analysis::visit(const node_method_decl& node){
	#ifdef __DEBUG
		std::cout << "Accessing method " << node.id << std::endl;
	#endif
	symtable_element method(node.id, determine_type(node.type.type),
							new std::list<symtable_element>());
	into_method = true;
	actual_method = &method;
	// Add to the symbol table, define a new scope
#ifdef __DEBUG
	assert(s_table.put_func(node.id, method) == symtables_stack::FUNC_PUT);
#else
	s_table.put_func(node.id, method);
#endif
	for(auto p : node.parameters) {
		p->accept(*this);
	}

	node.body->accept(*this);
	// Close the scope defined by the method.
	#ifdef __DEBUG
		std::cout << "Finishing analysis of method " << node.id << std::endl;
	#endif
	s_table.finish_func_analysis();
	into_method = false;
	actual_method = nullptr;
}

void semantic_analysis::visit(const node_parameter_identifier& node) {
	#ifdef __DEBUG
		std::cout << "Accessing parameter " << node.id << std::endl;
	#endif
	// We add the identifier to the actual symbol table
	if(node.type.type == Type::ID &&
	   s_table.get(node.type.id)->get_class() != symtable_element::T_CLASS){
		register_error(std::string("Identifier "+node.id+" has unknown type "
						+ node.type.id + "."),
				ERROR_20);
	}
	else{
		// {node.type.type != Type::ID ||
		//  s_table.get(node.type.id)->get_class() == symtable_element::T_CLASS}

		// Rule 23: Parameter's identifier and method's name must differ.
		if (node.id == actual_method->get_key()){
			register_error(std::string("Parameter's identifier and method's name "
					"coincide:" + node.id), ERROR_23);
		}
		symtable_element param(node.id, determine_type(node.type.type));
		s_table.put_func_param(node.id, param);
	}
}

void semantic_analysis::visit(const node_body& node) {
	#ifdef __DEBUG
		std::cout << "Accessing body of method" << std::endl;
	#endif

	if(node.is_extern){
		#ifdef __DEBUG
			std::cout << "The body is extern" << std::endl;
		#endif
		// TODO: cuando esté resuelto lo de symtable, indicar
		// que el método es externo (tenemos la referencia al
		// método actual en actual_method)
	}
	else {
		#ifdef __DEBUG
			std::cout << "The body is declared" << std::endl;
		#endif

		node.block->accept(*this);
	}
}

void semantic_analysis::visit(const node_block& node) {
#ifdef __DEBUG
	std::cout << "Accessing a block." << std::endl;
#endif
	// A block defines a new scope.
	s_table.push_symtable();
	// Analyze the content of the block.
	for(auto s : node.content) {
		stm_call_appropriate_accept(s);
	}
	s_table.pop_symtable();
}

void semantic_analysis::visit(const node_assignment_statement& node) {
	#ifdef __DEBUG
		std::cout << "Accessing assignment statement" << std::endl;
	#endif

	symtable_element::id_type type_id, type_expr;

	// Check and determine the type of the location.
	node.location->accept(*this);
	if (well_formed){
		type_id = type_l_expr;

		expr_call_appropriate_accept(node.expression);
		if (well_formed){
			type_expr = type_l_expr;
			// Rule 16: both, the type of the location and the expression to be assigned,
			// must be the same.
			if (type_id != type_expr){
				register_error(std::string("Location and expression assigned have different types."),
						ERROR_16);
			}
			// Rule 17: in a -= or += assignment, the location and the
			// expression assigned must evaluated to integer or float.
			if (node.oper == AssignOper::PLUS_ASSIGN ||
			node.oper == AssignOper::MINUS_ASSIGN){

				if((type_id != symtable_element::FLOAT &&
					type_id != symtable_element::INTEGER) ||
					(type_expr != symtable_element::FLOAT &&
					type_expr != symtable_element::INTEGER)){

					register_error(std::string("Location and expression assigned "
							"must evaluate to integer or float."),
											ERROR_17);
				}
			}
		}
		else{
			// {not well_formed}

		}
	}
	else{
		// {not well_formed}
	}
}

void semantic_analysis::visit(const node_location& node) {
	symtable_element::id_type type_index, type_id;
	symtable_element::id_class class_id;
	symtable_element *aux = nullptr;
	#ifdef __DEBUG
		std::cout << "Accessing location expression" << std::endl;
	#endif

	aux = dereference(node.ids);
	if(well_formed){
		type_id = aux->get_type();
		class_id = aux->get_class();
		if (node.array_idx_expr != nullptr){
			// The location represents an field from an array.
			if (class_id != symtable_element::T_ARRAY){
				// Rule 11: if the location is an array position,
				// the corresponding id must point to an array, and the
				// index must be an integer
				register_error(std::string("Trying to index a value that is not an array."),
										ERROR_11);
				well_formed = false;
			}
			else{
				// {class_id == symtable_element::T_ARRAY}
				expr_call_appropriate_accept(node.array_idx_expr);
				if(well_formed){
					// Rule 11: if the location is an array position,
					// the corresponding id must point to an array, and the
					// index must be an integer
					type_index = type_l_expr;
					if (type_index != symtable_element::INTEGER){
						register_error(std::string("Trying to index an array "
													"value without an integer "
													"expression."), ERROR_11);
						well_formed = false;
					}
					else{
						// {type_index == symtable_element::INTEGER}
						// The location references to an aeeay field, and is
						// well formed
						type_l_expr = type_id;
						class_l_expr = class_id;
					}
				}
			}
		}
		else{
			// {node.array_idx_expr == nullptr}
			// The location is well formed, and does not reference to an array.
			type_l_expr = type_id;
			class_l_expr = class_id;
		}
	}
}

void semantic_analysis::visit(const node_int_literal& node) {
#ifdef __DEBUG
	std::cout << "Accessing int literal of value " << node.value << std::endl;
#endif
	type_l_expr = symtable_element::INTEGER;
	well_formed = true;
}

void semantic_analysis::visit(const node_bool_literal& node) {
#ifdef __DEBUG
	std::cout << "Accessing bool literal of value " << node.value << std::endl;
#endif
	type_l_expr = symtable_element::BOOLEAN;
	well_formed = true;
}

void semantic_analysis::visit(const node_float_literal& node) {
#ifdef __DEBUG
	std::cout << "Accessing FLOAT literal of value " << node.value << std::endl;
#endif
	type_l_expr = symtable_element::FLOAT;
	well_formed = true;
}

void semantic_analysis::visit(const node_string_literal& node) {
#ifdef __DEBUG
	std::cout << "Accessing string literal of value " << node.value << std::endl;
#endif
	//TODO: falta symtable_element::STRING
	type_l_expr = symtable_element::FLOAT;
	well_formed = true;
}

void semantic_analysis::visit(const node_method_call_expr& node) {
	#ifdef __DEBUG
		std::cout << "Accessing method call expression" << std::endl;
	#endif
	analyze_method_call(*(node.method_call_data));
}

void semantic_analysis::visit(const node_method_call_statement& node) {
	#ifdef __DEBUG
		std::cout << "Accessing method call statement" << std::endl;
	#endif
	analyze_method_call(*(node.method_call_data));
}

void semantic_analysis::analyze_method_call(const method_call& data) {
	#ifdef __DEBUG
		std::cout << "Accessing method call statement" << std::endl;
	#endif

	symtable_element::id_type type_method, type_parameter;
	symtable_element *aux = nullptr;

	// Determine the type of the location
	aux = dereference(data.ids);
	if(well_formed){
		if(aux->get_class() == symtable_element::T_FUNCTION){
			// Rule 5: the number and type of the actual parameters,
			// must be the same of the formal parameters
			std::list<symtable_element>* func_params = aux->get_func_params();

			if(func_params->size() != data.parameters.size()){
				register_error(std::string("Method call with less parameters"
										"than expected."), ERROR_5);
				well_formed = false;
			}
			else{
				// {func_params->size() == data.parameters.size()}
				std::list<symtable_element>::iterator it = func_params->begin();
				for(auto r : data.parameters){
					expr_call_appropriate_accept(r);
					if(well_formed){
						if(it->get_type() != type_l_expr){
							register_error(std::string("Call to method with"
									"wrong argument's type."), ERROR_5);
							well_formed = false;
							break;
						}
						else{
							// {it->get_type == type_l_expr}

							// Rule 7: string literals only with extern methods.
							/*if(type_l_expr == symtable_element::STRING){
								// TODO: chequear que si el método que está en
								// en aux, es extern
							}*/
						}
					}
					else{
						// {not well_formed}
						break;
					}
					it++;
				}
				type_l_expr = aux->get_type();
				class_l_expr = aux->get_class();
				// Rule 6: if a method call is used as an expression, the method
				// must return some value. We just simply return the type of return
				// of the method.
			}
		}
		else{
			// {class_l_expr != symtable_element::T_FUNCTION}
			// Rule 21: Method call operation over...methods only.
			register_error(std::string("Method call operation over a value "
										"different than a method."), ERROR_22);
			well_formed = false;
		}
	}
}

void semantic_analysis::visit(const node_if_statement& node){
	expr_call_appropriate_accept(node.expression);
	if(well_formed){
		// Rule 12: the guard must be a boolean expression.
		if (type_l_expr != symtable_element::BOOLEAN){
			register_error(std::string("Non-boolean guard of an if expression ."),
							ERROR_12);
		}

		stm_call_appropriate_accept(node.then_statement);

		if(node.else_statement != nullptr) {
			stm_call_appropriate_accept(node.else_statement);
		}
	}
}

void semantic_analysis::visit(const node_for_statement& node){
	symtable_element::id_type type_expr;

	std::cout << "Accessing for statement" << std::endl;

	// Rule 18: from and to expression must evaluate to integers.
	expr_call_appropriate_accept(node.from);
	if(well_formed && type_l_expr!= symtable_element::INTEGER){
		register_error(std::string("Non-integer \"from\" expression of a for loop."),
				ERROR_18);
	}

	expr_call_appropriate_accept(node.to);
	if(well_formed && type_l_expr != symtable_element::INTEGER){
		register_error(std::string("Non-integer \"to\" expression of a for loop."),
				ERROR_18);
	}
	// To check rules that depend on this information...
	into_for_or_while = true;
	stm_call_appropriate_accept(node.body);
	into_for_or_while = false;
}

void semantic_analysis::visit(const node_while_statement& node) {
#ifdef __DEBUG
	std::cout << "Accessing while statement" << std::endl;
#endif

	expr_call_appropriate_accept(node.expression);
	if(well_formed && type_l_expr != symtable_element::BOOLEAN){
		register_error(std::string("Non-boolean guard of while loop."),
						ERROR_12);
	}
	// To check rules that depend on this information...
	into_for_or_while = true;
	stm_call_appropriate_accept(node.body);
	into_for_or_while = false;
}

void semantic_analysis::visit(const node_return_statement& node) {
	symtable_element::id_type type_method = actual_method->get_type();

#ifdef __DEBUG
	std::cout << "Accessing return statement" << std::endl;
#endif

	if(node.expression != nullptr) {
		if(type_method == symtable_element::VOID){
			// Rule 8: a return statement must have an associated expression
			// only if the method returns a value.
			register_error(std::string("Non-void returned value, from a method "
										"whose return type is void."), ERROR_8);
		}
		else{
			// {type_method != symtable_element::VOID}
			// Rule 9: the type of the value returned from the method must
			// be the same than the type of the expression of the return statement
			expr_call_appropriate_accept(node.expression);
			if(well_formed && type_l_expr != type_method){
				register_error(std::string("Method's return type differs from"
						"type of returned value."), ERROR_9);
			}
		}
	}
	else{
		// {node.expression == nullptr}
		if(type_method != symtable_element::VOID){
			// Rule 8.
			register_error(std::string("No value returned, from a method "
										"whose return type is not void."), ERROR_8);
		}
	}
}

void semantic_analysis::visit(const node_break_statement& node) {
#ifdef __DEBUG
	std::cout << "Accessing break statement" << std::endl;
#endif

	// Rule 19: break and continue, only into a loop's body.
	if (!into_for_or_while){
		register_error(std::string("Break statement outside a loop."),
				ERROR_19);
	}
}

void semantic_analysis::visit(const node_continue_statement& node) {
#ifdef __DEBUG
	std::cout << "Accessing continue statement" << std::endl;
#endif
	// Rule 19: break and continue, only into a loop's body.
	if (!into_for_or_while){
		register_error(std::string("Continue statement outside a loop."),
				ERROR_19);
	}
}

void semantic_analysis::visit(const node_skip_statement& node) {
#ifdef __DEBUG
	std::cout << "Accessing skip statement" << std::endl;
#endif
}

void semantic_analysis::visit(const node_binary_operation_expr& node) {
	symtable_element::id_type l_op_type, r_op_type;
	symtable_element::id_class l_op_class, r_op_class;

#ifdef __DEBUG
	std::cout << "Accessing binary operation" << std::endl;
#endif

	expr_call_appropriate_accept(node.left);
	if(well_formed){
		l_op_type = type_l_expr;
		l_op_class = class_l_expr;
		well_formed = false;
		expr_call_appropriate_accept(node.right);
		if(well_formed){
			r_op_type = type_l_expr;
			r_op_class = class_l_expr;

			// Rule 13: operands of arithmetic and relational operations,
			// must have type int or float.
			if (node.oper != Oper::EQUAL && node.oper != Oper::DISTINCT &&
				node.oper != Oper::AND && node.oper != Oper::OR){

				if ((l_op_type == symtable_element::INTEGER ||
				l_op_type == symtable_element::FLOAT) &&
				(r_op_type == symtable_element::INTEGER ||
				r_op_type == symtable_element::FLOAT)){

					well_formed = true;
					// TODO: hay conversión implícita de tipos?
					// TODO: determinar el tipo a retornar
					if (node.oper == Oper::LESS || node.oper == Oper::GREATER ||
					node.oper == Oper::LESS_EQUAL || node.oper == Oper::GREATER_EQUAL){
						type_l_expr = symtable_element::BOOLEAN;
					}
					else{
						type_l_expr = get_wider_type(l_op_type, r_op_type);
					}
				}
				else{
					register_error(std::string("Non-numeric operands of "
							"arithmetic or relational operation."), ERROR_13);
					well_formed = false;
				}
			}
			else{
				// {node.oper == Oper::EQUAL || node.oper == Oper::DISTINCT ||
				//  node.oper == Oper::AND || node.oper == Oper::OR}

				// Rule 14: eq_op operands must have the same type (int, float or boolean)
				if (node.oper == Oper::EQUAL || node.oper == Oper::DISTINCT){
					if (l_op_type != r_op_type ||
						(l_op_type != symtable_element::INTEGER &&
						 l_op_type != symtable_element::FLOAT &&
						 l_op_type != symtable_element::BOOLEAN)){

						register_error(std::string("eq_op operands with "
								"different or wrong types."), ERROR_14);
						well_formed = false;
					}
					else{
						well_formed = true;
						type_l_expr = symtable_element::BOOLEAN;
					}
				}
				else{
					// {node.oper == Oper::AND || node.oper == Oper::OR}
					// Rule 15: cond_op and ! operands, must evaluate to a boolean
					if (l_op_type != r_op_type || l_op_type != symtable_element::BOOLEAN){
						register_error(std::string("cond_op operands must "
								"evaluate to boolean."), ERROR_15);
						well_formed = false;
					}
					else{
						well_formed = true;
						type_l_expr = symtable_element::BOOLEAN;
					}
				}
			}
		}
	}
}

void semantic_analysis::visit(const node_negate_expr& node) {
#ifdef __DEBUG
	std::cout << "Accessing negate expression" << std::endl;
#endif

	expr_call_appropriate_accept(node.expression);
	if(well_formed){
		// Rule 15: cond_op and ! operands, must evaluate to a boolean.
		if (type_l_expr != symtable_element::BOOLEAN){
			register_error(std::string("! operand must evaluate to boolean."),
					ERROR_15);
			well_formed = false;
		}
	}
}

void semantic_analysis::visit(const node_negative_expr& node) {
	#ifdef __DEBUG
		std::cout << "Accessing negative expression" << std::endl;
	#endif
	expr_call_appropriate_accept(node.expression);

	if (well_formed && type_l_expr != symtable_element::INTEGER &&
	type_l_expr != symtable_element::FLOAT){
		// Rule 13: operands of arithmetic and relational operations,
		// must have type int or float
		register_error(std::string("Non-numeric operand for an arithmetic "
								"negation operation."), ERROR_13);
		well_formed = false;
	}
}

void semantic_analysis::visit(const node_parentheses_expr& node) {
#ifdef __DEBUG
	std::cout << "Accessing parentheses expression" << std::endl;
#endif

	expr_call_appropriate_accept(node.expression);
}
