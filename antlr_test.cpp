//
// Created by Anton on 12.04.2023.
//
#include <any>
#include <cstdlib>

#include "type.h"

#include "ASTBuilderVisitor.h"
#include "Semantics.h"

void antlr_test() {
	std::ifstream stream;
	stream.open("examples/asg.dg");
	if (stream.fail()) {
		throw "can't read file";
	}
	auto builder = std::make_unique<ASTBuilderVisitor>(stream);
	auto func = builder->parse();
	func->print(std::cout, 0);
	//TODO: check break and continue => type-checking => eliminate unreachable code
	Semantics sem(func);
	sem.connect_loops();
	sem.type_ast();
	sem.eliminate_unreachable_code();
//	func->print(std::cout, 0);
}
