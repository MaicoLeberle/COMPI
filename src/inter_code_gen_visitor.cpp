#include "inter_code_gen_visitor.h"

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

intermediate_symtable* inter_code_gen_visitor::get_symtable(void){
	return &s_table;
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

/*	Translate a variable declaration, register its information into the
 * 	table of symbols, and updates the offset.
 * */

void inter_code_gen_visitor::translate_var_decl(symtable_element::id_type type,
std::string id, std::string class_name, unsigned int array_size){

	// TODO: generar nuevo identificador, colocarlo en
	// la tabla, con offset = offset+integer_width
	// _ TODO: std::pair<symtables_stack::put_results, std::string*> put_var(symtable_element, std::string, unsigned int);
	// * me devuelve ahora un par: std::string es el id
	// TODO: al comenzar aquí, el valor de offset, antes de ser actualizado
	// es el valor del comienzo de la ubicación de esta variable?
	address_pointer dest, constant = nullptr;


	if(type == symtable_element::ID){
		// TODO: por qué class_name tiene que pasarse por puntero?
		std::string *class_name_aux = new std::string(class_name);
		symtable_element variable(id, class_name_aux);
		#ifdef __DEBUG
			std::pair<intermediate_symtable::put_results, std::string*> pair = s_table.put_var(variable,
																		id,
																		offset);
			assert(std::get<0>(pair) == intermediate_symtable::ID_PUT);
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
			/*if(type == symtable_element::INTEGER){
				constant = new_integer_constant(integer_initial_value);
			}
			else if(type == symtable_element::FLOAT){
				constant = new_float_constant(float_initial_value);
			}
			else{
				#ifdef __DEBUG
					assert(type == symtable_element::BOOLEAN);
				#endif
				constant = new_boolean_constant(boolean_initial_value);
			}*/

			symtable_element variable(id, type, array_size);
			std::pair<intermediate_symtable::put_results, std::string*> pair = s_table.put_var(variable,
																		id,
																		offset);
			offset += calculate_size(type)*array_size;
			#ifdef __DEBUG
				assert(std::get<0>(pair) == intermediate_symtable::ID_PUT);
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
			std::pair<intermediate_symtable::put_results, std::string*> pair = s_table.put_var(variable,
																		id,
																		offset);
			offset += calculate_size(type);
			#ifdef __DEBUG
				assert(std::get<0>(pair) == intermediate_symtable::ID_PUT);
			#endif
			// TODO: voy descubriendo la semántica de este procedimiento: debería
			// especificar que se trata de uno que ayuda a traducir las declaraciones
			// de variables que están en el código original
			dest = new_name_address(id);
			inst_list->push_back(new_copy(dest, constant));
		}
	}
}

void inter_code_gen_visitor::instance_initialization(std::string id_class,
	std::string id_instance){
	// Translate instance declaration into a sequence of code to
	// initialize each attribute.
	symtable_element *class_object = s_table.get(id_class);
	std::list<symtable_element> *fields = class_object->get_class_fields();
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
	// TODO: para declarar una nueva instancia:
	// std::pair<symtables_stack::put_results, std::string*> put_obj(symtable_element& , std::string, unsigned int, std::string);
	for(std::list<symtable_element>::iterator it = fields->begin();
	it != fields->end(); ++it){
		if((*it).get_class() == symtable_element::T_VAR){
			// TODO: actualmente estamos deinifiendo como addresses a cosas de la forma
			// id_instance "." nombre de atributo
			translate_var_decl((*it).get_type(), id_instance + "." + (*it).get_key(),
			id_class, 0);
		}
	}
}

/* _ Las temporales van siempre a registros, y yo, en el visitor, invento
 * los nombres de las temporales, pero al analizar cada expresión, manejo un
 * contador de temporales que lo inicio a 0.
 * _ Los métodos y clases tendran etiquetas únicas. Cuando registro un método
 * tengo que pasarle la cantidad de variables locales, que es información
 * almacenada en el ast.
 * _ La traducción de la declaración de una instancia, la podríamos traducir
 * a instrucciones que inicializan atributos (ya que no tenemos heap, si se
 * tradujera a un constructor, no tendríamos forma de recuperar la información
 * del mismo).
 * _ Al calcular el offset de las variables locales, siempre tengo que recordar
 * que this esta al comienzo, para sumarle su offset.
 * _ TODO: cunado termino: ids_info* get_ids_info(void);
 * _ TODO: con este método std::string get_id_rep(std::string); recupero el id
 * unico para una variable, de acuerdo al scope.
 * _ TODO: std::pair<symtables_stack::put_results, std::string*> put_var(symtable_element, std::string, unsigned int);
 * me devuelve ahora un par: std::string es el id
 * _ Para traducir métodos, recordar la utilización de la referencia this
 * */
void inter_code_gen_visitor::visit(const node_program& node){
	#ifdef __DEBUG
		std::cout << "Beginning translation." << std::endl;
	#endif
	// Define a new block: the global scope
	s_table.push_symtable();

	for(auto c : node.classes) {
		c->accept(*this);
	}
}

void inter_code_gen_visitor::visit(const node_class_decl& node) {
	#ifdef __DEBUG
		std::cout << "Translating class " << node.id << std::endl;
	#endif
	// Translate the fields of the class.
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
	#ifdef __DEBUG
		std::pair<intermediate_symtable::put_class_results, std::string*> ret = s_table.put_class(new_class,
																	new_class.get_key(),
																	std::list<std::string>());
		assert(std::get<0>(ret) == intermediate_symtable::CLASS_PUT);
	#else
		s_table.put_class(new_class.get_key(), new_class, std::list<std::string>());
	#endif

	// Relative addresses with respect to the beginning of this
	// class definition.
	// TODO: también al traducir la definición de los métodos?
	int prev_offset = offset;
	offset = 0;
	into_method = false;
	for(std::list<class_block_pointer>::iterator it = fields.begin();
	it != fields.end(); ++it){
		node_field_decl& aux = static_cast<node_field_decl&> (*(*it));
		aux.accept(*this);
	}
	// End of class declaration -> in offset we have the size of an instance.
	// TODO: tengo que guardar ese valor en algún lugar en la tabla, para poder
	// después recuperarlo, al momento de necesitar saber el tamaño que
	// ocupa una instancia, para atualizar el offset

	// Translation of methods.
	for(std::list<class_block_pointer>::iterator it = methods.begin();
	it != methods.end(); ++it){
		node_method_decl& aux = static_cast<node_method_decl&> (*(*it));
		aux.accept(*this);
	}

	// Restore offset value.
	offset = prev_offset;
	s_table.finish_class_analysis();
}


	// Statements
	void inter_code_gen_visitor::visit(const node_while_statement& node) {}
	void inter_code_gen_visitor::visit(const node_return_statement& node) {}
	void inter_code_gen_visitor::visit(const node_break_statement& node) {}
	void inter_code_gen_visitor::visit(const node_continue_statement& node) {}
	void inter_code_gen_visitor::visit(const node_skip_statement& node) {}
	// Expressions
	void inter_code_gen_visitor::visit(const node_binary_operation_expr& node) {}
	void inter_code_gen_visitor::visit(const node_negate_expr& node) {}
	void inter_code_gen_visitor::visit(const node_negative_expr& node) {}
	void inter_code_gen_visitor::visit(const node_parentheses_expr& node) {}

void inter_code_gen_visitor::visit(const node_field_decl& node) {
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
			//		put_obj_field(symtable_element&, std::string, unsigned int, std::string);
			// TODO: por qué el segundo argumento tiene que ser un puntero?
			std::string *class_name = new std::string(node.type.id);
			symtable_element id(f->id, class_name);

			if(into_method){
				// TODO: cambiar esto cuando se haga el typedef
				std::pair<intermediate_symtable::put_results, std::string*> pair =
						s_table.put_obj(id, f->id, offset, node.type.id, std::string(""));
				// TODO: el último argumento sería el id del primer parámetro del objeto,
				// pero no tengo claro cómo definirlo, de forma tal que pueda reemplazar
				// a futuro toda ocurrencia del id de este objeto, por el id de su primer
				// atributo.
				// TODO: al intentar recuperar el id del primer atributo, recordar
				// que parecieran estar guardándose estos en el orden inverso
				// a como uno los guarda en la tabla de símbolos.
				#ifdef __DEBUG
					assert(std::get<0>(pair) == intermediate_symtable::ID_PUT);
				#endif

				// Insert code that initialices the attributes of the object.
				instance_initialization(node.type.id, f->id);
			}
			else{
				// {not into_method}
				// Is an attribute declaration. We just save the corresponding
				// data, for future use.
				// TODO: cambiar esto cuando se haga el typedef
				std::pair<intermediate_symtable::put_field_results, std::string*> pair =
						s_table.put_obj_field(id, f->id, offset, node.type.id, std::string(""));
				// TODO: el último argumento sería el id del primer parámetro del objeto,
				// pero no tengo claro cómo definirlo, de forma tal que pueda reemplazar
				// a futuro toda ocurrencia del id de este objeto, por el id de su primer
				// atributo.
				// TODO: al intentar recuperar el id del primer atributo, recordar
				// que parecieran estar guardándose estos en el orden inverso
				// a como uno los guarda en la tabla de símbolos.
				#ifdef __DEBUG
					assert(std::get<0>(pair) == intermediate_symtable::FIELD_PUT);
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

					symtable_element id(f->id, determine_type(node.type.type), f->array_size);
					// TODO: cambiar esto cuando se haga el typedef
					std::pair<intermediate_symtable::put_field_results, std::string*> pair = s_table.put_var_field(id,
																				f->id,
																				offset);
					#ifdef __DEBUG
						assert(std::get<0>(pair) == intermediate_symtable::FIELD_PUT);
					#endif
					offset += calculate_size(determine_type(node.type.type))*f->array_size;
				}
			}
			else{
				// {f->array_size <= 0}
				// Basic type
				if(into_method){
					std::cout << "translate_var_decl." << std::endl;
					// TODO: este tercer argumento es medio fulero...
					translate_var_decl(determine_type(node.type.type), f->id,
										std::string(""), 0);
				}
				else{
					// {not into_method}

					symtable_element id(f->id, determine_type(node.type.type));


					#ifdef __DEBUG
						// TODO: cambiar esto cuando se haga el typedef
						std::pair<intermediate_symtable::put_field_results, std::string*> pair = s_table.put_var_field(id,
																									f->id,
																									offset);
						assert(std::get<0>(pair) == intermediate_symtable::FIELD_PUT);
					#else
						s_table.put_var_field(id, f->id, offset);
					#endif

					offset += calculate_size(determine_type(node.type.type));
				}

			}
		}
	}
}

void inter_code_gen_visitor::visit(const node_id& node) {
	// This visitor does not need to do some task into node_id.
}

void inter_code_gen_visitor::visit(const node_method_decl& node){
	// TODO: aquí debería agregar la variable "this" a la tabla de símbolos?
	// donde debería utilizar this en la traducción?
	#ifdef __DEBUG
		std::cout << "Translating method " << node.id << std::endl;
	#endif

	symtable_element method(node.id, determine_type(node.type.type),
							new std::list<symtable_element>());
	#ifdef __DEBUG
		// TODO: a esta altura del partido, no se cuantas variables locales
		// tiene el método. Sin embargo, necesito ya mismo agregarlo a la
		// tabla. Debería poder agregar esta inf. faltante al final
		std::pair<intermediate_symtable::put_field_results, std::string*> pair = s_table.put_func_field(method,
																		node.id,
																		0,
																		actual_class->get_key());
		assert(std::get<0>(pair) == intermediate_symtable::FIELD_PUT);
	#else
		s_table.put_func_field(method, node.id, 0, actual_class->get_key());
	#endif
	// Generate code for the method declaration.
	inst_list->push_back(new_label(actual_class->get_key()+"."+node.id));
	// Initialize offset.
	// TODO: hace falta esto?
	unsigned int offset_prev = offset;
	offset = 0;
	for(auto p : node.parameters) {
		p->accept(*this);
	}

	into_method = true;
	// Save inst_list
	instructions_list *aux = inst_list;
	inst_list = new instructions_list(); // TODO: eliminar!
	node.body->accept(*this);
	// Into offset we have the size of the sum of the local an temporal variables.
	// We add the corresponding enter instruction at the beginning of the procedure.
	aux->push_back(new_enter_procedure(offset));
	// TODO: agregar a aux todas las instrucciones en inst_list y hacer que
	// inst_list apunte a aux
	for (instructions_list::iterator it = inst_list->begin();
	it != inst_list->end(); ++it){
			aux->push_back(*it);
	}
	// TODO: eliminar el contenido al que apunta inst_list
	inst_list = aux;
	offset = offset_prev;
	s_table.finish_func_analysis();
}

void inter_code_gen_visitor::visit(const node_parameter_identifier& node) {
	/* Paso por parámetro los atributos de la clase, prefijandoles "self.". */
	#ifdef __DEBUG
		std::cout << "Translating parameter " << node.id << std::endl;
	#endif

	switch(node.type.type){
		case Type::ID:
			{
				std::string *class_name = new std::string(node.type.id);
				// TODO: la tabla de símbolos hace una copia de obj_param?
				// TODO: cual es el offset de este parámetro, en el método? siendo
				// que se guarda en registros...
				// TODO: el último parámetro debería ser un string de la forma
				// node.id "." nombre del primer atributo de la clase
				symtable_element obj_param(node.id, class_name);

				#ifdef __DEBUG
					std::pair<intermediate_symtable::put_param_results, std::string*> pair1 = s_table.put_obj_param(
																				 obj_param,
																				 node.id,
																				 0,
																				 node.type.id,
																				 std::string(""));
					assert(std::get<0>(pair1) == intermediate_symtable::PARAM_PUT);
				#else
					s_table.put_obj_param(obj_param, node.id, 0, node.type.id,
										 std::string(""));
				#endif
			}
			break;

		default:{
				// TODO: estoy asumiendo que un parámetro o bien es un objeto o bien
				// un tipo básico.
				symtable_element var_param(node.id, determine_type(node.type.type));
				#ifdef __DEBUG
					std::pair<intermediate_symtable::put_param_results, std::string*> pair2 = s_table.put_var_param(
																				 var_param,
																				 node.id,
																				 0);
					assert(std::get<0>(pair2) == intermediate_symtable::PARAM_PUT);
				#else
					s_table.put_var_param(obj_param, node.id, 0);
				#endif
			}
	}
	// TODO: en la arquitectura x86_64, es el método el que construye
	// el stack frame?: "The following enter instruction
	// sets up the stack frame" (de "x86-64-architecture-guide"). Pero es quien
	// lo llama quien coloca los parametros en los registros. Debemos contabilizar
	// el tamaño de los parámetros?
	// TODO: si los parámetros van siempre a registros, no debería estar alterando
	// el offset, cierto?
	//offset += calculate_size(determine_type(node.type.type));
}

void inter_code_gen_visitor::visit(const node_body& node) {
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

void inter_code_gen_visitor::visit(const node_block& node) {
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

void inter_code_gen_visitor::visit(const node_assignment_statement& node) {
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

void inter_code_gen_visitor::visit(const node_location& node) {
	#ifdef __DEBUG
		std::cout << "Translating location expression" << std::endl;
	#endif

	// TODO: lo deberíamos convertir a algo de la forma offset(base,index,scale)?
	// no está eso dependiendo de la arq. x86_64, siendo que no es la idea del
	// código intermedio?: vamos generando las direcciones
	// TODO: por ahora, convertimos la location en un address. Es correcto
	// usar este nombre para la address?
	std::string name = node.ids.printable_field_id();
	temp = new_name_address(name);
}

void inter_code_gen_visitor::visit(const node_int_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating int literal with value " << node.value << std::endl;
	#endif
	// TODO: convertir a string
	// TODO: ver el código intermediate.c: no hace falta crear una temporal
	// para almacenar la constance. Basta con crear un objeto struct
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

void inter_code_gen_visitor::visit(const node_bool_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating bool literal with value " << node.value << std::endl;
	#endif
	// TODO: tengo que generar un address temporal, y ahí guardar este valor...
	temp = new_boolean_constant(node.value);
}

void inter_code_gen_visitor::visit(const node_float_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating FLOAT literal with value " << node.value << std::endl;
	#endif

	temp = new_boolean_constant(node.value);
}

void inter_code_gen_visitor::visit(const node_string_literal& node) {
	#ifdef __DEBUG
		std::cout << "Translating string literal of value " << node.value << std::endl;
	#endif
	// TODO: representación de literales string?
	// TODO: no tengo representación de este tipo de literales en el 3-address code
	//temp = std::string("$"+node.value);
}

void inter_code_gen_visitor::visit(const node_method_call_expr& node) {
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

void inter_code_gen_visitor::visit(const node_method_call_statement& node) {
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

void inter_code_gen_visitor::visit(const node_if_statement& node){
	#ifdef __DEBUG
			std::cout << "Translating if statement" << std::endl;
	#endif
	expr_call_appropriate_accept(node.expression);
	// TODO: la visita a node.expression, guarda en un atributo de la instancia
	// el valor de la referencia (temp) en donde se almacena el
	// resultado de evaluar node.expression
	// TODO: L1 y L2 son 2 etiquetas de valores fijos, a definir
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

void inter_code_gen_visitor::visit(const node_for_statement& node){
	#ifdef __DEBUG
			std::cout << "Translating for statement" << std::endl;
	#endif

	expr_call_appropriate_accept(node.from);
	// TODO: var es la variable sobre la que itera el ciclo, temp es la dirección
	// en donde se almacenó la evaluación de node.from
	// TODO: la variable x es declarada fuera del ciclo?
	address_pointer var = new_name_address(node.id);
	inst_list->push_back(new_copy(var, temp));
	expr_call_appropriate_accept(node.to);
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
/*
void inter_code_gen_visitor::visit(const node_while_statement& node) {
#ifdef __DEBUG
	std::cout << "Translating while statement" << std::endl;
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

void inter_code_gen_visitor::visit(const node_return_statement& node) {
	symtable_element::id_type type_method = actual_method->get_type();

#ifdef __DEBUG
	std::cout << "Translating return statement" << std::endl;
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

void inter_code_gen_visitor::visit(const node_break_statement& node) {
#ifdef __DEBUG
	std::cout << "Translating break statement" << std::endl;
#endif

	// Rule 19: break and continue, only into a loop's body.
	if (!into_for_or_while){
		register_error(std::string("Break statement outside a loop."),
				ERROR_19);
	}
}

void inter_code_gen_visitor::visit(const node_continue_statement& node) {
#ifdef __DEBUG
	std::cout << "Translating continue statement" << std::endl;
#endif
	// Rule 19: break and continue, only into a loop's body.
	if (!into_for_or_while){
		register_error(std::string("Continue statement outside a loop."),
				ERROR_19);
	}
}

void inter_code_gen_visitor::visit(const node_skip_statement& node) {
#ifdef __DEBUG
	std::cout << "Translating skip statement" << std::endl;
#endif
}

void inter_code_gen_visitor::visit(const node_binary_operation_expr& node) {
	symtable_element::id_type l_op_type, r_op_type;
	symtable_element::id_class l_op_class, r_op_class;

#ifdef __DEBUG
	std::cout << "Translating binary operation" << std::endl;
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

void inter_code_gen_visitor::visit(const node_negate_expr& node) {
#ifdef __DEBUG
	std::cout << "Translating negate expression" << std::endl;
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

void inter_code_gen_visitor::visit(const node_negative_expr& node) {
#ifdef __DEBUG
	std::cout << "Translating negative expression" << std::endl;
#endif
	// TODO: notar que sí tenemos un nodo especial para esta
	// situación.
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

void inter_code_gen_visitor::visit(const node_parentheses_expr& node) {
#ifdef __DEBUG
	std::cout << "Translating parentheses expression" << std::endl;
#endif

	expr_call_appropriate_accept(node.expression);
}

*/
