//
// Created by Anton on 29.05.2023.
//

#include "DGen.h"

#include <any>
#include <cstdlib>

#include "type.h"

#include "ASTBuilderVisitor.h"
#include "Semantics.h"
#include "CodegenVisitor.h"
#include "DGenJIT.h"

DGen::DGen(std::istream &input): input(input) {}

std::string DGen::generate_json(std::optional<int> seed) {
	auto builder = std::make_unique<ASTBuilderVisitor>(input);
	func = builder->parse();
//	func->print(std::cout, 0);
	Semantics sem(func);
	sem.connect_loops();
	inputs = sem.type_ast();
	sem.type_check();
	sem.eliminate_unreachable_code();

	if (!seed.has_value()) {
		seed = time(NULL);
	}

	std::srand(*seed);

	CodegenVisitor visitor(this);
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

	return "{}";
}

std::string gather_rec(void *res, Type type) {
	int32_t num;
	uint8_t *str_ptr, *arr_ptr;
	uint32_t size;
	uint8_t ch;
	std::string tmp;
	int type_size;

	switch (type.getCurrentType()) {
		case TypeKind::INT:
			num = *(int32_t*)res;
			return std::to_string(num);
		case TypeKind::STRING:
			str_ptr = *(uint8_t**)res;
			size = Symbol::allocated_vals[str_ptr];
			//TODO: escape json
			return "\"" + std::string((const char*)str_ptr, size) + "\"";
		case TypeKind::CHAR:
			ch = *(uint8_t*)res;
			return std::to_string((char)ch);
		case TypeKind::BOOL:
			ch = *(uint8_t*)res;
			if (ch) {
				return "true";
			} else {
				return "false";
			}
		case TypeKind::ARR:
			arr_ptr = *(uint8_t**)res;
			size = Symbol::allocated_vals[arr_ptr];
			type_size = Symbol::create_symbol(Position(0, 0), type.dropType(), "tmp")->get_sizeof();
			tmp = "[";
			for (int i = 0; i < size; i++) {
				tmp += gather_rec(arr_ptr + i*type_size, type.dropType());
				if (i + 1 != size) {
					tmp += + ",";
				}
			}
			tmp += ']';
			return tmp;
	}

	return "";
}

void DGen::gather_res(void *res) {
	std::cout << "test data:" << std::endl;

	std::string test_input = "{\n";
	for (const auto &in_sym: inputs) {
		test_input += "\t" + in_sym->name + ": ";
		test_input += in_sym->serialize();
		test_input += ",\n";
	}
	test_input[test_input.size()-2] = ' ';
	test_input += "}";
	std::cout << test_input << std::endl;

	auto res_str = gather_rec(res, func->ret_type);
	std::cout << "expected result:" << std::endl;
	std::cout << res_str << std::endl;
	//TODO: clear allocated_vals, clear in_syms
}
