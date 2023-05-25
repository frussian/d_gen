//
// Created by Anton on 12.04.2023.
//
#include <any>
#include <cstdlib>

#include "type.h"

#include "ASTBuilderVisitor.h"
#include "BuildError.h"

void antlr_test() {
	std::ifstream stream;
	stream.open("examples/prefix_func.dg");
	if (stream.fail()) {
		throw "can't read file";
	}
	auto builder = std::make_unique<ASTBuilderVisitor>(stream);
	auto func = builder->parse();
	func->print(std::cout, 0);
}
