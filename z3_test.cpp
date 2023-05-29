//
// Created by Anton on 13.04.2023.
//

#include <z3++.h>

#include <iostream>

void test_z3() {
	z3::context ctx;
	auto b = ctx.bool_const("b");
	auto i = ctx.int_const("i");
	z3::expr one = ctx.int_val(1);
	z3::solver solver(ctx);
	z3::expr r = one == i;
	solver.add(b || r);
	std::cout << one.is_var() << b.is_var();
	std::cout << solver.check() << std::endl;
	auto model = solver.get_model();
	for (int j = 0; j < model.size(); j++) {
		auto val = model[j];
		if (j == 1) {
			std::cout << "bool: " << model.get_const_interp(val).is_true() << std::endl;
		}
		std::cout << val.name() << " " << model.get_const_interp(val) << std::endl;
	}
}
