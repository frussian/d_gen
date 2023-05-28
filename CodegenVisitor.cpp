//
// Created by Anton on 27.05.2023.
//

#include "utils/assert.h"

#include "CodegenVisitor.h"

CodegenVisitor::CodegenVisitor() {
	ctx = std::make_unique<llvm::LLVMContext>();
	mod = std::make_unique<llvm::Module>("test", *ctx);

	llvm::Function *d_gen_func =
			llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx),
														   {}, false),
								   llvm::Function::ExternalLinkage, "d_gen_func", mod.get());

	llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ctx, "EntryBlock", d_gen_func);
	builder = std::make_unique<llvm::IRBuilder<>>(BB);
}


llvm::Value *CodegenVisitor::code_gen(FunctionNode *func) {
	code_gen(func->body);
	return nullptr;
}

llvm::Value *CodegenVisitor::code_gen(BodyNode *body) {
	std::cout << "visiting body node" << std::endl;
	for (auto stmt: body->stmts) {
		stmt->code_gen(this);
	}

	return nullptr;
}

llvm::Value *CodegenVisitor::code_gen(BoolNode *node) {
	return builder->getInt8(node->val);
}

llvm::Value *CodegenVisitor::code_gen(NumberNode *node) {
	return builder->getInt32(node->num);
}

llvm::Value *CodegenVisitor::code_gen(StringNode *node) {
	return builder->CreateGlobalStringPtr(node->str);
}

llvm::Value *CodegenVisitor::code_gen(CharNode *node) {
	return builder->getInt8(node->ch);
}

llvm::Value *CodegenVisitor::code_gen(BinOpNode *node) {
	auto lhs = node->lhs->code_gen(this);
	auto rhs = node->rhs->code_gen(this);
	switch (node->op_type) {
		case BinOpType::SUM:
			return builder->CreateAdd(lhs, rhs);
		case BinOpType::SUB:
			return builder->CreateSub(lhs, rhs);
		case BinOpType::MUL:
			return builder->CreateMul(lhs, rhs);
		case BinOpType::DIV:
			return builder->CreateSDiv(lhs, rhs);
		case BinOpType::OR:
			return builder->CreateOr(lhs, rhs);
		case BinOpType::AND:
			return builder->CreateAnd(lhs, rhs);
		case BinOpType::LT:
			return builder->CreateICmpSLT(lhs, rhs);
		case BinOpType::LE:
			return builder->CreateICmpSLE(lhs, rhs);
		case BinOpType::GT:
			return builder->CreateICmpSGT(lhs, rhs);
		case BinOpType::GE:
			return builder->CreateICmpSGE(lhs, rhs);
		case BinOpType::EQ:
			return builder->CreateICmpEQ(lhs, rhs);
		case BinOpType::NEQ:
			return builder->CreateICmpNE(lhs, rhs);
	}
}

llvm::Value *CodegenVisitor::code_gen(IdentNode *node) {
	auto symbol = node->symbol;

	if (!symbol->is_input) {
		return builder->CreateLoad(symbol->alloca->getAllocatedType(), symbol->alloca);
	}

	return symbol->code_gen(get_ctx());
}

llvm::Value *CodegenVisitor::code_gen(DefNode *node) {
	std::cout << "visiting def node" << std::endl;
	node->sym->create_alloca(get_ctx());
	if (!node->rhs) {
		return nullptr;
	}

	auto rhs = node->rhs->code_gen(this);
	builder->CreateStore(rhs, node->sym->alloca);

	return nullptr;
}

LLVMCtx CodegenVisitor::get_ctx() {
	return {ctx.get(), mod.get(), builder.get()};
}

//todo: pass pointer as res
extern "C" void gather_res(CodegenVisitor *visitor, int32_t res) {
//	todo: check func ret type here visitor->func->type
	std::cout << "func returned " << res << std::endl;
}

llvm::Value *CodegenVisitor::code_gen(ReturnNode *node) {
	//TODO: call function to process the result
	auto res = node->expr->code_gen(this);

	auto gather_res_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx),
												{llvm::Type::getInt32PtrTy(*ctx),
												 llvm::Type::getInt32Ty(*ctx)}, false);

	auto gather_res_cb = mod->getOrInsertFunction("gather_res", gather_res_t);

	builder->CreateCall(gather_res_cb, {Symbol::get_ptr(this, get_ctx()), res});
	return builder->CreateRetVoid();
}

llvm::orc::ThreadSafeModule CodegenVisitor::get_module() {
	return {std::move(mod), std::move(ctx)};
}

llvm::Value *CodegenVisitor::code_gen(AsgNode *node) {
	std::cout << "visiting asg node" << std::endl;
	auto addr = get_address(node->lhs);
	auto rhs = node->rhs->code_gen(this);

	return builder->CreateStore(rhs, addr);
}

llvm::Value *CodegenVisitor::get_address(ASTNode *node) {
	if (auto ident = dynamic_cast<IdentNode*>(node)) {
		return ident->symbol->alloca;
	} else if (auto lookup = dynamic_cast<ArrLookupNode*>(node)) {
		std::vector<llvm::Value*> idxs;
		idxs.reserve(lookup->idxs.size());
		for (const auto &idx: lookup->idxs) {
			idxs.push_back(idx->code_gen(this));
		}

		auto sym = lookup->ident->symbol;
		auto base_type = Symbol::map_type_to_llvm_type(sym->type.dropType(), get_ctx());
		return builder->CreateGEP(base_type, sym->alloca, idxs);
	}

	ASSERT(false, "expected ident or array lookup node on the left side");
}

llvm::Value *CodegenVisitor::code_gen(ArrLookupNode *node) {
	auto symbol = node->ident->symbol;

	if (!symbol->is_input) {
		return builder->CreateLoad(Symbol::map_type_to_llvm_type(node->get_type(), get_ctx()), get_address(node));
	}

	std::vector<llvm::Value*> idxs;
	idxs.reserve(node->idxs.size());
	for (const auto &idx: node->idxs) {
		idxs.push_back(idx->code_gen(this));
	}

	auto arr_sym = std::dynamic_pointer_cast<ArraySym>(symbol);
	return arr_sym->code_gen_idx(idxs, get_ctx());
}

extern "C" uint32_t get_property(PropertyLookupNode *node, uint8_t *data) {
	std::cout << "get property " << node << " " << (uint64_t)data << std::endl;
	auto sym = node->ident->symbol;
	if (sym->is_input) {
		auto arr = std::dynamic_pointer_cast<ArraySym>(sym);
		std::cout << "dyn cast got " << arr << std::endl;
		return arr->get_size();
	}
	return Symbol::allocated_vals[data];
}

llvm::Value *CodegenVisitor::code_gen(PropertyLookupNode *node) {
	auto sym = node->ident->symbol;
	auto property_cb_t = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx),
											{llvm::Type::getInt8PtrTy(*ctx),
											 Symbol::map_type_to_llvm_type(sym->type, get_ctx())}, false);

	auto property_cb = mod->getOrInsertFunction("get_property", property_cb_t);

	llvm::Value *data_ptr = Symbol::get_ptr(nullptr, get_ctx());
	if (!sym->is_input) {
		data_ptr = sym->alloca;
	}

	return builder->CreateCall(property_cb, {Symbol::get_ptr(node, get_ctx()), data_ptr});
}

extern "C" uint8_t *create_arr(uint32_t size) {
	auto *data = static_cast<uint8_t *>(malloc(size));
	Symbol::allocated_vals[data] = size;
	return data;
}

llvm::Value *CodegenVisitor::code_gen(ArrCreateNode *node) {
	auto pointed_sizeof = Symbol::create_symbol(Position(0, 0), node->type.dropType(), "tmp")->get_sizeof();

	auto create_arr_t = llvm::FunctionType::get(Symbol::map_type_to_llvm_type(node->type, get_ctx()),
												 {llvm::Type::getInt32Ty(*ctx)}, false);
	auto size = builder->CreateMul(builder->getInt32(pointed_sizeof), node->len->code_gen(this));

	auto create_arr_cb = mod->getOrInsertFunction("create_arr", create_arr_t);

	return builder->CreateCall(create_arr_cb, {size}, "created_arr_ptr");
}
