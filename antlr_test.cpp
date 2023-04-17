//
// Created by Anton on 12.04.2023.
//

#include "antlr4-runtime.h"
#include "d_genLexer.h"
#include "d_genParser.h"

void antlr_test() {
	std::ifstream stream;
	stream.open("input.scene");

	antlr4::ANTLRInputStream input(stream);

	d_genLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	d_genParser parser(&tokens);

	d_genParser::FileContext* tree = parser.file();
	for (auto e: tree->elements) {
		std::cout << e->DRAW() << '\n';
	}
	std::cout << tree->elements.size() << std::endl;
//	ImageVisitor visitor;
}