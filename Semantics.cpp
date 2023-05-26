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
		s_table->add_symbol(arg->name, std::make_shared<Symbol>(arg->pos, arg->type, arg->name));
	}
	func->body->visitChildren(&type_visitor, s_table);
}

bool Semantics::type_visitor(ASTNode *node, std::any &ctx) {
	auto s_table = std::any_cast<SymbolTable*>(ctx);

	if (auto def = dynamic_cast<DefNode*>(node)) {
		auto new_table = new SymbolTable(s_table);
		new_table->add_symbol(def->name, std::make_shared<Symbol>(def->pos, def->type, def->name));
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

