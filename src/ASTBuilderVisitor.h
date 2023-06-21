//
// Created by Anton on 25.05.2023.
//

#ifndef D_GEN_ASTBUILDERVISITOR_H
#define D_GEN_ASTBUILDERVISITOR_H

#include "antlr4-runtime.h"
#include "d_genLexer.h"
#include "d_genParser.h"
#include "d_genBaseVisitor.h"

#include "ast.h"

class ASTBuilderVisitor: public d_genBaseVisitor {
public:
	explicit ASTBuilderVisitor(std::istream &input);
	~ASTBuilderVisitor() override = default;
	FunctionNode *parse();
private:
	std::istream &input;
	static Position getStartPos(antlr4::ParserRuleContext *ctx);

	//default for nonterminals with 1 nonterminal on the right side
	std::any visitChildren(antlr4::tree::ParseTree *node) override;

	std::any visitFunction(d_genParser::FunctionContext *ctx) override;

	std::any visitBody(d_genParser::BodyContext *ctx) override;

	std::any visitIf(d_genParser::IfContext *ctx) override;

	std::any visitWhile(d_genParser::WhileContext *ctx) override;

	std::any visitFor(d_genParser::ForContext *ctx) override;

	std::any visitDefine(d_genParser::DefineContext *ctx) override;

	std::any visitContinue(d_genParser::ContinueContext *ctx) override;

	std::any visitBreak(d_genParser::BreakContext *ctx) override;

	std::any visitReturn(d_genParser::ReturnContext *ctx) override;

	std::any visitSimple_asg(d_genParser::Simple_asgContext *ctx) override;

	std::any visitCrem_asg(d_genParser::Crem_asgContext *ctx) override;

	std::any visitPrecondition(d_genParser::PreconditionContext *ctx) override;

	std::any visitLogic_expr(d_genParser::Logic_exprContext *ctx) override;

	std::any visitLogic_t(d_genParser::Logic_tContext *ctx) override;

	std::any visitCmp_op(d_genParser::Cmp_opContext *ctx) override;

	std::any visitExpr(d_genParser::ExprContext *ctx) override;

	std::any visitT(d_genParser::TContext *ctx) override;

	std::any visitF_ident(d_genParser::F_identContext *ctx) override;

	std::any visitArray_lookup(d_genParser::Array_lookupContext *ctx) override;

	std::any visitArray_create(d_genParser::Array_createContext *ctx) override;

	std::any visitUnary_minus(d_genParser::Unary_minusContext *ctx) override;

	std::any visitProperty_lookup(d_genParser::Property_lookupContext *ctx) override;

	std::any visitType(d_genParser::TypeContext *ctx) override;

	std::any visitConst(d_genParser::ConstContext *ctx) override;

};


#endif //D_GEN_ASTBUILDERVISITOR_H
