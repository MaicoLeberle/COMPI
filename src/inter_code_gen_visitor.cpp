#include "inter_code_gen_visitor.h"

/* TODO: cosas que faltan:
 	 	 _ no estoy pudiendo escribir código intermedio en los casos de test
 	 	 de este visitor, ya que no puedo escribir a mano nombre de temporales:
 	 	 ahora las temp son de la forma @tn
 	 	 _ ver rutina de llamada a procedimiento: The contents of the rIP are
 	 	 not directly readable by software. However, the rIP is pushed onto the
 	 	 stack by a call instruction (página 21 del pdf. del manual de amd).
 	 	 _ punteros en assembly?
 	 	 _ definir una nueva instrucción de 3-direcciones que me indique
 	 	 que tengo que generar código para declarar un string.
 	 	 _ Agregar el tipo de las variables, al registrarlas en la tabla
 	 	 de símbolos, para que, cuando genero el código assembly, pueda saber
 	 	 si tengo o no un parametro integer, cosa de ponerlo en los registros,
 	 	 antes que en la pila.
*/
std::string printable_field_id(reference_list ids);

inter_code_gen_visitor::inter_code_gen_visitor() {
	inst_list = new instructions_list();
	this->offset = 0;
	into_method = false;
	next_temp = 1;
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

// TODO: este método está repetido en semantic_analysis
symtable_element::id_type inter_code_gen_visitor::determine_type(Type::_Type type_ast){
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

/* Returns the label that identifies the beginning of the method's translation
 * (in the intermediate code generated).
 * PARAM: ids: a reference_list that represents the identifier used for the call.
 * RETURNS: a std::string representing the label's content.
 * */
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
unsigned int inter_code_gen_visitor::calculate_size(symtable_element::id_type type){
	unsigned int ret = 0;

	switch(type){
		case symtable_element::INTEGER:
			ret = integer_width;
			break;

		case symtable_element::FLOAT:
			ret = float_width;
			break;

		case symtable_element::BOOLEAN:
			ret = boolean_width;
			break;

		case symtable_element::ID:
			// TODO: indexar la tabla de símbolos con el id, y extraer su tamaño.
			// TODO: notar que para este caso, sí es útil ir calculando un
			// offset de los atributos de una instancia, y guardar esa información
			// junto con la clase, para saber el tamaño que ocupa una instancia.
			break;
	}

	return ret;
}

// TODO: no es variable declaration: es variable definition!
/*	Translates a variable declaration, registers its information into the
 * 	table of symbols and updates the offset.
 * 	PRE : {the actual value of offset indicates where, the new variable,
 * 			must be stored}
 * */

void inter_code_gen_visitor::translate_var_decl(symtable_element::id_type type,
std::string id, std::string class_name, unsigned int array_length){

	// TODO: guardar el offset después de haberlo alterado: si x ocupa 4 bytes,
	// y es la primer variable, entonces comienza en -4 y llega hasta 0. Deberiamos
	// entonces restarle al offset,  en vez de sumarle?

	address_pointer dest, constant = nullptr;

	if(type == symtable_element::ID){
		std::string *class_name_aux = new std::string(class_name);
		symtable_element variable(id, class_name_aux);
		// TODO: notar que no le estamos pasando el último argumento. Se trata
		// supuestamente, del nombre del primer atributo.
		#ifdef __DEBUG
			t_results pair = s_table.put_obj(variable,
											id,
											this->offset,
											*class_name_aux,
											std::string(""));
			assert(std::get<0>(pair) == ID_PUT);
		#else
			s_table.put_var(variable, id, this->offset);
		#endif
		instance_initialization(class_name, id);
		// After instance_initialization, offset had been updated correctly.
	}
	else{
		if (array_length > 0){
			// Array.
			// Register the variable into the symbol's table.
			symtable_element variable(id, type, array_length);
			t_results pair = s_table.put_var(variable, id, this->offset);

			#ifdef __DEBUG
				assert(std::get<0>(pair) == ID_PUT);
			#endif

			// Initialize each position of the array with the corresponding
			// initial value.
			unsigned int arr_pos_width = this->calculate_size(type);
			unsigned int arr_size =  arr_pos_width*array_length;

			switch(type){
				case symtable_element::INTEGER:
					constant = new_integer_constant(integer_initial_value);
					break;

				case symtable_element::FLOAT:
					constant = new_float_constant(float_initial_value);
					break;

				case symtable_element::BOOLEAN:
					constant = new_boolean_constant(boolean_initial_value);
			}

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
		else {
			// {array_length <= 0}
			// Basic type
			switch(type){
				case symtable_element::INTEGER:
					constant = new_integer_constant(integer_initial_value);
					break;

				case symtable_element::FLOAT:
					constant = new_float_constant(float_initial_value);
					break;

				case symtable_element::BOOLEAN:
					constant = new_boolean_constant(boolean_initial_value);
			}
			dest = new_name_address(id);
			symtable_element variable(id, type);

			if(class_name != std::string("")){
				// Then it is an instance's attribute.
				inst_list->push_back(new_indexed_copy_to(dest,
									  new_integer_constant(this->offset),
									  constant));
			}
			else{
				// {class_name == std::string("")}
				// The variable defined is not an attribute of an instance.

				t_results pair = s_table.put_var(variable, id, this->offset);
				#ifdef __DEBUG
					assert(std::get<0>(pair) == ID_PUT);
				#endif

				inst_list->push_back(new_copy(dest, constant));
			}
			this->offset += calculate_size(type);


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
	symtable_element *object = s_table.get(*it);
	symtable_element *element = nullptr;
	std::string *class_name = object->get_class_type();
	it++;

	// INV : {class_name is the name of the class to which "it" is and
	//			attribute of}
	while(it != it_end){
		// TODO: por que es un puntero?
		at_offset += *s_table.get_offset(*it, *class_name);

		element = s_table.get(*it);

		if(element->get_class() == symtable_element::id_class::T_OBJ){
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
	// TODO: el último argumento a put_obj sería el id del primer
	// parámetro del objeto.
	// TODO: actualmente, no estaría haciendo falta usar first_field
	// Directamente guardamos un puntero al primer atributo, al
	// final de todos los atributos.

	// TODO: cambiar esto: el id de una instancia, debería apuntar al primer
	// atributo del objeto, pero no debería hacer falta almacenar esa referencia.
	// Sólo hay problemas cuando el objeto no tiene atributos: allí tenemos
	// que pensar cuál es la forma más razonable de definir qué almacenaría
	// el id del objeto.

	// Reset the offset, to compute new offsets, with respect to the beginning
	// of the instance.
	unsigned int prev_offset = this->offset;
	this->offset = 0;

	// Extract the beginning
	std::list<symtable_element>::iterator it = fields->begin();
	if(it != fields->end() and (*it).get_class() == symtable_element::T_VAR){
		first_field = id_instance + "." + (*it).get_key();
		symtable_element variable(first_field, (*it).get_class_type());
		t_results pair = s_table.put_var(variable, first_field, prev_offset);
		#ifdef __DEBUG
			assert(std::get<0>(pair) == ID_PUT);
		#endif
		// TODO: por ahora, no trabajamos con arreglos
		translate_var_decl((*it).get_type(), id_instance, id_class, 0);
		it++;
	}
	else{
		// {it == fields->end() or (*it).get_class() != symtable_element::T_VAR}
		// The class hasn't attributes.
		// TODO: estamos solamente almacenando una referencia....
		inst_list->push_back(new_unary_assign(new_name_address(id_instance),
												new_name_address(id_instance),
												quad_oper::ADDRESS_OF));
		this->offset += reference_width;
	}

	for(; it != fields->end(); ++it){
		if((*it).get_class() == symtable_element::T_VAR){
			//translate_var_decl((*it).get_type(), id_instance + "." + (*it).get_key(),
			//id_class, 0);
			translate_var_decl((*it).get_type(), id_instance, id_class, 0);
		}
	}
	inst_list->push_back(new_unary_assign(new_name_address(id_instance),
											new_name_address(first_field),
											quad_oper::ADDRESS_OF));
	this->offset = prev_offset + this->offset;
	// Update the symbol's table, with information about the instance.
	// The identifier of the instance will "point" to the first attribute.
	symtable_element id(id_instance, new std::string(id_class));
	t_results pair = s_table.put_obj(id,
									id_instance,
									this->offset,
									id_class,
									first_field);
	// TODO: al intentar recuperar el id del primer atributo,
	// recordar que parecieran estar guardándose estos en el orden
	// inverso a como uno los guarda en la tabla de símbolos.
	#ifdef __DEBUG
		assert(std::get<0>(pair) == ID_PUT);
	#endif

	this->offset += reference_width;
	/*if(first_field != std::string("")){
		// Save a pointer to the first attribute.
		inst_list->push_back(new_unary_assign(new_name_address(id_instance),
												new_name_address(first_field),
												quad_oper::ADDRESS_OF));
	}
	else{
		// {first_field == std::string("")}
		// The object hasn't attributes.
		inst_list->push_back(new_unary_assign(new_name_address(id_instance),
												new_name_address(id_instance),
												quad_oper::ADDRESS_OF));
	}*/

	return first_field;
}

/* _ TODO: con este método std::string get_id_rep(std::string); recupero el id
 * unico para una variable, de acuerdo al scope.
 * _ Recordar que la semántica de los operadores booleanos es short-circuit
 * (ver página 400 del dragon book)
 * */
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
	actual_class = &new_class;

	// Define a new scope
	s_table.push_symtable();
	#ifdef __DEBUG
		t_class_results ret = s_table.put_class(new_class, new_class.get_key());
		assert(std::get<0>(ret) == CLASS_PUT);
	#else
		s_table.put_class(new_class, new_class.get_key());
	#endif

	// Gather information about fields. Set relative addresses with respect to the
	// beginning of this class definition.
	// TODO: también al traducir la definición de los métodos?
	int prev_offset = this->offset;
	this->offset = 0;
	into_method = false;

	for(std::list<class_block_pointer>::iterator it = fields.begin();
		it != fields.end();
		++it){
			node_field_decl& aux = static_cast<node_field_decl&> (*(*it));
			aux.accept(*this);
	}
	// End of class declaration -> in offset we have the size of an instance.
	// TODO: tengo que guardar ese valor en algún lugar en la tabla, para poder
	// después recuperarlo, al momento de necesitar saber el tamaño que
	// ocupa una instancia, para atualizar el offset

	// Translation of methods.
	for(std::list<class_block_pointer>::iterator it = methods.begin();
		it != methods.end();++it){
			node_method_decl& aux = static_cast<node_method_decl&> (*(*it));
			aux.accept(*this);
	}
	// Restore offset value.
	this->offset = prev_offset;
	s_table.finish_class_analysis();
	//s_table.pop_symtable();
}

void inter_code_gen_visitor::visit(node_field_decl& node) {
	// TODO: podríamos utilizar Symbolic Type Widths (pagina 386, libro dragon)
	#ifdef __DEBUG
		std::cout << "Translating field declaration." << std::endl;
	#endif

	for(auto f : node.ids) {
		#ifdef __DEBUG
			std::cout << "Translating field " << f->id << std::endl;
		#endif

		// Translate each identifier. Add the identifier to the symbol table.
		if (node.type.type == Type::ID){
			// TODO: quizás haga falta agregar a las three-address un operador
			// que obtenga la "dirección de memoria" de una variable, para
			// indicar que almaceno un puntero a la instancia recién creada.

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
				// TODO: el último argumento sería el id del primer parámetro del objeto,
				// pero no tengo claro cómo definirlo, de forma tal que pueda reemplazar
				// a futuro toda ocurrencia del id de este objeto, por el id de su primer
				// atributo.
				// TODO: al intentar recuperar el id del primer atributo, recordar
				// que parecieran estar guardándose estos en el orden inverso
				// a como uno los guarda en la tabla de símbolos.
				#ifdef __DEBUG
					assert(std::get<0>(pair) == FIELD_PUT);
				#endif
			}
			this->offset += calculate_size(determine_type(node.type.type));
		}
		else{
			// {node.type.type != Type::ID}
			// Basic type or array
			if (f->array_size > 0){
				// Array
				if(into_method){
					// TODO: este tercer argumento es medio fulero...
					translate_var_decl(determine_type(node.type.type), f->id,
					std::string(""), f->array_size);
				}
				else{
					// {not into_method}

					symtable_element id(f->id,
										determine_type(node.type.type),
										f->array_size);
					// TODO: cambiar esto cuando se haga el typedef
					t_field_results pair = s_table.put_var_field(id,
																f->id,
																this->offset);
					#ifdef __DEBUG
						assert(std::get<0>(pair) == FIELD_PUT);
					#endif
					this->offset += calculate_size(determine_type(node.type.type))*f->array_size;
				}
			}
			else{
				// {f->array_size <= 0}
				// Basic type
				if(into_method){
					// TODO: este tercer argumento es medio fulero...
					translate_var_decl(determine_type(node.type.type), f->id,
										std::string(""), 0);
				}
				else{
					// {not into_method}
					symtable_element id(f->id, determine_type(node.type.type));

					#ifdef __DEBUG
						// TODO: cambiar esto cuando se haga el typedef
						t_field_results pair = s_table.put_var_field(id,
																	f->id,
																	this->offset);
						assert(std::get<0>(pair) == FIELD_PUT);
					#else
						s_table.put_var_field(id, f->id, this->offset);
					#endif

					this->offset += calculate_size(determine_type(node.type.type));
				}

			}
		}
	}
}

void inter_code_gen_visitor::visit(node_id& node) {
	// This visitor does not need to do some task into node_id.
}

void inter_code_gen_visitor::visit(node_method_decl& node){
	// TODO: el manejo de la referencia this, todavía no está definido cómo
	// hacerlo. Aquí hay posibles soluciones planteadas:
	//	* en la generación de código asm, los parámetros de las funciones
	// los pasamos a la memoria (al ingresar a la función). Por lo tanto,
	// mantenemos el offset de las variables.
	// 	* aquí debería agregar la variable "this" a la tabla de símbolos?
	// donde debería utilizar this en la traducción?
	//  * tengo que agregar como parámetros en las llamadas, a los
	// atributos del objeto para el cual se llama el método.
	#ifdef __DEBUG
		std::cout << "Translating method " << node.id << std::endl;
	#endif

	symtable_element method(node.id,
							determine_type(node.type.type),
							new std::list<symtable_element>());
	#ifdef __DEBUG
	    t_func_results pair =
	    		s_table.put_func(method,
						node.id,
						0,
						actual_class->get_key());
	    assert(std::get<0>(pair) == FUNC_PUT);
	#else
		//s_table.put_func_field(method, method.get_key(), 0, actual_class->get_key());
		s_table.put_func(method, node.id, 0, actual_class->get_key());
    #endif
	// Generate code for the method declaration.
	std::string *class_name = new std::string(this->actual_class->get_key());
	inst_list->push_back(new_label_inst(new_method_label_address(node.id,
																*class_name)));

	// Initialize offset.
	unsigned int offset_prev = this->offset;
	// TODO: usar la referencia this, para mejorar la legibilidad del código!

	// TODO: notar que la forma de modelar scope con código asm consiste:
	// 				_ en que aquí registremos parámetros, a los que le
	//				asignamos un offset fijo en el stack frame;
	//
	//				_ luego, que el código de llamada ubique los
	// 				parámetros en el stack en orden que se corresponda con este
	//				offset.
	//
	//				_ finalmente, el procedimiento asume estos offsets, y va
	//				a buscar ahí los parámetros.
	this->offset = 0;

	// TODO: borrar esto!!!!!!!!!!!!!
	// Add the "this" reference as first parameter.
	std::string this_param("this");
	symtable_element obj_param(this_param, class_name);
	//if(node.id != std::string("main")){
	#ifdef __DEBUG
		t_param_results pair1 = s_table.put_obj_param(obj_param,
													 this_param,
													 this->offset,
													 *class_name,
													 std::string(""));
		assert(std::get<0>(pair1) == PARAM_PUT);
	#else
		s_table.put_obj_param(obj_param, this_param, this->offset, *class_name,
							 std::string(""));
	#endif
	//}

	this->offset += reference_width;

	// Add the remaining parameters.
	for(auto p : node.parameters) {
		p->accept(*this);
	}
	// TODO: quizás estamos mezclando cuestiones de código asm aquí, pero
	// para simplificar, reiniciamos el cálculo del offset, cuando
	// vamos a analizar el cuerpo del método.
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
	// TODO: eliminar el contenido al que apunta inst_list
	inst_list = aux;
	this->offset = offset_prev;
	s_table.finish_func_analysis();
	#ifdef __DEBUG
		// TODO: esta era la forma adecuada de utilizar put_func_field,
		// aparentemente: primero put_func, agregamos los parametros de la misma
		// y recorremos el cuerpo de la misa. Cuando esta todo listo,
		// agregamos el metodo a la clase, con put_func_field

		t_field_results pair2 = s_table.put_func_field(method,
														method.get_key(),
														0,
														actual_class->get_key());
		assert(std::get<0>(pair2) == FIELD_PUT);
	#else
		s_table.put_func_field(method, method.get_key(), 0, actual_class->get_key());
    #endif
}

void inter_code_gen_visitor::visit(node_parameter_identifier& node) {
	/* Paso por parámetro los atributos de la clase, prefijandoles "self.". */
	#ifdef __DEBUG
		std::cout << "Translating parameter " << node.id << std::endl;
	#endif

	switch(node.type.type){
		case Type::ID:{
				std::string *class_name = new std::string(node.type.id);
				// TODO: la tabla de símbolos hace una copia de obj_param?
				// TODO: el último parámetro debería ser un string de la forma
				// node.id "." nombre del primer atributo de la clase
				symtable_element obj_param(node.id, class_name);

				#ifdef __DEBUG
					t_param_results pair1 = s_table.put_obj_param(obj_param,
																 node.id,
																 this->offset,
																 node.type.id,
																 std::string(""));
					assert(std::get<0>(pair1) == PARAM_PUT);
				#else
					s_table.put_obj_param(obj_param,
										node.id,
										this->offset,
										node.type.id,
										std::string(""));
				#endif
			}
			break;

		default:{
				// TODO: estoy asumiendo que un parámetro o bien es un objeto o bien
				// un tipo básico.
				symtable_element var_param(node.id, determine_type(node.type.type));
				#ifdef __DEBUG
					t_param_results pair2 = s_table.put_var_param(var_param,
																var_param.get_key(),
																this->offset);
					assert(std::get<0>(pair2) == PARAM_PUT);
				#else
					s_table.put_var_param(var_param,
											var_param.get_key(),
											this->offset);
				#endif
			}
	}
	// TODO: en la arquitectura x86_64, es el método el que construye
	// el stack frame?: "The following enter instruction
	// sets up the stack frame" (de "x86-64-architecture-guide"). Pero es quien
	// lo llama quien coloca los parametros en los registros. Debemos contabilizar
	// el tamaño de los parámetros?

	// Update the offset so, outside this method, this data is used to
	// calculate how much data is reserved into the stack.
	this->offset += calculate_size(determine_type(node.type.type));
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
	expr_call_appropriate_accept(node.get_expression());
	address_pointer r_value = temp;
	location_pointer location = node.get_location();

	if(node.oper == AssignOper::ASSIGN){
		if(location->is_object_field()){
			reference_list ids = location->get_ids();
			std::string object = ids[0];
			unsigned int at_offset = this->get_attribute_offset(ids);

			inst_list->push_back(
					new_indexed_copy_to(new_name_address(object),
										new_integer_constant(at_offset),
										r_value));
		}
		else{
			// {not location.is_object_field()}
			location->accept(*this);
			address_pointer l_value = temp;
			inst_list->push_back(new_copy(l_value, r_value));
		}
	}
	else{
		// {node.oper != AssignOper::ASSIGN}
		quad_oper oper;
		if(node.oper == AssignOper::PLUS_ASSIGN){
			oper = quad_oper::PLUS;
		}
		else{
			// {node.oper != AssignOper::PLUS_ASSIGN}
			oper = quad_oper::MINUS;
		}

		if(location->is_object_field()){
			// The l-value denotes an instance's attribute.
			reference_list ids = location->get_ids();
			std::string object_name = ids[0];
			address_pointer object_address = new_name_address(object_name);
			unsigned int at_offset = this->get_attribute_offset(ids);
			address_pointer at_offset_address = new_integer_constant(at_offset);

			// Extract the value of the attribute, and save it into a
			// temporal variable.
			t_results pair =  s_table.new_temp(this->offset);
			// Update the offset, taking into account the type of the
			// instance's attribute.
			this->offset += this->calculate_size(
								this->determine_type(
										(node.get_expression())->get_type().type));
			address_pointer temp_field = new_name_address(*std::get<1>(pair));

			inst_list->push_back(new_indexed_copy_from(temp_field,
														object_address,
														at_offset_address));

			// Update the value saved into the temporal.
			inst_list->push_back(new_binary_assign(temp_field,
													temp_field,
													r_value,
													oper));

			// Assign the result to the attribute.
			inst_list->push_back(new_indexed_copy_to(object_address,
													at_offset_address,
													temp_field));
		}
		else{
			// {not location.is_object_field()}
			location->accept(*this);
			address_pointer l_value = temp;
			inst_list->push_back(new_binary_assign(l_value,
													l_value,
													r_value,
													oper));
		}
	}
}

void inter_code_gen_visitor::visit(node_location& node) {
	#ifdef __DEBUG
		std::cout << "Translating location expression" << std::endl;
	#endif

	// TODO: por ahora, convertimos la location en un address. Es correcto
	// usar este nombre para la address?
	// TODO: preguntar si estamos dentro de un método. En tal caso, averiguar
	// si location se trata de un parámetro del metodo. Si no fuera así, se trata
	// de un atributo del objeto actual. Entonces tengo que tener en cuenta el
	// puntero this.
	std::string name = node.ids.printable_field_id();
	temp = new_name_address(name);
}

void inter_code_gen_visitor::visit(node_int_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating int literal with value " << node.value << std::endl;
	#endif
	// TODO: ver el código intermediate.c: no hace falta crear una temporal
	// para almacenar la constante. Basta con crear un objeto struct
	// address, que representa la cte (en este caso), o bien una dirección, o
	// bien una etiqueta, o bien una temporal.
	// TODO: deberían primero guardarse en una dirección temporal, para que, luego,
	// desde afuera, utilicemos la misma?. Pensar en el caso de la generación
	// de las instrucciones que las utilizan: si no se almacenara siempre en
	// en una dirección, entonces tendría que distinguir, al crear una
	// instrucción, si se trata de una constante o una dirección.
	//last_expr = std::string("$"+node.value)
	temp = new_integer_constant(node.value);
}

void inter_code_gen_visitor::visit(node_bool_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating bool literal with value " << node.value << std::endl;
	#endif
	// TODO: tengo que generar un address temporal, y ahí guardar este valor...
	temp = new_boolean_constant(node.value);
}

void inter_code_gen_visitor::visit(node_float_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating FLOAT literal with value " << node.value << std::endl;
	#endif

	temp = new_float_constant(node.value);
}

void inter_code_gen_visitor::visit(node_string_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating string literal of value " << node.value << std::endl;
	#endif
	// TODO: representación de literales string?
	// TODO: no tengo representación de este tipo de literales en el 3-address code
	//temp = std::string("$"+node.value);
}

void inter_code_gen_visitor::visit(node_method_call_expr& node) {
	#ifdef __DEBUG
			std::cout << "Translating method call statement" << std::endl;
	#endif
	// First parameter: "this" reference
	address_pointer this_param = nullptr;
	if(node.method_call_data->ids.size() >= 2){
		// It is a call of the form obj1. ... .objn.method()
		// Then, this = objn.
		std::string object_name = node.method_call_data->ids[node.method_call_data->ids.size()-2];
		this_param = new_name_address(object_name);
	}
	else{
		// {node.method_call_data->ids.size() < 2}
		// It is a call of the form method()
		// Then, this = actual object.
		// TODO: debería ser simplemente this?
		this_param = new_name_address(std::string("this"));
	}

	// Add the "this" reference as parameter.
	inst_list->push_back(new_parameter_inst(this_param));

	// Add the remaining parameters.
	for(auto r : node.method_call_data->parameters){
		expr_call_appropriate_accept(r);
		inst_list->push_back(new_parameter_inst(temp));
	}
	// TODO: generar una nueva dirección para almacenar el valor de retorno
	// actualmente es la variable dest
	address_pointer dest = nullptr;
	std::string method_label = obtain_methods_label(node.method_call_data->ids);
	inst_list->push_back(new_function_call_inst(dest, new_label_address(method_label),
						new_integer_constant(node.method_call_data->parameters.size())));
}

void inter_code_gen_visitor::visit(node_method_call_statement& node) {
	#ifdef __DEBUG
			std::cout << "Translating method call statement" << std::endl;
	#endif

	// First parameter: "this" reference
	address_pointer this_param = nullptr;
	if(node.method_call_data->ids.size() >= 2){
		// It is a call of the form obj1. ... .objn.method()
		// Then, this = objn.
		std::string object_name = node.method_call_data->ids[node.method_call_data->ids.size()-2];
		this_param = new_name_address(object_name);

	}
	else{
		// {node.method_call_data->ids.size() < 2}
		// It is a call of the form method()
		// Then, this = actual object.
		// TODO: debería ser simplemente this?
		this_param = new_name_address(std::string("this"));
	}
	inst_list->push_back(new_parameter_inst(this_param));
	// Remaining parameters.
	for(auto r : node.method_call_data->parameters){
		expr_call_appropriate_accept(r);
		inst_list->push_back(new_parameter_inst(temp));
	}
	std::string method_label = obtain_methods_label(node.method_call_data->ids);
	inst_list->push_back(new_procedure_call_inst(new_label_address(method_label),
						new_integer_constant(node.method_call_data->parameters.size()+1)));
}

void inter_code_gen_visitor::visit(node_if_statement& node){
	#ifdef __DEBUG
			std::cout << "Translating if statement" << std::endl;
	#endif
	expr_call_appropriate_accept(node.expression);
	// TODO: la visita a node.expression, guarda en un atributo de la instancia
	// el valor de la referencia (temp) en donde se almacena el
	// resultado de evaluar node.expression
	// TODO: L1 y L2 son 2 etiquetas de valores fijos, a definir
	// TODO: la intermediate_symtable va a generarme etiquetas únicas.
	// TODO: leer página 410 del libro Dragón: backpatching, para ver el temita
	// de cómo definir correctamente las etiquetas
	std::string label_else("L1");
	inst_list->push_back(new_conditional_jump_inst(temp,
													new_label_address(label_else),
													quad_oper::IFFALSE));
	// Code for the "if" branch.
	stm_call_appropriate_accept(node.then_statement);
	// Unconditional jump to the final part of the translation.
	std::string label_end("L2");
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

	expr_call_appropriate_accept(node.from);
	// TODO: var es la variable sobre la que itera el ciclo, temp es la dirección
	// en donde se almacenó la evaluación de node.from

	// The variable node.id is local to the loop.
	address_pointer var = new_name_address(node.id);
	inst_list->push_back(new_copy(var, temp));

	// TODO: L1 es una etiqueta a definir, de valor fijo.
	std::string label_beginning("L1");
	inst_list->push_back(new_label_inst(new_label_address(label_beginning)));

	// TODO: lo siguiente no hace falta, ya que en nuestro three-address code,
	// tenemos directamente una instrucción que realiza una comparación de orden
	// y salta a una posición etiquetada

	/*// TODO: temp es el lugar en donde se almacenó la evaluación de node.to, y
	// z es al lugar en donde se almacenará el resultado de la comparación
	inst_list->push_back("LTE x to z");
	// TODO: L2 es una etiqueta a definir, de valor fijo.
	inst_list->push_back("IFFALSE z L2");*/

	std::string label_ending("L2");
	expr_call_appropriate_accept(node.to);
	// if temp < var goto label_ending
	inst_list->push_back(new_relational_jump_inst(temp, var, quad_oper::LESS,
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
	// TODO: podríamos colocar como comentarios de todos los métodos el código
	// intermedio que nos inventamos.
	// TODO: el valor de esta etiqueta podría cambiar
	std::string label_beginning("L1");
	inst_list->push_back(new_label_inst(new_label_address(label_beginning)));

	// Evaluate the guard. Its value is in temp.
	expr_call_appropriate_accept(node.expression);

	// If the guard is false, jump to the end of the code.
	std::string label_ending("L2");
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
	expr_call_appropriate_accept(node.expression);

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
	expr_call_appropriate_accept(node.left);
	left_operand = temp;

	expr_call_appropriate_accept(node.right);
	right_operand = temp;

	// New temporal for the result.
	t_results pair =  s_table.new_temp(this->offset);
	// TODO: chequear put_results
	// TODO: no estoy usando new_temp_address
	// TODO: notar que aquí habría ayudado que semantic_analysis guarde en el node
	// el tipo que le asigno a la expresión, para actualizar el offset
	// como corresponde.
	dest = new_name_address(*std::get<1>(pair));

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

		case Oper::AND:
			// TODO: nuestro and es short-circuit
			break;

		case Oper::OR:
			// TODO: nuestro or es short-circuit
			break;
	}

	inst_list->push_back(new_binary_assign(dest,
			   left_operand, right_operand, oper));

	temp = dest;
}

void inter_code_gen_visitor::visit(node_negate_expr& node) {
	address_pointer dest = nullptr;

	#ifdef __DEBUG
		std::cout << "Translating negate expression" << std::endl;
	#endif

	// Evaluate the operand.
	expr_call_appropriate_accept(node.expression);

	// New temporal for the result.
	t_results pair =  s_table.new_temp(this->offset);
	// TODO: chequear put_results
	// TODO: no estoy usando new_temp_address
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
	expr_call_appropriate_accept(node.expression);

	// New temporal for the result.
	t_results pair =  s_table.new_temp(this->offset);
	// TODO: chequear put_results
	// TODO: no estoy usando new_temp_address
	dest = new_name_address(*std::get<1>(pair));
	inst_list->push_back(new_unary_assign(dest, temp, quad_oper::NEGATIVE));

	// Update offset.
	// TODO: aquí haría falta que el analizador semántico me indique el tipo
	// de la expresión.

	temp = dest;
}

void inter_code_gen_visitor::visit(node_parentheses_expr& node) {
	address_pointer dest = nullptr;

	#ifdef __DEBUG
		std::cout << "Translating parentheses expression" << std::endl;
	#endif

	// TODO: no debemos hacer nada?. Ya en el análisis sintáctico, al momento
	// de reconocer las expresiones parentizadas, se impone el orden adecuado
	// de evaluación.
	expr_call_appropriate_accept(node.expression);

	/*// New temporal for the result.
	t_results pair =  s_table.new_temp(offset);
	// TODO: chequear put_results
	// TODO: no estoy usando new_temp_address
	dest = new_name_address(*std::get<1>(pair));

	inst_list->push_back(new_copy(dest, temp));

	temp = dest;

	// Update offset.
	// TODO: aquí haría falta que el analizador semántico me indique el tipo
	// de la expresión.*/
}

