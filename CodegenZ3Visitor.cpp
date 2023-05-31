//
// Created by Anton on 29.05.2023.
//

#include "CodegenZ3Visitor.h"
#include "utils/assert.h"

CodegenZ3Visitor::CodegenZ3Visitor(llvm::LLVMContext *ctx,
								   llvm::Module *mod,
								   llvm::IRBuilder<> *builder,
								   CodegenVisitor *cg_vis):
								   ctx(ctx),
								   mod(mod),
								   builder(builder),
								   cg_vis(cg_vis),
                                   exprs(z3_ctx) {

}

//ident - addr or symbol
//arr_lookup - addr or (symbol + indexes)
//consts
//bin op
//property_lookup

extern "C" void z3_gen(CodegenZ3Visitor *visitor, ASTNode *cond, PrecondNode *pre_cond) {
	visitor->start_z3_gen(cond, pre_cond);
}

llvm::Value *CodegenZ3Visitor::prepare_eval_ctx(ASTNode *cond, PrecondNode *pre_cond) {
	//update ptrs and indices
	traverse_ast_cb(cond, this);
	cond->visitChildren(&traverse_ast_cb, this);

	auto z3_gen_cb_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx),
											   {llvm::Type::getInt8PtrTy(*ctx),
												llvm::Type::getInt8PtrTy(*ctx),
												llvm::Type::getInt8PtrTy(*ctx)},
											   false);

	auto z3_gen_cb = mod->getOrInsertFunction("z3_gen", z3_gen_cb_t);
	return builder->CreateCall(z3_gen_cb, {Symbol::get_ptr(this, get_ctx()),
										   Symbol::get_ptr(cond, get_ctx()),
										   Symbol::get_ptr(pre_cond, get_ctx())});
}

extern "C" void upd_arr_idxs(ArrLookupNode *node, int *idxs, int len) {
	std::vector<int> idxs_vec;
	idxs_vec.reserve(len);
	for (int i = 0; i < len; i++) {
		idxs_vec.push_back(idxs[i]);
	}
	node->current_idxs = std::move(idxs_vec);
}

extern "C" void upd_arr_lookup_addr(ArrLookupNode *node, void *addr) {
	node->current_ptr = addr;
}

extern "C" void upd_ident_addr(IdentNode *node, void *addr) {
	node->symbol->addr = addr;
}

llvm::Value *CodegenZ3Visitor::prepare_eval_ctx(IdentNode *node) {
	//TODO: refactor this with CodegenVisitor parts

	auto sym = node->symbol;
	if (sym->is_input) {
		return nullptr;
	}
	auto upd_ident_addr_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx),
													{llvm::Type::getInt8PtrTy(*ctx),
												  llvm::Type::getInt8PtrTy(*ctx)}, false);

	auto upd_ident_addr_cb = mod->getOrInsertFunction("upd_ident_addr", upd_ident_addr_t);
	return builder->CreateCall(upd_ident_addr_cb, {Symbol::get_ptr(node, get_ctx()), sym->alloca});
}

llvm::Value *CodegenZ3Visitor::prepare_eval_ctx(ArrLookupNode *node) {
	//TODO: refactor this with CodegenVisitor parts
	auto sym = node->ident->symbol;
	if (!sym->is_input) {
		llvm::Value *addr = sym->alloca;

		for (auto &idx: node->idxs) {
			auto ptr = builder->CreateLoad(addr->getType()->getPointerElementType(), addr);
			addr = builder->CreateGEP(ptr->getType()->getPointerElementType(),
									  ptr, idx->code_gen(cg_vis));
		}
		auto upd_lookup_addr_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx),
														 {llvm::Type::getInt8PtrTy(*ctx),
														 llvm::Type::getInt8PtrTy(*ctx)}, false);

		auto upd_lookup_addr = mod->getOrInsertFunction("upd_arr_lookup_addr", upd_lookup_addr_t);
		return builder->CreateCall(upd_lookup_addr,
								   {Symbol::get_ptr(node, get_ctx()), addr});
	}

	std::vector<llvm::Value*> idxs;
	idxs.reserve(node->idxs.size());
	for (const auto &idx: node->idxs) {
		idxs.push_back(idx->code_gen(cg_vis));
	}

	auto t = llvm::IntegerType::getInt32Ty(*ctx);
	auto var_arr = builder->CreateAlloca(t, builder->getInt32(idxs.size()), "arr_idxs");
	for (int i = 0; i < idxs.size(); i++) {
		auto el_ptr = builder->CreateGEP(t, var_arr, builder->getInt64(i));
		builder->CreateStore(idxs[i], el_ptr);
	}

	//void (ArrLookup *, int *idxs, int len)
	auto idx_cb_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx),
											{llvm::Type::getInt8PtrTy(*ctx),
											 llvm::Type::getInt32PtrTy(*ctx),
											 llvm::Type::getInt32Ty(*ctx)}, false);

	auto idx_cb = mod->getOrInsertFunction("upd_arr_idxs", idx_cb_t);

	return builder->CreateCall(idx_cb, {Symbol::get_ptr(node, get_ctx()),
										var_arr,
										builder->getInt32(idxs.size())});
}

LLVMCtx CodegenZ3Visitor::get_ctx() {
	return {ctx, mod, builder};
}

bool CodegenZ3Visitor::traverse_ast_cb(ASTNode *node, std::any ctx) {
	auto visitor = std::any_cast<CodegenZ3Visitor*>(ctx);
	if (auto ident = dynamic_cast<IdentNode*>(node)) {
		visitor->prepare_eval_ctx(ident);
	} else if (auto arr_lookup = dynamic_cast<ArrLookupNode*>(node)) {
		visitor->prepare_eval_ctx(arr_lookup);
	} //TODO: implement property_lookup

	return true;
}

void CodegenZ3Visitor::start_z3_gen(ASTNode *cond, PrecondNode *pre_cond) {
	syms_to_expr_id.clear();
    exprs = z3::expr_vector(z3_ctx);
	auto cond_expr = cond->gen_expr(this);
    //TODO: if exprs.empty() then return (no unknown variables)
	z3::solver solver(z3_ctx);
	if (pre_cond->prob != 1) {
		auto r = std::abs(std::rand() % 100);
		if (r > pre_cond->prob) {
			std::cout << "decided to negate, recv " << r << " prob" << std::endl;
			cond_expr = !cond_expr;
		}
	} else {
		//todo: other method based on coverage
	}
	solver.add(cond_expr);
    for (const auto &item: syms_to_expr_id) {
        auto sym = item.first;
        if (dynamic_cast<CharSym*>(sym)) {
            //TODO: this should be a part of precond->cond
            auto expr_idx = item.second;
            auto expr = exprs[expr_idx];
            solver.add(97 <= expr && expr <= 122);
        }
    }

	std::cout << "err: " << solver.check_error() << std::endl;
	std::cout << "solver " << solver << std::endl;

	auto res = solver.check();
	if (res != z3::sat) {
		std::cout << "couldn't check satisfiability " << res << std::endl;
		return;
	}

	std::cout << "satisfiability checked successfully" << std::endl;

	auto model = solver.get_model();
	std::cout << "model " << model.to_string() << std::endl;
	for (const auto &item: syms_to_expr_id) {
		auto sym = item.first;
		auto expr_idx = item.second;
        auto expr = exprs[expr_idx];
        auto eval = model.eval(expr, true);
		sym->fill_val(eval);
	}
}

z3::expr CodegenZ3Visitor::gen_expr(BoolNode *node) {
	return z3_ctx.bool_val(node->val);
}

z3::expr CodegenZ3Visitor::gen_expr(CharNode *node) {
	return z3_ctx.int_val(node->ch);
}

z3::expr CodegenZ3Visitor::gen_expr(NumberNode *node) {
	return z3_ctx.int_val(node->num);
}

z3::expr CodegenZ3Visitor::gen_expr(IdentNode *node) {
	auto sym = node->symbol;
	if (sym->is_input) {
		auto expr = sym->get_expr(z3_ctx);
		if (!sym->has_val()) {
            syms_to_expr_id[sym.get()] = exprs.size();
            exprs.push_back(expr);
		}
		return expr;
	}

	return get_expr_from_void(sym->addr, sym->type);
}

z3::expr CodegenZ3Visitor::gen_expr(ArrLookupNode *node) {
	auto sym = node->ident->symbol;
	if (sym->is_input) {
		auto arr_sym = std::dynamic_pointer_cast<ArraySym>(sym).get();
		auto indexed_sym = ArraySym::get_symbol_by_idxs(arr_sym, node->current_idxs);
		std::string idx_str = "__arr_" + arr_sym->name;
		for (auto &e: node->current_idxs) {
			idx_str += std::to_string(e) + "_";
		}
		indexed_sym->name = idx_str;
		auto expr = indexed_sym->get_expr(z3_ctx);
		if (!indexed_sym->has_val()) {
            syms_to_expr_id[indexed_sym.get()] = exprs.size();
            exprs.push_back(expr);
		}
		return expr;
	}

	return get_expr_from_void(node->current_ptr, node->get_type());
}

z3::expr CodegenZ3Visitor::get_expr_from_void(void *ptr, Type type) {
	int32_t int32;
	int8_t int8;
	switch (type.getCurrentType()) {
		case TypeKind::INT:
			int32 = *(int32_t*)ptr;
			return z3_ctx.int_val(int32);
		case TypeKind::CHAR:
			int8 = *(int8_t*)ptr;
			return z3_ctx.int_val(int8);
		case TypeKind::BOOL:
			int8 = *(int8_t*)ptr;
			return z3_ctx.bool_val(int8 != 0);
		default:
			throw std::runtime_error("unexpected type on gen_expr: " + type.to_string());
	}
}

z3::expr CodegenZ3Visitor::gen_expr(BinOpNode *node) {
	auto lhs = node->lhs->gen_expr(this);
	auto rhs = node->rhs->gen_expr(this);
	switch (node->op_type) {
		case BinOpType::SUM:
			return lhs + rhs;
		case BinOpType::SUB:
			return lhs - rhs;
		case BinOpType::OR:
			return lhs || rhs;
		case BinOpType::LT:
			return lhs < rhs;
		case BinOpType::LE:
			return lhs <= rhs;
		case BinOpType::GT:
			return lhs > rhs;
		case BinOpType::GE:
			return lhs >= rhs;
		case BinOpType::EQ:
			return lhs == rhs;
		case BinOpType::NEQ:
			return lhs != rhs;
		case BinOpType::MUL:
			return lhs * rhs;
		case BinOpType::DIV:
			return lhs / rhs;
		case BinOpType::AND:
			return lhs && rhs;
	}
}

