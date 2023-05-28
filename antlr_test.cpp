//
// Created by Anton on 12.04.2023.
//
#include <any>
#include <cstdlib>

#include "type.h"

#include "ASTBuilderVisitor.h"
#include "Semantics.h"
#include "CodegenVisitor.h"
#include "DGenJIT.h"

void antlr_test() {
	std::ifstream stream;
	stream.open("examples/simple.dg");
	if (stream.fail()) {
		throw "can't read file";
	}
	auto builder = std::make_unique<ASTBuilderVisitor>(stream);
	auto func = builder->parse();
	func->print(std::cout, 0);
	Semantics sem(func);
	sem.connect_loops();
	sem.type_ast();
	sem.type_check();
	sem.eliminate_unreachable_code();

	CodegenVisitor visitor;
	visitor.code_gen(func);

	auto mod = visitor.get_module();
	mod.getModuleUnlocked()->print(llvm::errs(), nullptr);

	auto J = cantFail(DGenJIT::Create());
	cantFail(J->addModule(std::move(mod)));

	auto d_gen_func = (void(*)())cantFail(J->lookup("d_gen_func")).getAddress();

	//loop
	std::cout << "calling d_gen_func" << std::endl;
	d_gen_func();
	std::cout << "called d_gen_func" << std::endl;
}
