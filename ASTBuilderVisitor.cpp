//
// Created by Anton on 25.05.2023.
//

#include "ASTBuilderVisitor.h"
#include "utils/assert.h"
#include "BuildError.h"

#define gen_bin_op_construction(func) \
	auto lhs = std::any_cast<ASTNode*>(func(ctx->operands[0])); \
	for (int i = 1; i < ctx->operands.size(); i++) { \
		auto pos = getStartPos(ctx->operands[i]); \
		auto rhs = std::any_cast<ASTNode*>(func(ctx->operands[i])); \
		lhs = BinOpNode::create(pos, ctx->ops[i-1]->getText(), lhs, rhs); \
	} \
	return lhs;

class ErrListener: public antlr4::BaseErrorListener {
public:
	std::vector<Err> errors;

	void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
					 size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {
		errors.push_back(Err{Position(line, charPositionInLine), msg});
	};
};

ASTBuilderVisitor::ASTBuilderVisitor(std::istream &input): input(input) {}

FunctionNode *ASTBuilderVisitor::parse() {
	antlr4::ANTLRInputStream in_stream(input);

	auto err_listener = std::make_unique<ErrListener>();

	d_genLexer lexer(&in_stream);
	lexer.removeErrorListeners();
	lexer.addErrorListener(err_listener.get());
	antlr4::CommonTokenStream tokens(&lexer);

	d_genParser parser(&tokens);
	parser.removeErrorListeners();
	parser.addErrorListener(err_listener.get());

	d_genParser::ProgramContext* program = parser.program();

//	std::cout << tokens.size() << std::endl;
//	for (auto tok: tokens.getTokens()) {
//		std::cout << "tok: " << tok->toString() << std::endl;
//	}

	if (!err_listener->errors.empty()) {
		throw BuildError(std::move(err_listener->errors));
	}

	auto res = std::any_cast<FunctionNode *>(visitProgram(program));
	return res;
}

Position ASTBuilderVisitor::getStartPos(antlr4::ParserRuleContext *ctx) {
	auto start_token = ctx->getStart();
	return {static_cast<int>(start_token->getLine()),
			static_cast<int>(start_token->getCharPositionInLine())};
}

std::any ASTBuilderVisitor::visitChildren(antlr4::tree::ParseTree *node) {
	for (auto c: node->children) {
		if (c->getTreeType() == antlr4::tree::ParseTreeType::RULE) {
			return c->accept(this);
		}
	}
	ASSERT(false, "visiting child with zero nonterminals");
}

std::any ASTBuilderVisitor::visitFunction(d_genParser::FunctionContext *ctx) {
	ASTNode *pre_cond = nullptr;
	if (ctx->precondition()) {
		pre_cond = std::any_cast<PrecondNode*>(ctx->precondition()->accept(this));
	}
	auto ret_type = std::any_cast<Type>(ctx->ret_type->accept(this));
	auto f_name = ctx->f_name->getText();
	std::vector<DefNode*> args;
	args.reserve(ctx->args.size());
	for (int i = 0; i < ctx->args.size(); i++) {
		auto ident_name = ctx->args[i]->getText();
		auto ident_type = std::any_cast<Type>(ctx->arg_types[i]->accept(this));
		args.push_back(new DefNode(getStartPos(ctx->arg_types[i]), ident_name, ident_type, nullptr));
	}
	auto body = std::any_cast<BodyNode*>(ctx->body()->accept(this));
	return new FunctionNode(getStartPos(ctx), pre_cond, ret_type, std::move(f_name), std::move(args), body);
}

std::any ASTBuilderVisitor::visitBody(d_genParser::BodyContext *ctx) {
	auto pos = getStartPos(ctx);
	std::vector<ASTNode*> stmts;
	stmts.reserve(ctx->stmts.size());
	for (auto stmt: ctx->stmts) {
		stmts.push_back(std::any_cast<ASTNode*>(stmt->accept(this)));
	}
	return new BodyNode(pos, std::move(stmts));
}

std::any ASTBuilderVisitor::visitIf(d_genParser::IfContext *ctx) {
	PrecondNode *precond = nullptr;
	if (auto precond_ctx = ctx->precondition()) {
		precond = std::any_cast<PrecondNode*>(visitPrecondition(precond_ctx));
	}

	auto cond = std::any_cast<ASTNode*>(visitLogic_expr(ctx->logic_expr()));
	auto body = std::any_cast<BodyNode*>(visitBody(ctx->body(0)));
	BodyNode *else_body = nullptr;
	if (ctx->body().size() == 2) {
		else_body = std::any_cast<BodyNode*>(visitBody(ctx->body(1)));
	}

	return static_cast<ASTNode*>(new IfNode(getStartPos(ctx), precond, cond, body, else_body));
}

std::any ASTBuilderVisitor::visitWhile(d_genParser::WhileContext *ctx) {
	PrecondNode *precond = nullptr;
	if (auto precond_ctx = ctx->precondition()) {
		precond = std::any_cast<PrecondNode*>(visitPrecondition(precond_ctx));
	}
	auto cond = std::any_cast<ASTNode*>(visitLogic_expr(ctx->logic_expr()));
	auto body = std::any_cast<BodyNode*>(visitBody(ctx->body()));
	return static_cast<ASTNode*>(new ForNode(getStartPos(ctx), precond,
											 nullptr, cond, nullptr, body));
}

std::any ASTBuilderVisitor::visitFor(d_genParser::ForContext *ctx) {
	PrecondNode *precond = nullptr;
	if (auto precond_ctx = ctx->precondition()) {
		precond = std::any_cast<PrecondNode*>(visitPrecondition(precond_ctx));
	}
	ASTNode *pre_asg = nullptr;
	if (ctx->pre_asg) {
		pre_asg = std::any_cast<ASTNode*>(ctx->pre_asg->accept(this));
	}
	auto cond = std::any_cast<ASTNode*>(visitLogic_expr(ctx->logic_expr()));
	ASTNode *inc_asg = nullptr;
	if (ctx->inc_asg) {
		inc_asg = std::any_cast<ASTNode*>(ctx->inc_asg->accept(this));
	}
	auto body = std::any_cast<BodyNode*>(visitBody(ctx->body()));
	return static_cast<ASTNode*>(new ForNode(getStartPos(ctx), precond, pre_asg,
											 cond, inc_asg, body));
}

std::any ASTBuilderVisitor::visitDefine(d_genParser::DefineContext *ctx) {
	auto type = std::any_cast<Type>(visitType(ctx->type()));
	auto ident = ctx->IDENT()->getText();
	ASTNode *rhs = nullptr;
	if (auto le = ctx->logic_expr()) {
		rhs = std::any_cast<ASTNode*>(visitLogic_expr(le));
	}
	return static_cast<ASTNode*> (
			new DefNode(getStartPos(ctx), std::move(ident), type, rhs)
	);
}

std::any ASTBuilderVisitor::visitContinue(d_genParser::ContinueContext *ctx) {
	return static_cast<ASTNode*>(new ContinueNode(getStartPos(ctx)));
}

std::any ASTBuilderVisitor::visitBreak(d_genParser::BreakContext *ctx) {
	return static_cast<ASTNode*>(new BreakNode(getStartPos(ctx)));
}

std::any ASTBuilderVisitor::visitReturn(d_genParser::ReturnContext *ctx) {
	auto expr = std::any_cast<ASTNode*>(ctx->logic_expr()->accept(this));
	return static_cast<ASTNode*>(new ReturnNode(getStartPos(ctx), expr));
}

std::any ASTBuilderVisitor::visitSimple_asg(d_genParser::Simple_asgContext *ctx) {
	antlr4::ParserRuleContext *lhs_rule;
	if (auto ident_name = ctx->f_ident()) {
		lhs_rule = ident_name;
	} else {
		lhs_rule = ctx->array_lookup();
	}
	auto lhs = std::any_cast<ASTNode*>(lhs_rule->accept(this));
	auto rhs = std::any_cast<ASTNode*>(ctx->logic_expr()->accept(this));
	std::string assg;
	if (ctx->ASSG_TYPE()) {
		assg = ctx->ASSG_TYPE()->getText();
	} else {
		assg = ctx->ASSG()->getText();
	}
	return static_cast<ASTNode*>(AsgNode::create(getStartPos(ctx), lhs, assg, rhs));
}

std::any ASTBuilderVisitor::visitCrem_asg(d_genParser::Crem_asgContext *ctx) {
	antlr4::ParserRuleContext *lhs_rule;
	if (auto ident_name = ctx->f_ident()) {
		lhs_rule = ident_name;
	} else {
		lhs_rule = ctx->array_lookup();
	}
	auto lhs = std::any_cast<ASTNode*>(lhs_rule->accept(this));
	return static_cast<ASTNode*>(CremNode::create(getStartPos(ctx), lhs, ctx->CREM_TYPE()->getText()));
}

std::any ASTBuilderVisitor::visitPrecondition(d_genParser::PreconditionContext *ctx) {
	int prob = -1;
	if (auto num = ctx->NUM()) {
		prob = std::atoi(num->getText().c_str());
	}
	ASTNode *expr = nullptr;
	if (ctx->logic_expr()) {
		expr = std::any_cast<ASTNode*>( visitLogic_expr(ctx->logic_expr()) );
	}
	return new PrecondNode(getStartPos(ctx), prob, expr);
//	return visitLogic_expr(ctx->logic_expr());
}

std::any ASTBuilderVisitor::visitLogic_expr(d_genParser::Logic_exprContext *ctx) {
//		std::cout << "visit logic expr" << std::endl;
	gen_bin_op_construction(visitLogic_t)
}

std::any ASTBuilderVisitor::visitLogic_t(d_genParser::Logic_tContext *ctx) {
//		std::cout << "visit logic t" << std::endl;
	gen_bin_op_construction(visitLogic_f)
}

std::any ASTBuilderVisitor::visitCmp_op(d_genParser::Cmp_opContext *ctx) {
//		std::cout << "visit cmp op" << std::endl;

	auto lhs = std::any_cast<ASTNode*>(visitExpr(ctx->expr(0)));
	auto rhs = std::any_cast<ASTNode*>(visitExpr(ctx->expr(1)));
	//need to static_cast in order to any_cast later
	return static_cast<ASTNode*>(
			BinOpNode::create(getStartPos(ctx),
							  ctx->CMP_OP()->getText(),
							  lhs, rhs)
	);
}

std::any ASTBuilderVisitor::visitExpr(d_genParser::ExprContext *ctx) {
//		std::cout << "visit expr" << std::endl;

	gen_bin_op_construction(visitT)
}

std::any ASTBuilderVisitor::visitT(d_genParser::TContext *ctx) {
//		std::cout << "visit t" << std::endl;
//		for (auto c: ctx->children) {
//			auto str = c->getText();
//			std::replace(str.begin(), str.end(), '\r', ' ');
//			std::cout << "t: " << str << ", ";
//		}
//		std::cout << std::endl;

	gen_bin_op_construction(visitF)
}

std::any ASTBuilderVisitor::visitF_ident(d_genParser::F_identContext *ctx) {
	return static_cast<ASTNode*> (
			new IdentNode(getStartPos(ctx),ctx->IDENT()->getText())
	);
}

std::any ASTBuilderVisitor::visitArray_lookup(d_genParser::Array_lookupContext *ctx) {
//		std::cout << "visit arr lookup" << std::endl;
	std::vector<ASTNode*> idxs;
	idxs.reserve(ctx->expr().size());
	for (const auto &expr: ctx->expr()) {
		idxs.push_back(std::any_cast<ASTNode*>(visitExpr(expr)));
	}
	return static_cast<ASTNode*> (
			new ArrLookupNode(getStartPos(ctx),
							  ctx->IDENT()->getText(),
							  idxs
			)
	);
}

std::any ASTBuilderVisitor::visitArray_create(d_genParser::Array_createContext *ctx) {
//		std::cout << "visit array create" << std::endl;
	auto type = std::any_cast<Type>(visitType(ctx->type()));
	auto len = std::any_cast<ASTNode*>(visitExpr(ctx->expr()));
	return static_cast<ASTNode*> (
			ArrCreateNode::create(getStartPos(ctx), type, len)
	);
}

std::any ASTBuilderVisitor::visitProperty_lookup(d_genParser::Property_lookupContext *ctx) {
	return static_cast<ASTNode*> (new PropertyLookupNode(getStartPos(ctx),
														 ctx->IDENT(0)->getText(),
														 ctx->IDENT(1)->getText()));
}

std::any ASTBuilderVisitor::visitType(d_genParser::TypeContext *ctx) {
	return Type::create(ctx->types);
}

std::any ASTBuilderVisitor::visitConst(d_genParser::ConstContext *ctx) {
	auto pos = getStartPos(ctx);
	ASTNode *constNode = nullptr;
	if (auto char_token = ctx->CHAR()) {
		constNode = CharNode::create(pos, char_token);
	} else if (auto token = ctx->STRING()) {
		auto text = token->getText();
		constNode = new StringNode(pos, text.substr(1, text.size()-2));
	} else if (auto num_token = ctx->NUM()) {
		constNode = NumberNode::create(pos, num_token);
	} else if (auto bool_token = ctx->BOOL()) {
		constNode = BoolNode::create(pos, bool_token);
	}
	//TODO: replace
	return constNode;
}

std::any ASTBuilderVisitor::visitUnary_minus(d_genParser::Unary_minusContext *ctx) {
	auto f = std::any_cast<ASTNode*>(ctx->f()->accept(this));
	return static_cast<ASTNode*>(
			new BinOpNode(getStartPos(ctx), BinOpType::MUL, new NumberNode(getStartPos(ctx), -1), f)
		);
}
