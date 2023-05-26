//
// Created by Anton on 25.05.2023.
//

#ifndef D_GEN_SEMANTICS_H
#define D_GEN_SEMANTICS_H

#include "ast.h"
#include "Symbol.h"

class Semantics {
public:
	std::vector<Symbol*> symbols;

	explicit Semantics(FunctionNode *func);
	void eliminate_unreachable_code();
	void connect_loops();
	void type_ast();
private:
	FunctionNode *func;
	void eliminate_unreachable_code_visit_body(BodyNode *body);
	void connect_loops_visit_body(BodyNode *body, ForNode *loop);
	static bool type_visitor(ASTNode*, std::any &ctx);
};


#endif //D_GEN_SEMANTICS_H
