//
// Created by Anton on 25.05.2023.
//

#include "Semantics.h"

void Semantics::eliminate_unreachable_code(FunctionNode *func) {
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


