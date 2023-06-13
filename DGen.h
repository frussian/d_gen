//
// Created by Anton on 29.05.2023.
//

#ifndef D_GEN_DGEN_H
#define D_GEN_DGEN_H

#include <memory>
#include <vector>
#include <optional>
#include <istream>

class CodegenVisitor;
class FunctionNode;
class Symbol;

extern "C" void gather_res(CodegenVisitor *visitor, void *res);

class DGen {
public:
	explicit DGen(std::istream &input);

	//TODO: add args: seed, number of tests, coverage
	std::string generate_json(int tests_num, std::optional<int> seed = std::optional<int>());

	//only gets called once
	static void init_backend();
private:
	std::istream &input;
	std::vector<std::string> tests;
	void gather_res(void *res);
	FunctionNode *func;
	std::vector<std::shared_ptr<Symbol>> inputs;

	void reset();
	friend void ::gather_res(CodegenVisitor *visitor, void *res);
};

#endif //D_GEN_DGEN_H
