#include "inter_code_gen_visitor.h"

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

		#ifdef __DEBUG
		default:
			assert(false);
		#endif
	}

	return ret;
}

void inter_code_gen_visitor::stm_call_appropriate_accept(statement_pointer s){
	switch (s->type_of_statement()){
		case node_statement::field_decl:{
				node_field_decl& aux = static_cast<node_field_decl&> (*s);
				aux.accept(*this);
				break;
		}

		case node_statement::block:{
				node_block& aux = static_cast<node_block&> (*s);
				aux.accept(*this);
				break;
		}

		case node_statement::assignment_statement:{
				node_assignment_statement& aux = static_cast<node_assignment_statement&> (*s);
				aux.accept(*this);
				break;
		}

		case node_statement::method_call_statement:{
			node_method_call& aux = static_cast<node_method_call&> (*s);
			aux.accept(*this);
			break;
		}

		case node_statement::if_statement:{
			node_if_statement& aux = static_cast<node_if_statement&> (*s);
			aux.accept(*this);
			break;
		}

		case node_statement::for_statement:{
			node_for_statement& aux = static_cast<node_for_statement&> (*s);
			aux.accept(*this);
			break;
		}

		case node_statement::while_statement:{
			node_while_statement& aux = static_cast<node_while_statement&> (*s);
			aux.accept(*this);
			break;
		}

		case node_statement::return_statement:{
			node_return_statement& aux = static_cast<node_return_statement&> (*s);
			aux.accept(*this);
			break;
		}

		case node_statement::break_statement:{
			node_break_statement& aux = static_cast<node_break_statement&> (*s);
			aux.accept(*this);
			break;
		}

		case node_statement::continue_statement:{
			node_continue_statement& aux = static_cast<node_continue_statement&> (*s);
			aux.accept(*this);
			break;
		}

		case node_statement::skip_statement:{
			node_skip_statement& aux = static_cast<node_skip_statement&> (*s);
			aux.accept(*this);
		}
	}
}

void inter_code_gen_visitor::expr_call_appropriate_accept(expr_pointer s){
	switch (s->type_of_expression()){
		case node_expr::int_literal:{
				node_int_literal& aux = static_cast<node_int_literal&> (*s);
				aux.accept(*this);
				break;
		}

		case node_expr::bool_literal:{
				node_bool_literal& aux = static_cast<node_bool_literal&> (*s);
				aux.accept(*this);
				break;
		}

		case node_expr::string_literal:{
				node_string_literal& aux = static_cast<node_string_literal&> (*s);
				aux.accept(*this);
				break;
		}

		case node_expr::binary_operation_expr:{
			node_binary_operation_expr& aux = static_cast<node_binary_operation_expr&> (*s);
			aux.accept(*this);
			break;
		}

		case node_expr::location:{
			node_location& aux = static_cast<node_location&> (*s);
			aux.accept(*this);
			break;
		}

		case node_expr::negate_expr:{
			node_negate_expr& aux = static_cast<node_negate_expr&> (*s);
			aux.accept(*this);
			break;
		}

		case node_expr::negative_expr:{
			node_negative_expr& aux = static_cast<node_negative_expr&> (*s);
			aux.accept(*this);
			break;
		}

		case node_expr::parentheses_expr:{
			node_parentheses_expr& aux = static_cast<node_parentheses_expr&> (*s);
			aux.accept(*this);
			break;
		}

		case node_expr::method_call_expr:{
			node_method_call& aux = static_cast<node_method_call&> (*s);
			aux.accept(*this);
			break;
		}
	}
}


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
	symtable_element new_class(node.id, new std::list<symtable_element>());
	// Define a new scope
#ifdef __DEBUG
	assert(s_table.put_class(node.id, new_class) == symtables_stack::CLASS_PUT);
#else
	s_table.put_class(node.id, new_class);
#endif
	// TODO: qué código generamos para la declaración de clase?
	for(auto cb : node.class_block) {
		if (cb->is_node_field_decl()){
			node_field_decl& aux = static_cast<node_field_decl&> (*cb);
			aux.accept(*this);
		}
		else{
			// {not cb->is_node_field_decl()}
			node_method_decl& aux = static_cast<node_method_decl&> (*cb);
			aux.accept(*this);
		}
	}
	// Close the scope introduced by the class
	s_table.finish_class_analysis();
}

void inter_code_gen_visitor::visit(const node_field_decl& node) {
	symtable_element *id = nullptr;

	#ifdef __DEBUG
	 	 std::cout << "Translating field declaration." << std::endl;
	#endif

	for(auto f : node.ids) {
		// Translate each identifier. Add the identifier to the symbol table.
		if (node.type.type == Type::ID){
			// Declaration of instances.
			// TODO: crear objeto instrucción apropiado
			inst_list->push_back("CREATE_ID" + f->id + node.type.id);
			// Create a new symtable_element object, that represents
			// an object, with the id and the name of the corresponding class.
			id = new symtable_element(f->id, node.type.id);
		}
		else{
			// {node.type.type != Type::ID}
			// Basic type or array
			if (f->array_size > 0){
				// Array
				// TODO: crear objeto instrucción apropiado. Estudiar
				// el valor de node.type.id, para elegir la instrucción correcta
				// (entre CREATE_INT_ARRAY, CREATE_FLOAT_ARRAY y
				// CREATE_BOOLEAN_ARRAY)
				inst_list->push_back("CREATE_INT_ARRAY" + f->id + f->array_size);
				id = new symtable_element(f->id, determine_type(node.type.type),
					  f->array_size);
			}
			else{
				// {f->array_size < 0}
				// Basic type
				// TODO: crear objeto instrucción apropiado. Estudiar
				// el valor de node.type.id, para elegir la instrucción correcta
				// (entre CREATE_INT, CREATE_FLOAT y
				// CREATE_BOOLEAN)
				inst_list->push_back("CREATE_INT" + f->id);
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

void inter_code_gen_visitor::visit(const node_id& node) {
	// This visitor does not need to do some task into node_id.
}

void inter_code_gen_visitor::visit(const node_method_decl& node){
#ifdef __DEBUG
	std::cout << "Translating method " << node.id << std::endl;
#endif
	symtable_element method(node.id, determine_type(node.type.type),
							new std::list<symtable_element>());
	// TODO: quitar esto cuando se resuelva lo de la tabla
	into_method = true;
	actual_method = &method;
	// Add to the symbol table, define a new scope
#ifdef __DEBUG
	assert(s_table.put_func(node.id, method) == symtables_stack::FUNC_PUT);
#else
	s_table.put_func(node.id, method);
#endif

	// TODO: crear objeto instrucción apropiado. quizás un label como
	// nombre_de_clase::nombre_de_metodo para que el label sea único?
	inst_list->push_back("CREATE_INT_ARRAY" + node.id);

	for(auto p : node.parameters) {
		p->accept(*this);
	}

	node.body->accept(*this);
	// Close the scope defined by the method.
	s_table.finish_func_analysis();
	// TODO: quitar esto cuando se resuelva lo de la tabla
	into_method = false;
	actual_method = nullptr;
}

void inter_code_gen_visitor::visit(const node_parameter_identifier& node) {
	#ifdef __DEBUG
		std::cout << "Translating parameter " << node.id << std::endl;
	#endif

	symtable_element param(node.id, determine_type(node.type.type));
	s_table.put_func_param(node.id, param);

	// TODO: instrucción a generar para este caso?
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
}

void inter_code_gen_visitor::visit(const node_assignment_statement& node) {
	#ifdef __DEBUG
		std::cout << "Translating assignment statement" << std::endl;
	#endif

	snode.location->accept(*this);
	expr_call_appropriate_accept(node.expression);
	// TODO: crear objeto instrucción apropiado. La visita a node.location
	// y node.expression deberían guardar en un atributo de la instancia
	// una representación de la location y una direccion en donde se
	// guarda la expression, de forma que
	// podamos disponer aquí de esa información, para construir la instrucción
	// adecuada.
	switch(node.oper){
		case AssignOper::ASSIGN:
			inst_list->push_back("ASSIG LOCATION  ADDRESS_EXPRESSION");
			break

		case AssignOper::PLUS_ASSIGN:
			inst_list->push_back("ASSIGPLUS LOCATION  ADDRESS_EXPRESSION");
			break

		case AssignOper::MINUS_ASSIGN:
			inst_list->push_back("ASSIGMINUS LOCATION  ADDRESS_EXPRESSION");
			break
	}
}

void inter_code_gen_visitor::visit(const node_location& node) {
#ifdef __DEBUG
	std::cout << "Translating location expression" << std::endl;
#endif

	// TODO: generar una representación adecuada de la location
	// y guardarla en un atributo de la instancia actual, para ser
	// empleado por otros métodos
}

void inter_code_gen_visitor::visit(const node_int_literal& node) {
#ifdef __DEBUG
	std::cout << "Translating int literal of value " << node.value << std::endl;
#endif
	// TODO: generar una representación adecuada de la location
	// y guardarla en un atributo de la instancia actual, para ser
	// empleado por otros métodos
}

void inter_code_gen_visitor::visit(const node_bool_literal& node) {
#ifdef __DEBUG
	std::cout << "Translating bool literal of value " << node.value << std::endl;
#endif
	// TODO: guardar el literal en una nueva dirección A, generar la instrucción
	// adecuada para ello, y guardar en un atributo de la instancia la dirección
	// A
}

void inter_code_gen_visitor::visit(const node_float_literal& node) {
#ifdef __DEBUG
	std::cout << "Translating FLOAT literal of value " << node.value << std::endl;
#endif
	// TODO: guardar el literal en una nueva dirección A, generar la instrucción
	// adecuada para ello, y guardar en un atributo de la instancia la dirección
	// A
}

void inter_code_gen_visitor::visit(const node_string_literal& node) {
#ifdef __DEBUG
	std::cout << "Translating string literal of value " << node.value << std::endl;
#endif
	// TODO: guardar el literal en una nueva dirección A, generar la instrucción
	// adecuada para ello, y guardar en un atributo de la instancia la dirección
	// A

void inter_code_gen_visitor::visit(const node_method_call& node) {
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

void inter_code_gen_visitor::visit(const node_if_statement& node){
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

void inter_code_gen_visitor::visit(const node_for_statement& node){
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

