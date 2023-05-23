//
// Created by Anton on 12.04.2023.
//

#include "antlr4-runtime.h"
#include "d_genLexer.h"
#include "d_genParser.h"
#include "d_genBaseVisitor.h"

class ErrListener: public antlr4::BaseErrorListener {
public:
	explicit ErrListener() = default;
	~ErrListener() = default;
	void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
					 size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {
//		if (e != nullptr) rethrow_exception(e);
		std::cout << line << " " << charPositionInLine << std::endl;
		std::cout << msg << std::endl;
	};
};

class ProgVisitor: public d_genBaseVisitor {
public:
	std::any visitStatement(d_genParser::StatementContext *ctx) override {
		if (auto asg = ctx->assignment()) {
			std::cout << "asg " << asg->IDENT()->getText() << " " << asg->ASSG_TYPE() << std::endl;
		} else if (auto ret = ctx->return_()) {
			std::cout << ret->expr() << std::endl;
		}
		return visitChildren(ctx);
	}
};

void antlr_test() {
	std::ifstream stream;
	stream.open("examples/prefix_func.dg");
	if (stream.fail()) {
		throw "can't read file";
	}
	antlr4::ANTLRInputStream input(stream);

	auto err_listener = new ErrListener();

	d_genLexer lexer(&input);
	lexer.removeErrorListeners();
	lexer.addErrorListener(err_listener);
	antlr4::CommonTokenStream tokens(&lexer);

	d_genParser parser(&tokens);
	parser.removeErrorListeners();
	parser.addErrorListener(err_listener);

	d_genParser::ProgramContext* program = parser.program();
	auto visitor = new ProgVisitor;
	std::any res = visitor->visitProgram(program);
	int *ires = any_cast<int>(&res);

//	std::cout << parser.getErrorHandler() << std::endl;
//	std::cout << program->toString() << std::endl;
	for (auto id: program->function()->IDENT()) {
		std::cout << id->getText() << std::endl;
	}
//	ImageVisitor visitor;
}
