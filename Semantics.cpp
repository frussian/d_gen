//
// Created by Anton on 25.05.2023.
//

#include "Semantics.h"
#include "BuildError.h"
#include "SymbolTable.h"

Semantics::Semantics(FunctionNode *func): func(func) {}

void Semantics::eliminate_unreachable_code() {
	eliminate_unreachable_code_visit_body(func->body);
}

void Semantics::eliminate_unreachable_code_visit_body(BodyNode *body) {
	for (int i = 0; i < body->stmts.size(); i++) {
		auto stmt = body->stmts[i];
		if (dynamic_cast<ContinueNode*>(stmt) ||
		dynamic_cast<BreakNode*>(stmt) ||
		dynamic_cast<ReturnNode*>(stmt)) {
			body->stmts.resize(i+1);
			return;
		}
		if (auto loop = dynamic_cast<ForNode*>(stmt)) {
			eliminate_unreachable_code_visit_body(loop->body);
		} else if (auto cond = dynamic_cast<IfNode*>(stmt)) {
			eliminate_unreachable_code_visit_body(cond->body);
			if (cond->else_body) {
				eliminate_unreachable_code_visit_body(cond->else_body);
			}
		}
	}
}

void Semantics::connect_loops() {
	connect_loops_visit_body(func->body, nullptr);
}

void Semantics::connect_loops_visit_body(BodyNode *body, ForNode *loop) {
	for (auto stmt : body->stmts) {
		if (auto cont = dynamic_cast<ContinueNode*>(stmt)) {
			if (!loop) {
				throw BuildError(Err{cont->pos, "cannot match a continue node to a loop"});
			}
			cont->loop = loop;
			return;
		} else if (auto br = dynamic_cast<BreakNode*>(stmt)) {
			if (!loop) {
				throw BuildError(Err{br->pos, "cannot match a break node to a loop"});
			}
			br->loop = loop;
			return;
		} else if (auto f_loop = dynamic_cast<ForNode*>(stmt)) {
			connect_loops_visit_body(f_loop->body, f_loop);
		} else if (auto cond = dynamic_cast<IfNode*>(stmt)) {
			connect_loops_visit_body(cond->body, loop);
			if (cond->else_body) {
				connect_loops_visit_body(cond->else_body, loop);
			}
		}
	}
}

void Semantics::type_ast() {
	auto *s_table = new SymbolTable;
	for (const auto &arg: func->args) {
		s_table->add_symbol(arg->name, Symbol::create_symbol(arg->pos, arg->type, arg->name, true));
	}
	func->body->visitChildren(&type_visitor, s_table);
}

bool Semantics::type_visitor(ASTNode *node, std::any &ctx) {
	auto s_table = std::any_cast<SymbolTable*>(ctx);

	if (auto def = dynamic_cast<DefNode*>(node)) {
		auto new_table = new SymbolTable(s_table);
		auto sym = Symbol::create_symbol(def->pos, def->type, def->name);
		def->sym = sym;
		new_table->add_symbol(def->name, sym);
		ctx = new_table;
	} else if (auto ident = dynamic_cast<IdentNode*>(node)) {
		auto symbol = s_table->find_symbol(ident->name);
		if (!symbol) {
			throw BuildError(Err{ident->pos, "undefined symbol " + ident->name});
		}
		std::cout << "connect symbol " << ident->name << " at " << ident->pos <<
		" to symbol defined at " << symbol->pos << std::endl;
		ident->symbol = symbol;
	}

	return true;
}

void Semantics::type_check() {
	func->visitChildren(&type_check_visitor, func);
}

bool Semantics::type_check_visitor(ASTNode *node, std::any &ctx) {
	auto func = std::any_cast<FunctionNode*>(ctx);
	if (auto pre_cond = dynamic_cast<PrecondNode*>(node)) {
		type_check_precondition(pre_cond);
		return false;
	} else if (auto ret = dynamic_cast<ReturnNode*>(node)) {
		auto t = ret->expr->get_type();
		if (t != func->ret_type) {
			throw BuildError(Err{ret->pos, "ret type is different, expected " +
				func->ret_type.to_string() + ", got " +
				t.to_string()});
		}
		return false;
	} else if (auto asg = dynamic_cast<AsgNode*>(node)) {
		if (auto ident = dynamic_cast<IdentNode*>(asg->lhs)) {
			if (ident->symbol->is_input) {
				throw BuildError(Err{ident->pos, "can not assign to input variable"});
			}
		} else if (auto arr_lookup = dynamic_cast<ArrLookupNode*>(asg->lhs)) {
			if (arr_lookup->ident->symbol->is_input) {
				throw BuildError(Err{arr_lookup->pos, "can not assign to input variable"});
			}
		}
		type_check_asg(asg);
		return false;
	} else if (auto def = dynamic_cast<DefNode*>(node)) {
		if (!def->rhs) {
			return false;
		}
		auto t = def->rhs->get_type();
		if (def->type != t) {
			throw BuildError(Err{def->pos, "define statement have different types declared and evaluated: " +
										   def->type.to_string() + " and " +
										   t.to_string()});
		}
		return false;
	} else if (auto loop = dynamic_cast<ForNode*>(node)) {
		if (loop->precond) {
			type_check_precondition(loop->precond);
		}
		if (loop->pre_asg) {
			type_check_asg(loop->pre_asg);
		}
		if (loop->inc_asg) {
			type_check_asg(loop->inc_asg);
		}
		auto cond_t = loop->cond->get_type();
		if (cond_t != TypeKind::BOOL) {
			throw BuildError(Err{loop->pos,
								 "loop stop condition must be evaluated to bool, got " +
								 cond_t.to_string()});
		}
		loop->body->visitChildren(&type_check_visitor, func);
		return false;
	} else if (auto if_node = dynamic_cast<IfNode*>(node)) {
		if (if_node->precond) {
			type_check_precondition(if_node->precond);
		}
		auto cond_t = if_node->cond->get_type();
		if (cond_t != TypeKind::BOOL) {
			throw BuildError(Err{if_node->pos,
								 "if operator condition must be evaluated to bool, got " +
								 cond_t.to_string()});
		}
		if_node->body->visitChildren(&type_check_visitor, func);
		if (if_node->else_body) {
			if_node->else_body->visitChildren(&type_check_visitor, func);
		}
		return false;
	}

	return true;
}

void Semantics::type_check_precondition(PrecondNode *pre_cond) {
	auto t = pre_cond->expr->get_type();
	if (t != TypeKind::BOOL) {
		throw BuildError(Err{pre_cond->pos, "precondition must be evaluated to bool"});
	}
}

void Semantics::type_check_asg(AsgNode *asg) {
	auto lhs_t = asg->lhs->get_type();
	auto rhs_t = asg->rhs->get_type();
	if (! (lhs_t == rhs_t || lhs_t.is_numerical() && rhs_t.is_numerical()) ) {
		throw BuildError(Err{asg->pos, "assignment sides have different types: " +
									   lhs_t.to_string() + " and " +
									   rhs_t.to_string()});
	}
}