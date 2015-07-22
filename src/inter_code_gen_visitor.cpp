#include "inter_code_gen_visitor.h"

inter_code_gen_visitor::inter_code_gen_visitor() {
	inst_list = new instructions_list();
	offset = 0;
	into_method = false;
	next_temp = 1;

	// TODO: quizás deberíamos colocar todas estas constantes en un archivo
	// separado
	// Initial values of attributes.
	integer_initial_value = 0;
	float_initial_value = 0.0;
	boolean_initial_value = true; // TODO: true?

	// Type's width (in bytes).
	// TODO: colocar el ancho correcto
	integer_width = 4;
	float_width = 4;
	boolean_width = 4;
}

const instructions_list* inter_code_gen_visitor::get_inst_list() {
	return inst_list;
}

unsigned int inter_code_gen_visitor::calculate_type(Type type){
	unsigned int ret = 0;

	switch(type.type){
		case Type::INT:
			ret = integer_width;
			break;

		case Type::FLOAT:
			ret = float_width;
			break;

		case Type::BOOLEAN:
			ret = boolean_width;
			break;

		case Type::ID:
			// TODO: indexar la tabla de símbolos con el id, y extraer su tamaño.
			break;
	}

	return ret;
}

void inter_code_gen_visitor::instance_initialization(std::string id_class){
	// Translate instance declaration into a sequence of code to
	// initialize each attribute.
	symtable_element *class_object = s_table.get(id_class);
	std::list<symtable_element> *fields = class_object->get_class_fields();

	for(std::list<symtable_element>::iterator it = fields->begin();
	it != fields->end(); ++it){
		if((*it).get_class() == symtable_element::T_VAR){
			if((*it).get_type() == symtable_element::INTEGER){
				// TODO: generar nuevo identificador, colocarlo en
				// la tabla, con offset = offset+integer_width
				address_pointer constant = new_integer_constant(
											integer_initial_value);

				inst_list->push_back(new_copy(f->id, constant));
			}
			else if((*it).get_type() == symtable_element::FLOAT){
				// TODO: generar nuevo identificador, colocarlo en
				// la tabla, con offset = offset+float_width
				address_pointer constant = new_float_constant(
											float_initial_value);

				inst_list->push_back(new_copy(f->id, constant));

			}
			else if((*it).get_type() == symtable_element::BOOLEAN){
				// TODO: generar nuevo identificador, colocarlo en
				// la tabla, con offset = offset+boolean_width
				address_pointer constant = new_boolean_constant(
											boolean_initial_value);

				inst_list->push_back(new_copy(f->id, constant));
			}
			else if((*it).get_type() == symtable_element::ID){
				instance_initialization((*it).get_key());
			}
		}
	}
}

/* _ El código intermedio debería alterar cada identificador, para que sea
 * único, asistiendose con una tabla de símbolos, que va a generar el
 * id único.
 * _ Deberíamos guardar el offset de las variables, que calcula nuestro
 * visitor.
 * _ Las temporales van siempre a registros, y yo, en el visitor, invento
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
	// TODO: divido en métodos y atributos, para poder
	// realizar la traducción de forma ordenada. Sería
	// mejor manejar la gramática original?
	std::list<class_block_pointer> fields;
	std::list<class_block_pointer> methods;
	for(auto cb : node.class_block) {
		if (cb->is_node_field_decl()){
			//node_field_decl& aux = static_cast<node_field_decl&> (*cb);
			//aux.accept(*this);
			fields.push_back(cb);
		}
		else{
			// {not cb->is_node_field_decl()}
			//node_method_decl& aux = static_cast<node_method_decl&> (*cb);
			//aux.accept(*this);
			methods.push_back(cb);
		}
	}
	// Saving information of fields..
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
	// TODO: tengo que guardar ese valor en algún lugar...

	// Translation of methods.
	for(std::list<class_block_pointer>::iterator it = methods.begin();
	it != methods.end(); ++it){
		node_method_decl& aux = static_cast<node_method_decl&> (*(*it));
		aux.accept(*this);
	}

	// Restore offset value.
	offset = prev_offset;
}

	// Statements
	void inter_code_gen_visitor::visit(const node_method_call_statement& node) {}
	void inter_code_gen_visitor::visit(const node_if_statement& node) {}
	void inter_code_gen_visitor::visit(const node_for_statement& node) {}
	void inter_code_gen_visitor::visit(const node_while_statement& node) {}
	void inter_code_gen_visitor::visit(const node_return_statement& node) {}
	void inter_code_gen_visitor::visit(const node_break_statement& node) {}
	void inter_code_gen_visitor::visit(const node_continue_statement& node) {}
	void inter_code_gen_visitor::visit(const node_skip_statement& node) {}
	// Expressions
	void inter_code_gen_visitor::visit(const node_int_literal& node) {}
	void inter_code_gen_visitor::visit(const node_float_literal& node) {}
	void inter_code_gen_visitor::visit(const node_bool_literal& node){}
	void inter_code_gen_visitor::visit(const node_string_literal& node){}
	void inter_code_gen_visitor::visit(const node_binary_operation_expr& node) {}
	void inter_code_gen_visitor::visit(const node_location& node) {}
	void inter_code_gen_visitor::visit(const node_negate_expr& node) {}
	void inter_code_gen_visitor::visit(const node_negative_expr& node) {}
	void inter_code_gen_visitor::visit(const node_parentheses_expr& node) {}
	void inter_code_gen_visitor::visit(const node_method_call_expr& node) {}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_field_decl& node) {
	// TODO: podríamos utilizar Symbolic Type Widths (pagina 386, libro dragon)
	#ifdef __DEBUG
		std::cout << "Translating field declaration." << std::endl;
	#endif

	// TODO: acá debería usar offsets en vez de ids?
	for(auto f : node.ids) {
		// Translate each identifier. Add the identifier to the symbol table.
		if (node.type.type == Type::ID){
			// Declaration of instances.
			if(into_method){
				instance_initialization(node.type.id);
			}
			else{
				// {not into_method}
				// Is an attribute declaration. We just save the corresponding
				// data, for future use.
				// TODO: guardar el campo en la tabla, para poder consultarlo
				// al momento de crear una instancia de esta clase
			}
		}
		else{
			// {node.type.type != Type::ID}
			// Basic type or array
			if (f->array_size > 0){
				// Array
				switch(node.type.type){
					case Type::INTEGER:
						if(into_method){
							// TODO: agregar a la tabla de símbolos f->id junto con
							// el valor actual del offset
							offset += f->array_size*integer_width;
						}
						else{
							// {not into_method}
							// TODO: guardar en la tabla de símbolos esta
							// información
						}
						break;

					case Type::FLOAT:
						if (into_method){
							// TODO: agregar a la tabla de símbolos f->id junto con
							// el valor actual del offset
							offset += f->array_size*float_width;
						}
						else{
							// {not into_method}
							// TODO: guardar en la tabla de símbolos esta
							// información
						}
						break;

					case Type::BOOLEAN:
						if (int_method){
							// TODO: agregar a la tabla de símbolos f->id junto con
							// el valor actual del offset
							offset += f->array_size*boolean_width;
						}
						else{
							// {not into_method}
							// TODO: guardar en la tabla de símbolos esta
							// información
						}
						break;
				}
			}
			else{
				// {f->array_size < 0}
				// Basic type
				if (f->array_size > 0){
					switch(node.type.type){
						case Type::INTEGER:
							if (into_method){
								// TODO: agregar a la tabla de símbolos f->id junto con
								// el valor actual del offset
								offset += integer_width;
							}
							else{
								// {not into_method}
								// TODO: guardar en la tabla de símbolos esta
								// información
							}
							break;

						case Type::FLOAT:
							if (into_method){
								// TODO: agregar a la tabla de símbolos f->id junto con
								// el valor actual del offset
								offset += float_width;
							}
							else{
								// {not into_method}
								// TODO: guardar en la tabla de símbolos esta
								// información
							}
							break;

						case Type::BOOLEAN:
							if (into_method){
								// TODO: agregar a la tabla de símbolos f->id junto con
								// el valor actual del offset
								offset += boolean_width;
							}
							else{
								// {not into_method}
								// TODO: guardar en la tabla de símbolos esta
								// información
							}
							break;
					}
				}
			}
		}
	}
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_id& node) {
	// This visitor does not need to do some task into node_id.
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_method_decl& node){
#ifdef __DEBUG
	std::cout << "Translating method " << node.id << std::endl;
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
	aux->push_back(new_beggin_procedure(offset));
	// TODO: agregar a aux todas las instrucciones en inst_list y hacer que
	// inst_list apunte a aux
	offset = offset_prev;
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_parameter_identifier& node) {
#ifdef __DEBUG
	std::cout << "Translating parameter " << node.id << std::endl;
#endif

	// TODO: en la arquitectura x86_64, es el método el que construye
	// el stack frame?: "The following enter instruction
	// sets up the stack frame" (de "x86-64-architecture-guide"). Pero es quien
	// lo llama quien coloca los parametros en los registros. Debemos contabilizar
	// el tamaño de los parámetros?
	offset += calculate_size(node.type);
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_body& node) {
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

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_block& node) {
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

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_assignment_statement& node) {
#ifdef __DEBUG
	std::cout << "Translating assignment statement" << std::endl;
#endif
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
			inst_list->push_back(new_plus_assign(l_value, r_value));
			break;

		case AssignOper::MINUS_ASSIGN:
			inst_list->push_back(new_minus_assign(l_value, r_value));
	}
}
/*
void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_location& node) {
#ifdef __DEBUG
	std::cout << "Translating location expression" << std::endl;
#endif

	// TODO: lo deberíamos convertir a algo de la forma offset(base,index,scale)?
	// no está eso dependiendo de la arq. x86_64, siendo que no es la idea del
	// código intermedio?: vamos generando las direcciones

}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_int_literal& node) {
#ifdef __DEBUG
	std::cout << "Translating int literal of value " << node.value << std::endl;
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
	last_expr = std::string("$"+node.value)
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_bool_literal& node) {
#ifdef __DEBUG
	std::cout << "Translating bool literal of value " << node.value << std::endl;
#endif
	// TODO: convertir a string
	last_expr = std::string("$"+node.value)
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_float_literal& node) {
#ifdef __DEBUG
	std::cout << "Translating FLOAT literal of value " << node.value << std::endl;
#endif
	// TODO: convertir a string
	last_expr = std::string("$"+node.value)
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_string_literal& node) {
#ifdef __DEBUG
	std::cout << "Translating string literal of value " << node.value << std::endl;
#endif
	// TODO: representación de literales string?
	last_expr = std::string("$"+node.value)
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_method_call& node) {
#ifdef __DEBUG
		std::cout << "Translating method call statement" << std::endl;
#endif


	for(auto r : node.parameters){
		expr_call_appropriate_accept(r);
		// TODO: generar la instrucción correcta, con el address correcto,
		// que sería aquel que se almacenó en un atributo de la instancia,
		// tras la llamada a accept de r.
		inst_list->push_back("PARAM address");
	}
	// TODO: procedure name?
	// TODO: generar una nueva dirección para RETURN_VALUE_ADDRESS
	inst_list->push_back("CALL PROCEDURE_NAME" + node.parameters.size() + "RETURN_VALUE_ADDRESS");

}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_if_statement& node){
#ifdef __DEBUG
		std::cout << "Translating if statement" << std::endl;
#endif
	expr_call_appropriate_accept(node.expression);
	// TODO: la visita a node.expression, guarda en un atributo de la instancia
	// el valor de la referencia (TEMPORARY_VARIABLE) en donde se almacena el
	// resultado de evaluar node.expression
	// TODO: L1 y L2 son 2 etiquetas de valores fijos, a definir
	inst_list->push_back("IFFALSE TEMPORARY_VARIABLE L1");
	// "then" branch code.
	stm_call_appropriate_accept(node.then_statement);
	inst_list->push_back("GOTO L2");
	inst_list->push_back("LABEL L1");
	if(node.else_statement != nullptr) {
		// "else" branch code.
		stm_call_appropriate_accept(node.else_statement);
	}
	inst_list->push_back("LABEL L2");
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_for_statement& node){
#ifdef __DEBUG
		std::cout << "Translating for statement" << std::endl;
#endif

	expr_call_appropriate_accept(node.from);
	// TODO: x es la variable sobre la que itera el ciclo, from es la dirección
	// en donde se almacenó la evaluación de node.from
	inst_list->push_back("ASSIGN x from");
	expr_call_appropriate_accept(node.to);
	// TODO: L1 es una etiqueta a definir, de valor fijo.
	inst_list->push_back("LABEL L1");
	// TODO: to es el lugar en donde se almacenó la evaluación de node.to, y
	// z es al lugar en donde se almacenará el resultado de la comparación
	inst_list->push_back("LTE x to z");
	// TODO: L2 es una etiqueta a definir, de valor fijo.
	inst_list->push_back("IFFALSE z L2");
	// Body code.
	stm_call_appropriate_accept(node.body);
	inst_list->push_back("ASSIGNPLUS x 1");
	inst_list->push_back("GOTO L1");
	inst_list->push_back("LABEL L2");
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_while_statement& node) {
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

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_return_statement& node) {
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

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_break_statement& node) {
#ifdef __DEBUG
	std::cout << "Translating break statement" << std::endl;
#endif

	// Rule 19: break and continue, only into a loop's body.
	if (!into_for_or_while){
		register_error(std::string("Break statement outside a loop."),
				ERROR_19);
	}
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_continue_statement& node) {
#ifdef __DEBUG
	std::cout << "Translating continue statement" << std::endl;
#endif
	// Rule 19: break and continue, only into a loop's body.
	if (!into_for_or_while){
		register_error(std::string("Continue statement outside a loop."),
				ERROR_19);
	}
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_skip_statement& node) {
#ifdef __DEBUG
	std::cout << "Translating skip statement" << std::endl;
#endif
}

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_binary_operation_expr& node) {
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

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_negate_expr& node) {
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

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_negative_expr& node) {
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

void inter_code_gen_visitor::inter_code_gen_visitor::visit(const node_parentheses_expr& node) {
#ifdef __DEBUG
	std::cout << "Translating parentheses expression" << std::endl;
#endif

	expr_call_appropriate_accept(node.expression);
}

*/
