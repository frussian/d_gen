//
// Created by Anton on 13.04.2023.
//

#include <z3++.h>

#include <iostream>

z3::expr get_b(z3::context &ctx, z3::expr_vector &vec) {
    auto expr = ctx.bool_const("b");
    vec.push_back(expr);
    return expr;
}

void test_z3() {
	z3::context ctx;
    z3::expr_vector exprs(ctx);
	auto b = get_b(ctx, exprs);
	auto i = ctx.int_const("i");
	z3::expr one = ctx.int_val(1);
    exprs.push_back(one);
	z3::solver solver(ctx);
	z3::expr r = one == i;
	solver.add(b || r);
	std::cout << one.is_var() << b.is_var();
	std::cout << solver.check() << std::endl;
	auto model = solver.get_model();
	for (int j = 0; j < model.size(); j++) {
		auto val = model[j];

		std::cout << val.name() << " " << model.get_const_interp(val) << std::endl;
	}

    for (z3::expr expr: exprs) {
        std::cout << "expr: " << model.eval(expr, true) << std::endl;
    }
    std::cout << model.eval(b, true).is_true() << " " << model.eval(i, true).get_numeral_int() << std::endl;

}
