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
	offset = 0;
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

/*	Translates a variable declaration, registers its information into the
 * 	table of symbols and updates the offset.
 * 	PRE : {the actual value of offset indicates where, the new variable,
 * 			is stored}
 * */

void inter_code_gen_visitor::translate_var_decl(symtable_element::id_type type,
std::string id, std::string class_name, unsigned int array_size){

	// TODO: guardar el offset después de haberlo alterado: si x ocupa 4 bytes,
	// y es la primer variable, entonces comienza en -4 y llega hasta 0. Deberiamos
	// entonces restarle al offset,  en vez de sumarle?

	address_pointer dest, constant = nullptr;

	if(type == symtable_element::ID){
		// TODO: por qué class_name tiene que pasarse por puntero?
		std::string *class_name_aux = new std::string(class_name);
		symtable_element variable(id, class_name_aux);
		// TODO: notar que no le estamos pasando el último argumento. Se trata
		// supuestamente, del nombre del primer atributo.
		#ifdef __DEBUG
			t_results pair = s_table.put_obj(variable,
											id,
											offset,
											*class_name_aux,
											std::string(""));
			assert(std::get<0>(pair) == ID_PUT);
		#else
			s_table.put_var(variable, id, offset);
		#endif
		instance_initialization(class_name, id);
		// After instance_initialization, offset had been updated correctly.
	}
	else{
		if (array_size > 0){
			// Array
			// TODO: cómo inicializamos un arreglo?

			symtable_element variable(id, type, array_size);
			t_results pair = s_table.put_var(variable, id, offset);
			offset += calculate_size(type)*array_size;
			#ifdef __DEBUG
				assert(std::get<0>(pair) == ID_PUT);
			#endif

			dest = new_name_address(id);
			inst_list->push_back(new_copy(dest, constant));
		}
		else {
			// {f->array_size <= 0}
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

			symtable_element variable(id, type);
			t_results pair = s_table.put_var(variable, id, offset);
			offset += calculate_size(type);
			#ifdef __DEBUG
				assert(std::get<0>(pair) == ID_PUT);
			#endif

			dest = new_name_address(id);
			inst_list->push_back(new_copy(dest, constant));
		}
	}
}

std::string inter_code_gen_visitor::instance_initialization(std::string id_class,
std::string id_instance){
	// Translate instance declaration into a sequence of code to
	// initialize each attribute.
	symtable_element *class_object = s_table.get(id_class);
	std::list<symtable_element> *fields = class_object->get_class_fields();
	std::string first_field;

	// TODO: así es como recuperamos los atributos de instancia:
	// classA obj1;
	/*[4:06:52 PM] Maico Leberle: classB {
	    classA obj1;
	}
	[4:08:46 PM] Maico Leberle: classB obj2;
	[4:08:57 PM] Maico Leberle: obj2.obj1.x  = 1;
	[4:09:11 PM] Maico Leberle: get_id_rep(obj1);
	[4:09:27 PM] Maico Leberle: get_list_attributes(get_owner_class(get_id_rep(obj1)))
*/
	// Extract the beginning
	std::list<symtable_element>::iterator it = fields->begin();
	first_field = id_instance + "." + (*it).get_key();
	if((*it).get_class() == symtable_element::T_VAR){
		// TODO: por ahora, no trabajamos con arreglos
		translate_var_decl((*it).get_type(), first_field, id_class, 0);
		it++;
	}

	for(; it != fields->end(); ++it){
		if((*it).get_class() == symtable_element::T_VAR){
			// TODO: actualmente estamos deinifiendo como addresses a cosas de la forma
			// id_instance "." nombre de atributo
			// TODO: por ahora, no trabajamos con arreglos
			translate_var_decl((*it).get_type(), id_instance + "." + (*it).get_key(),
			id_class, 0);
		}
	}

	return first_field;
}

/* _ Al calcular el offset de las variables locales, siempre tengo que recordar
 * que this esta al comienzo, para sumarle su offset.
 * _ TODO: cunado termino: ids_info* get_ids_info(void);
 * _ TODO: con este método std::string get_id_rep(std::string); recupero el id
 * unico para una variable, de acuerdo al scope.
 * _ Para traducir métodos, recordar la utilización de la referencia this
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
		// TODO: cambiar cuando tenga los typedef
		t_class_results ret = s_table.put_class(new_class,
																	new_class.get_key(),
																	std::list<std::string>());
		assert(std::get<0>(ret) == CLASS_PUT);
	#else
		s_table.put_class(new_class,
						  new_class.get_key(),
						  std::list<std::string>());
	#endif

	// Gather information about fields. Set relative addresses with respect to the
	// beginning of this class definition.
	// TODO: también al traducir la definición de los métodos?
	int prev_offset = offset;
	offset = 0;
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
	offset = prev_offset;
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
			// Declaration of instances.
			// TODO: sería cómodo un typedef para este tipo de par
			//std::pair<symtables_stack::put_field_results, std::string*> pair =
			//		put_obj_field(symtable_element&, std::string,
			//		unsigned int, std::string);
			// TODO: por qué el segundo argumento tiene que ser un puntero?
			std::string *class_name = new std::string(node.type.id);
			symtable_element id(f->id, class_name);

			if(into_method){
				// TODO: cambiar esto cuando se haga el typedef
				// TODO: el último argumento a put_obj sería el id del primer
				// parámetro del objeto, pero no tengo claro cómo definirlo, de
				// forma tal que pueda reemplazar a futuro toda ocurrencia del
				// id de este objeto, por el id de su primer atributo.
				t_results pair =
						s_table.put_obj(id, f->id, offset, node.type.id, std::string(""));
				// TODO: al intentar recuperar el id del primer atributo, recordar
				// que parecieran estar guardándose estos en el orden inverso
				// a como uno los guarda en la tabla de símbolos.
				#ifdef __DEBUG
					assert(std::get<0>(pair) == ID_PUT);
				#endif

				// Insert code that initializes the attributes of the object.
				std::string first_field = instance_initialization(node.type.id, f->id);
				// TODO: cómo hago para guardar en s_table el dato first_field,
				// ahora que ya he puesto la variable en la tabla?
			}
			else{
				// {not into_method}
				// It is an attribute declaration. We just save the corresponding
				// data, for future uses.
				// TODO: cambiar esto cuando se haga el typedef
				t_field_results pair =
						s_table.put_obj_field(id, f->id, offset, node.type.id, std::string(""));
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
			offset += calculate_size(determine_type(node.type.type));
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
																offset);
					#ifdef __DEBUG
						assert(std::get<0>(pair) == FIELD_PUT);
					#endif
					offset += calculate_size(determine_type(node.type.type))*f->array_size;
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
																	offset);
						assert(std::get<0>(pair) == FIELD_PUT);
					#else
						s_table.put_var_field(id, f->id, offset);
					#endif

					offset += calculate_size(determine_type(node.type.type));
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

	symtable_element method(node.id, determine_type(node.type.type),
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
		s_table.put_func(method, method.get_key(), 0, actual_class->get_key());
    #endif
	// Generate code for the method declaration.
	inst_list->push_back(new_label(actual_class->get_key()+"."+node.id));

	// Initialize offset.
	unsigned int offset_prev = offset;
	offset = 0;
	for(auto p : node.parameters) {
		p->accept(*this);
	}

	into_method = true;
	// Save inst_list
	instructions_list *aux = inst_list;
	inst_list = new instructions_list();
	node.body->accept(*this);
	// Into offset we have the size of the sum of the local and temporal variables.
	// We add the corresponding enter instruction at the beginning of the procedure.
	aux->push_back(new_enter_procedure(offset));
	for (instructions_list::iterator it = inst_list->begin();
	it != inst_list->end(); ++it){
			aux->push_back(*it);
	}
	// TODO: eliminar el contenido al que apunta inst_list
	inst_list = aux;
	offset = offset_prev;
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
					t_param_results pair1 = s_table.put_obj_param(
																				 obj_param,
																				 node.id,
																				 offset,
																				 node.type.id,
																				 std::string(""));
					assert(std::get<0>(pair1) == PARAM_PUT);
				#else
					s_table.put_obj_param(obj_param, node.id, offset, node.type.id,
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
																offset);
					assert(std::get<0>(pair2) == PARAM_PUT);
				#else
					s_table.put_var_param(var_param,
											var_param.get_key(),
											offset);
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
	offset += calculate_size(determine_type(node.type.type));
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
	// TODO: no llamar a acept para node.location, sino hacer aquí la visita
	// para construir la instrucción copy adecuada. Notar que el problema
	// de hacer la visita a node.location, es que no nos alcanza con devolver
	// un address (en temp) desde ahí, para los casos en el que la location
	// representa una posición en un vector
	node.location->accept(*this);
	address_pointer l_value = temp;
	expr_call_appropriate_accept(node.expression);
	address_pointer r_value = temp;
	// TODO: crear objeto instrucción apropiado. La visita a node.location
	// y node.expression deberían guardar en un atributo de la instancia
	// una representación de la location y una direccion en donde se
	// guarda la expression, de forma que
	// podamos disponer aquí de esa información, para construir la instrucción
	// adecuada.
	switch(node.oper){
		case AssignOper::ASSIGN:
			inst_list->push_back(new_copy(l_value, r_value));
			break;

		case AssignOper::PLUS_ASSIGN:
			inst_list->push_back(new_binary_assign(l_value, l_value, r_value,
													quad_oper::PLUS));
			break;

		case AssignOper::MINUS_ASSIGN:
			inst_list->push_back(new_binary_assign(l_value, l_value, r_value,
													quad_oper::MINUS));
	}
}

void inter_code_gen_visitor::visit(node_location& node) {
	#ifdef __DEBUG
		std::cout << "Translating location expression" << std::endl;
	#endif

	// TODO: lo deberíamos convertir a algo de la forma offset(base,index,scale)?
	// no está eso dependiendo de la arq. x86_64, siendo que no es la idea del
	// código intermedio?: vamos generando las direcciones
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

	// TODO: no estoy utilizando la variable this_param
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
	inst_list->push_back(new_conditional_jump_inst(temp, label_else, quad_oper::IFFALSE));
	// Code for the "if" branch.
	stm_call_appropriate_accept(node.then_statement);
	// Unconditional jump to the final part of the translation.
	std::string label_end("L2");
	inst_list->push_back(new_unconditional_jump_inst(label_end));

	// Code for the "else" branch.
	inst_list->push_back(new_label(label_else));
	if(node.else_statement != nullptr) {
		stm_call_appropriate_accept(node.else_statement);
	}
	// Ending.
	inst_list->push_back(new_label(label_end));
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
	inst_list->push_back(new_label(label_beginning));

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
						label_ending));

	// Body code.
	stm_call_appropriate_accept(node.body);

	// Increment.
	address_pointer inc = new_integer_constant(1);
	inst_list->push_back(new_binary_assign(var, var, inc, quad_oper::PLUS));

	// Return to the beginning
	inst_list->push_back(new_unconditional_jump_inst(label_beginning));

	// Ending.
	inst_list->push_back(new_label(label_ending));
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
	inst_list->push_back(new_label(label_beginning));

	// Evaluate the guard. Its value is in temp.
	expr_call_appropriate_accept(node.expression);

	// If the guard is false, jump to the end of the code.
	std::string label_ending("L2");
	inst_list->push_back(new_conditional_jump_inst(temp, label_ending,
										quad_oper::IFFALSE));

	// Body.
	stm_call_appropriate_accept(node.body);

	// Return to the beginning.
	inst_list->push_back(new_unconditional_jump_inst(label_beginning));

	// The end of the loop.
	inst_list->push_back(new_label(label_ending));
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
	inst_list->push_back(new_unconditional_jump_inst(std::string("L2")));
}

void inter_code_gen_visitor::visit(node_continue_statement& node) {
	#ifdef __DEBUG
		std::cout << "Translating continue statement" << std::endl;
	#endif
	// Continue is translated as an unconditional jump to the beginning of the loop.
	inst_list->push_back(new_unconditional_jump_inst(std::string("L1")));
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
	t_results pair =  s_table.new_temp(offset);
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
	t_results pair =  s_table.new_temp(offset);
	// TODO: chequear put_results
	// TODO: no estoy usando new_temp_address
	dest = new_name_address(*std::get<1>(pair));
	inst_list->push_back(new_unary_assign(dest, temp, quad_oper::NEGATION));

	// Update offset.
	offset += boolean_width;

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
	t_results pair =  s_table.new_temp(offset);
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

