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
	std::vector<std::shared_ptr<Symbol>> type_ast();
	void type_check();
private:
	FunctionNode *func;
	void eliminate_unreachable_code_visit_body(BodyNode *body);
	void connect_loops_visit_body(BodyNode *body, ForNode *loop);
	static bool type_visitor(ASTNode *node, std::any &ctx);
	static bool type_check_visitor(ASTNode *node, std::any &ctx);
	static void type_check_precondition(PrecondNode *pre_cond);
	static void type_check_asg(AsgNode *asg_node);
};


#endif //D_GEN_SEMANTICS_H
