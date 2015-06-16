#include "semantic_analysis.h"

semantic_analysis::semantic_analysis () {
	errors = 0;
	into_for_or_while = false;
}
// TODO: cambiar en visitor.h: por alguna razon, lo que actualmente
// se ha subido no es la inferfaz que yo había definido: los metodos
// no son "virtuales puros"
// Adds an error message to the standard error output and appends
// the std::endl manipulator. Also, counts the error.
void semantic_analysis::register_error(std::string error, error_id error_encountered){
	std::cerr << error << std::endl;
	// The process is not interrupted. We just count the error.
	error += 1;
	last_error = error_encountered;
}

semantic_analysis::error_id semantic_analysis::get_last_error(){
	return last_error;
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

		#ifdef __DEBUG
		default:
			assert(false);
		#endif
	}

	return ret;
}

/*symtable_element *get_next_symtable_element(symtable_element *actual_element,
std::string id){
	#ifdef __DEBUG
	// PRE
	assert(actual_element->get_class() != symtable_element::NOT_FOUND);
	#endif

	switch(actual_element->get_class()){
		case symtable_element::T_CLASS:
			std::list<symtable_element> *fields = actual_element->get_class_fields();
			std::vector<int>::size_type i = (std::vector<int>::size_type) 1;
			// TODO: como es
			std::vector<int>::size_type limit = fields->size();

	}


}

symtable_element::id_type dereference_get_element(reference_list ids){
	#ifdef __DEBUG
		// PRE
		assert(s_table.get_length() > 0);
	#endif

	symtable_element *aux = s_table.get(ids[0]);
	// Rule 2: declaration before use
	if (aux->get_class() == symtable_element::NOT_FOUND){
		register_error(std::string("Id "+ids[0]+" must be declared before use."));
	}
	else{
		std::vector<int>::size_type i = (std::vector<int>::size_type) 1;
		std::vector<int>::size_type limit = ids.size();
		// INV : {aux is the symtable_element for the id number i-1 in ids}
		// TODO: corregir este invariante
		while(i < limit){
			// TODO: asumo que en ids se guardan los identificadores en orden
			// desde 0 (el primer id de la izquierda), hacia la derecha...
			if (__DEBUG)
				std::cout << "Accessing reference " << ids[i] << std::endl;

			// Index the previous symtable_element, with the next id.
			switch(aux->get_class()){
				case symtable_element::T_CLASS:
					std::list<symtable_element> *fields = aux->get_class_fields();


			}
			aux = aux.get(ids[i]);
			// Rule 2: declaration before use
			if (aux.get_class() == symtable_element::NOT_FOUND){
				register_error(std::string("Id "+ids[i]+" must be declared before use."));
				// TODO: cortamos esto acá?
				// TODO: cómo informamos de este error?
				break;
			}
			i++
		}
	}
	// {aux is the symtable_element referenced by ids or a symtable_element
	// with class symtable_element::NOT_FOUND, if ids is invalid.}
	return aux;
}*/

void semantic_analysis::visit(const node_program& node) {
	#ifdef __DEBUG
	 	 std::cout << "Beginning analysis." << std::endl;
	#endif
	for(auto c : node.classes) {
		c->accept(*this);
	}
	// {s_tables.get_length() == 1 || s_tables.get_length() == 0}
	#ifdef __DEBUG
		unsigned int length = s_table.get_length();
		assert(length == 1 || length == 0);
	#endif
	/*symtable_element* classA = new symtable_element(...);
	  s.put(classA);
	  s.push_symtable(classA);
	  Si ahora analizamos los elementos de la clase:
	  s.put_class_field(intX);
	  s.put(intX);
	  Método:
	  s.put_class_field(functionF); //Registrar en la clase que corresponde
	  s.put(functionF); // Registrar en la tabla de símbolos
	  s.push_symtable(functioF); // Abro
	  Para los parámetros:
	  s.put_func_param(parametro)
	  s.put(parametro)
	  Para variables locales del método:
	  s.put(variable local)
	  Si encuentro un bloque:
	  s.push_symtable()

	 * */
	/* TODO: para analizar locations id1.id2 :
	 * tomo el id1, lo uso para obtener la instancia. Tomo la instancia,
	 * y llamo a get_class_fields, y busco ahí los métodos
	 * */
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
	symtable_element new_class(node.id, new std::list<symtable_element>());
	// Define a new scope
	s_table.push_symtable(new_class);
	for(auto cb : node.class_block) {
		if (cb->is_node_field_decl()){
			// TODO: por qué tiene que ser node_field_decl&?
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
		// TODO: necesito un get a realizarse sobre el scope actual
		// el cual debería ser el interior de esta clase...es decir,
		// un get que no realice la búsqueda en alcances exteriores,
		// si la búsqueda en el más interno falló
		// HACER LA CONSULTA SOBRE EL OBJETO NEW_CLASS
		// TODO: directamente usamos put, y vemos qué esultado nos devuelve
		// para sabe si eta o no: NO, en este caso, sólo necesitamos
		// consultar.
		// TODO: mantenemos las 2 representaciones de expresiones,
		// y la tabla de símbolos no tiene por qué tener un enteo
		// especial para representa la existencia de expesiones
		// mal fomadas
		symtable_element *method = s_table.get(std::string("main"));
		if(method->get_class() == symtable_element::NOT_FOUND){
			register_error(std::string("\"main\" class without a \"main\" method."),
					ERROR_3);
		}
	}
}

void semantic_analysis::visit(const node_field_decl& node) {
	symtable_element *id = NULL;

	#ifdef __DEBUG
	 	 std::cout << "Accessing field declaration." << std::endl;
	#endif

	for(auto f : node.ids) {
		// Rule 1: this is the first time this identifier is declared in this
		// scope
		// TODO: necesito un método que me diga si un identificador está
		// declarado en el scope actual o no
		if(s_table.get(f->id)->get_class() != symtable_element::NOT_FOUND){
			// TODO: el operador + está sobre-cargado para std::string?.
			register_error(std::string("Identifier "+f->id+" already declared in this scope."),
					ERROR_1);
		}
		// Rule 4: if it is an array declaration => the length must be > 0
		if (f->is_array && f->array_size <= 0){
			register_error(std::string("Array declaration with id " +
							f->id +" has dimension <= 0."),
					ERROR_4);
		}
		// TODO: tenemos que unificar la manera de representar los tipos,
		// para no tener que estar averiguando aquí de qué tipo es el id.
		// Everything ok. Add the identifier to the symbol table.
		if (node.type.type == Type::ID){
			// Declaration of instances.
			// Create a new symtable_element object, that represents
			// an object, with the id and the name of the corresponding class.
			// TODO: así o con new?
			// TODO: chequear que el tipo exista
			id = new symtable_element(f->id, node.type.id);
		}
		else{
			// {node.type.type != Type::ID}
			// Basic type or array
			if (f->is_array){
				// Array
				// TODO: me rechaza el tipo, porque me pide algo que sea
				// del enum definido en symtable!
				id = new symtable_element(f->id, determine_type(node.type.type),
					  f->array_size);
			}
			else{
				// {f->array_size <= 0}
				// Basic type
				id = new symtable_element(f->id, determine_type(node.type.type));
			}
		}
		// Add the symbol to s_table.
		// TODO: esta bien definida la variable? Notar que recibe directamente
		// un symtable_element, no un puntero o referencia. Lo copia?
		#ifdef __DEBUG
			assert(s_table.put_class_field(*id) == symtables_stack::INSERTED);
		#else
			s_table.put_class_field(*id);
		#endif
	}
}

void semantic_analysis::visit(const node_id& node) {
	// This visitor does not need to do some task into node_id.
}

void semantic_analysis::visit(const node_method_decl& node) {
	#ifdef __DEBUG
		std::cout << "Accessing method " << node.id << std::endl;
	#endif

	symtable_element method(node.id, determine_type(node.type.type),
							new std::list<symtable_element>());
	for(auto p : node.parameters) {
		p->accept(*this);
	}

	node.body->accept(*this);
}

void semantic_analysis::visit(const node_parameter_identifier& node) {
	#ifdef __DEBUG
		std::cout << "Accessing parameter " << node.id << std::endl;
	#endif
	// We add the identifier to the actual symbol table
	// TODO: habría que re-utilizar el id_type definido en node.h
	// por otro lado, tendríamos que agregar un símbolo que represente
	// la imposibilidad de asignarle un tipo a una expresión
	// (por existir algún error en esta)
	if (node.type.type == Type::ID){
		// TODO: chequear que el tipo exista
	}
	// TODO: chequear que node.id no coincida con el nombre del
	// método.
	symtable_element param(node.id, determine_type(node.type.type));
	s_table.put_func_param(param);
}

void semantic_analysis::visit(const node_body& node) {}
	void semantic_analysis::visit(const node_block& node) {}
	// Statements
 	void semantic_analysis::visit(const node_assignment_statement& node) {}
 	void semantic_analysis::visit(const node_method_call& node) {}
 	void semantic_analysis::visit(const node_if_statement& node) {}
 	void semantic_analysis::visit(const node_for_statement& node) {}
 	void semantic_analysis::visit(const node_while_statement& node) {}
 	void semantic_analysis::visit(const node_return_statement& node) {}
 	void semantic_analysis::visit(const node_break_statement& node) {}
 	void semantic_analysis::visit(const node_continue_statement& node) {}
 	void semantic_analysis::visit(const node_skip_statement& node) {}
 	// Expressions
 	void semantic_analysis::visit(const node_int_literal& node) {}
 	void semantic_analysis::visit(const node_float_literal& node) {}
 	void semantic_analysis::visit(const node_bool_literal& node) {}
 	void semantic_analysis::visit(const node_string_literal& node) {}
 	void semantic_analysis::visit(const node_binary_operation_expr& node) {}
 	void semantic_analysis::visit(const node_location& node) {}
 	void semantic_analysis::visit(const node_negate_expr& node) {}
 	void semantic_analysis::visit(const node_negative_expr& node) {}
 	void semantic_analysis::visit(const node_parentheses_expr& node) {}
/*


void semantic_analysis::visit(const node_body& node) {
	#ifdef __DEBUG
		std::cout << "Accessing body of method" << std::endl;
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

void semantic_analysis::visit(const node_block& node) {
	// A block defines a new scope.
	s_table.push_symtable();
	// Analyze the content of the block.
	for(auto s : node.content) {
		s->accept(*this);
	}
}
void semantic_analysis::visit(const node_assignment_statement& node) {
	#ifdef __DEBUG
		std::cout << "Accessing assignment statement" << std::endl;
	#endif

		symtable_element::id_type type_id, type_expr;

	// Check and determine the type of the location.
	// TODO: tenemos que agregar al enum id_type de symtable,
	// el tipo ARRAY de, lo que sea...
	// TODO: cuando definamos nuestra representación de expresiones
	// definir el atributo type_l_expr que corresponda, y
	// hacer las correspondientes asignaciones aquí.

	node.location->accept(*this);
	if (well_formed){
		type_id = type_l_expr;
	}
	else{
		// {not well_formed}
		// TODO: continuamos?
	}
	// Rule 16: both, the type of the location and the expression to be assigned,
	// must be the same
	node.expression->accept(*this);
	if (well_formed){
		type_expr = type_l_expr;
		// TODO: tambien tendríamos que chequear que la expresión a asignar efectivamente
		// evalue a algo asignable...
		// TODO: realizar la comparación de tipos como corresponda, de acuerdo
		// a la forma que escojamos de representación de los mismos
		if (type_id != type_expr){
			register_error(std::string("Location and expression assigned have different types."));
		}
	}
	else{
		// {not well_formed}

	}
}

void semantic_analysis::visit(const node_method_call& node) {
	#ifdef __DEBUG
		std::cout << "Accessing method call statement" << std::endl;
	#endif

	symtable_element::id_type type_method, type_parameter;
	symtable_element aux; // TODO: debería usar un puntero, para no estar creando
						 // instancias que no me hacen falta?

	// Determine the type of the location
	//for(auto r : node.ids) {
	// TODO: get_type para el symtable_element que representa a un método,
	// me devuelve el tipo de retorno del método
	// TODO: cómo no consideramos una location a este elemento de
	// method_call, entonces no puedo reutilizar el metodo visit
	// para node_location, que ya realiza este proceso medio complicado...
	for(std::vector<int>::size_type i = (std::vector<int>::size_type) 0;
		i < node.ids.size(); i++){
		// TODO: asumo que en ids se guardan los identificadores en orden
		// desde 0 (el primer id de la izquierda), hacia la derecha...
		// TODO: ir indexando la tabla de símbolos, hasta el último
		// id, que debe referir a un método. Extraer su valor
		// de retorno, y guardarlo en type_method
		if (__DEBUG)
			std::cout << "Accessing reference " << r << std::endl;

		if (i == (std::vector<int>::size_type) 0){
			// First identifier, from the left
			aux = s_table.get(node.ids[i]); // TODO: get hace una copia de lo que devuelve?
									// no debería devolver mejor un puntero?
			// Rule 2: declaration before use
			if (aux.get_class == symtable_element::NOT_FOUND){
				register_error(std::string("Id "+r+" must be declared before use."));
				// TODO: cortamos esto acá?
				break;
			}
		}
		else{
			// {i != (std::vector<int>::size_type) 0}
			// Search into the previously obtained symtable_element
			// TODO: puedo hacer esto?
			aux = aux.get(node.ids[i]);
			// Rule 2: declaration before use
			if (aux.get_class == symtable_element::NOT_FOUND){
				register_error(std::string("Id "+r+" must be declared before use."));
				// TODO: cortamos esto acá?
				break;
			}
			if(i == node.ids.size() - (std::vector<int>::size_type)1){
				// The identifier must point to a method
			}
		}
		// TODO: get_type de symtable_element, para el caso de
		// un método, me devolvería el tipo de retorno del método?
		// TODO: asignar el tipo de retorno a type_l_expr

	}
	// Rule 5: the number and type of the actual parameters,
	// must be the same of the formal parameters
	for(auto r : node.parameters) {
		// TODO: recorrer node.parameters preguntando si coincide
		// el tipo de estos con los parametros formales del método.
		// También preguntar por la cantidad de parámetros
		type_parameter = r->accept(*this);
		// TODO: chequear que el parametro evalue efectivamente
		// a algun tipo basico
		// Rule 7: string literals only with extern methods
		// TODO: string type?
		if (type_parameter == symtable_element::STRING){
			// TODO: chequear que el método invocado sea extern
		}
	}

	// Rule 6: if a method call is used as an expression, the method
	// must return some value. We just simply return the type of return
	// of the method.
	// TODO: quizás convenga utilizar como representación de tipo
	// lo definido en symtable, para poder retornar directamente
	// la información que tengamos en la tabla
	return type_method;
}

void semantic_analysis::visit(const node_if_statement& node){
	node.expression->accept(*this);
	// Rule 12: the guard must be a boolean expression
	if (type_l_expr != symtable_element::BOOLEAN){
		register_error(std::string("Non-boolean guard of an if expression ."));
	}

	node.then_statement->accept(*this);

	if(node.else_statement != nullptr) {
		node.else_statement->accept(*this);
	}
}

void semantic_analysis::visit(const node_for_statement& node){
	symtable_element::id_type type_expr;

	std::cout << "Accessing for statement" << std::endl;

	// Rule 18: from and to expression must evaluate to integers
	node.from->accept(*this);
	type_expr = type_l_expr;
	// TODO: estoy usando como representacion del tipo de una expresion,
	// a los valores de un enum que fueron originalmente definidos para
	// representar el tipo de los id...?
	if (type_expr != symtable_element::INTEGER){
		register_error(std::string("Non-integer \"from\" expression of a for loop."));
	}

	node.to->accept(*this);
	type_expr = type_l_expr;
	if (type_expr != symtable_element::INTEGER){
		register_error(std::string("Non-integer \"to\" expression of a for loop."));
	}
	// To check rules that depend on this information...
	into_for_or_while = true;
	node.body->accept(*this);
	into_for_or_while = false;
}

void semantic_analysis::visit(const node_while_statement& node) {
	symtable_element::id_type type_guard;
	std::cout << "Accessing while statement" << std::endl;
	node.expression->accept(*this);
	type_guard = type_l_expr;
	// Rule 12: the guard must be a boolean expression
	if (type_guard != symtable_element::BOOLEAN){
		register_error(std::string("Error: non-boolean guard of an if expression ."));
	}
	// To check rules that depend on this information...
	into_for_or_while = true;
	node.body->accept(*this);
	into_for_or_while = false;
}

void semantic_analysis::visit(const node_return_statement& node) {
	symtable_element::id_type type_guard;
	std::cout << "Accessing return statement" << std::endl;

	if(node.expression != nullptr) {
		// Rule 8: a return statement must have an associated expression
		// only if the method returns a value
		// TODO: necesito poder consultar de forma transparente
		// el tipo de retorno del método que define el scope actual

		// Rule 9: the type of the value returned from the method must
		// be the same than the type of the expression of the return statement
		// TODO: cómo chequeamos esto?
		node.expression->accept(*this);
		type_expr = type_l_expr;

	}
	else{
		// {node.expression == nullptr}
		// TODO: chequear que el retorno del método sea void
	}
}

void semantic_analysis::visit(const node_break_statement& node) {
	std::cout << "Accessing break statement" << std::endl;
	// Rule 19: break and continue, only into a loop's body.
	if (!into_for_or_while){
		register_error(std::string("Break statement outside a loop."));
	}
}

void semantic_analysis::visit(const node_continue_statement& node) {
	std::cout << "Accessing continue statement" << std::endl;
	// Rule 19: break and continue, only into a loop's body.
	if (!into_for_or_while){
		register_error(std::string("Continue statement outside a loop."));
	}
}

void semantic_analysis::visit(const node_skip_statement& node) {
	std::cout << "Accessing skip statement" << std::endl;
}

void semantic_analysis::visit(const node_int_literal& node) {
	std::cout << "Accessing int literal of value " << node.value << std::endl;
	// TODO: acordarse de cambiar el tipo de valor retornado...
	return symtable_element::INTEGER;
}

void semantic_analysis::visit(const node_float_literal& node) {
	std::cout << "Accessing float literal of value " << node.value << std::endl;
	return symtable_element::FLOAT;
}

void semantic_analysis::visit(const node_bool_literal& node) {
	std::cout << "Accessing boolean literal of value " << node.value << std::endl;
	return symtable_element::BOOLEAN;
}

void semantic_analysis::visit(const node_string_literal& node) {
	std::cout << "Accessing float literal of value " << node.value << std::endl;
	return symtable_element::STRING; // TODO: ?
}

void semantic_analysis::visit(const node_binary_operation_expr& node) {
	symtable_element::id_type ret;
	// TODO: no nos convendría aquí tener un nodo especial para
	// cada tipo de operacion?
	symtable_element::id_type l_op_type = node.left->accept(*this);
	symtable_element::id_type r_op_type = node.right->accept(*this);
	std::cout << "Accessing binary operation" << std::endl;


	// Rule 13: operands of arithmetic and relational operations,
	// must have type int or float
	if (node.oper != EQUAL && node.oper != DISTINCT &&
		node.oper != AND && node.oper != OR){

		if ((l_op_type == symtable_element::INTEGER ||
			 l_op_type == symtable_element::FLOAT)
							&&
			(r_op_type == symtable_element::INTEGER ||
			 r_op_type == symtable_element::FLOAT)){

			// TODO: hay conversión implícita de tipos?
			// TODO: determinar el tipo a retornar

		}
		else{
			register_error(std::string("Non-numeric operands of arithmetic or relational operation."));
		}
	}
	else{
		// {node.oper == EQUAL || node.oper == DISTINCT ||
		//  node.oper == AND || node.oper == OR}

		// Rule 14: eq_op operands must have the same type (int, float or boolean)
		if (node.oper == EQUAL || node.oper == DISTINCT){
			if (l_op_type != r_op_type ||
				(l_op_type != symtable_element::INTEGER &&
				 l_op_type != symtable_element::FLOAT &&
				 l_op_type != symtable_element::BOOLEAN)){

				register_error(std::string("eq_op operands with different or wrong types."));
			}
		}
		else{
			// {node.oper == AND || node.oper == OR}
			// Rule 15: cond_op and ! operands, must evaluate to a boolean
			if (l_op_type != r_op_type || l_op_type != symtable_element::BOOLEAN){
				register_error(std::string("cond_op operands must evaluate to boolean."));
			}
		}
	}

	// TODO: setear el valor de ret!
	return ret;
}

void semantic_analysis::visit(const node_location& node) {
	symtable_element aux; // TODO: debería usar un puntero, para no estar creando
							 // instancias que no me hacen falta?
	symtable_element::id_type type_index, type_id;
	// Rule 10: the location must be already declared
	// TODO: necesito un método rápido para preguntar eso
	// TODO: tenemos que consultar la tabla con los distintos
	// id, hasta determinar el tipo. Haría falta un único método
	// de consulta, para poder repetirlo con los distintos ids
	// hasta llegar al último.
	if (__DEBUG)
		std::cout << "Accessing location expression" << std::endl;

	//for(auto r : node.ids) {
	//	std::cout << "Accessing reference " << r << std::endl;
	//	// TODO: ir indexando la tabla de símbolos hasta dar con el tipo
	//	// del último id, y asignarlo a type_id
	//}
	aux = s_table.get(node.ids[0]);
	// Rule 2: declaration before use
	if (aux.get_class == symtable_element::NOT_FOUND){
		register_error(std::string("Id "+r+" must be declared before use."));
	}
	else{
		// INV : {aux is the symtable_element for the id number i-1 in node.ids}
		for(std::vector<int>::size_type i = (std::vector<int>::size_type) 1;
				i < node.ids.size(); i++){
			// TODO: asumo que en ids se guardan los identificadores en orden
			// desde 0 (el primer id de la izquierda), hacia la derecha...
			if (__DEBUG)
				std::cout << "Accessing reference " << r << std::endl;

			// Index the previous symtable_element
			aux = aux.get(node.ids[i]);
			// Rule 2: declaration before use
			if (aux.get_class == symtable_element::NOT_FOUND){
				register_error(std::string("Id "+r+" must be declared before use."));
				// TODO: cortamos esto acá?
				break;
			}
		}
	}
	// {aux is the symtable_element that represents the object referenced by
	// the location}
	// TODO: necesito alguna manera
	if (node.array_idx_expr != NULL){
		// Rule 11: if the location is an array position,
		// the corresponding id must point to an array, and the
		// index must be an integer
		type_index = node.array_idx_expr->accept(*this);
		// TODO: cambiar symtable_element::ARRAY por lo que corresponda
		if (type_id != symtable_element::ARRAY ||
			type_index != symtable_element::INTEGER){
			// TODO: podría hacer falta un visitor que me genere un string bonito
			// que represente el texto asociado a esta porción del ast, para
			// informar mejor sobre la location y el index?
			std::cerr << "Error: array assignment with wrong id or index." << std::endl;
			// The process is not interrumpted. We just count the error.
			error += 1;
		}
	}

	return type_id;
}

void semantic_analysis::visit(const node_negate_expr& node) {
	std::cout << "Accessing negate expression" << std::endl;
	// TODO: notar que sí tenemos un nodo especial para esta
	// situación.
	symtable_element::id_type type_expr = node.expression->accept(*this);

	// Rule 15: cond_op and ! operands, must evaluate to a boolean
	if (type_expr != symtable_element::BOOLEAN){
		std::cerr << "Error: non-boolean operand for a cond_op." << std::endl;
		// The process is not interrumpted. We just count the error.
		error += 1;
	}

	return type_expr;
}

void semantic_analysis::visit(const node_negative_expr& node) {
	std::cout << "Accessing negative expression" << std::endl;
	// TODO: notar que sí tenemos un nodo especial para esta
	// situación.
	symtable_element::id_type type_expr = node.expression->accept(*this);

	if (type_expr != symtable_element::INTEGER && type != symtable_element::FLOAT){
		// Rule 13: operands of arithmetic and relational operations,
		// must have type int or float
		std::cerr << "Error: non-numeric operand for a negation operation." << std::endl;
		// The process is not interrumpted. We just count the error.
		error += 1;
	}

	return type_expr;
}

void semantic_analysis::visit(const node_parentheses_expr& node) {
	std::cout << "Accessing parentheses expression" << std::endl;

	return node.expression->accept(*this);
}
*/
