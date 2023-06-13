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

void test_z3_2() {
	z3::context ctx;
	z3::solver solver(ctx);

	// Set option
	solver.set("arith.random_initial_value", true);

	// Declare variables
	z3::expr x = ctx.int_const("x");
	z3::expr y = ctx.int_const("y");

	// Add constraints to solver
	solver.add(x + y > 0);

	// Solve the first constraint with random seed 1
	solver.push();
	solver.set("random_seed", (unsigned int) 1);
	std::cout << "Model 1: " << solver.check() << "\n";
	std::cout << solver.get_model() << "\n";
	solver.pop();

	// Solve the second constraint with random seed 2
	solver.push();
	solver.set("random_seed", (unsigned int) 2);
	std::cout << "Model 2: " << solver.check() << "\n";
	std::cout << solver.get_model() << "\n";
	solver.pop();

	// Solve the third constraint with random seed 3
	solver.push();
	solver.set("random_seed", (unsigned int) 3);
	std::cout << "Model 3: " << solver.check() << "\n";
	std::cout << solver.get_model() << "\n";
	solver.pop();
}

void test_z3(int seed) {
//	test_z3_2();
	z3::context ctx;

	z3::tactic t(ctx, "smt");

    z3::expr_vector exprs(ctx);
	auto b = get_b(ctx, exprs);
	auto i = ctx.int_const("i");
	z3::expr one = ctx.int_val(1);
    exprs.push_back(one);

	auto solver = t.mk_solver();
	solver.set("arith.random_initial_value", true);
	solver.set("random_seed", (unsigned int)std::rand());
	solver.set("completion", true);

	solver.add(i > 0);
	std::cout << solver.check() << std::endl;

	std::cout << solver << std::endl;

	auto model = solver.get_model();

	std::cout << model << std::endl;

	for (int j = 0; j < model.size(); j++) {
		auto val = model[j];

		std::cout << val.name() << " " << model.get_const_interp(val) << std::endl;
	}

//    for (z3::expr expr: exprs) {
//        std::cout << "expr: " << model.eval(expr, true) << std::endl;
//    }
//    std::cout << model.eval(b, true).is_true() << " " << model.eval(i, true) << std::endl;

}
