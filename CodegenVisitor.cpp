//
// Created by Anton on 27.05.2023.
//

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
	//TODO: process rhs

	return nullptr;
}

LLVMCtx CodegenVisitor::get_ctx() {
	return {ctx.get(), mod.get(), builder.get()};
}

llvm::Value *CodegenVisitor::code_gen(ReturnNode *node) {
	//TODO: call function to process the result
	return builder->CreateRetVoid();
}

llvm::orc::ThreadSafeModule CodegenVisitor::get_module() {
	return {std::move(mod), std::move(ctx)};
}

llvm::Value *CodegenVisitor::code_gen(AsgNode *node) {
	std::cout << "visiting asg node" << std::endl;
	//TODO: remove dynamic_cast, check all function
	//TODO: get_address method
	auto lhs = dynamic_cast<IdentNode*>(node->lhs);
//	auto lhs = node->lhs->code_gen(this);
	auto rhs = node->rhs->code_gen(this);
	auto alloca = lhs->symbol->alloca;

	return builder->CreateStore(rhs, alloca);
}
