//
// Created by Anton on 27.05.2023.
//

#ifndef D_GEN_CODEGENVISITOR_H
#define D_GEN_CODEGENVISITOR_H

#include <memory>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>

#include "ast.h"
#include "LLVMCtx.h"

class DGen;

class CodegenVisitor {
public:
	explicit CodegenVisitor(DGen *d_gen);
	llvm::Value *code_gen(FunctionNode *func);
	llvm::Value *code_gen(BodyNode *body);
	llvm::Value *code_gen(BoolNode *node);
	llvm::Value *code_gen(NumberNode *node);
	llvm::Value *code_gen(StringNode *node);
	llvm::Value *code_gen(CharNode *node);
	llvm::Value *code_gen(BinOpNode *node);
	llvm::Value *code_gen(IdentNode *node);
	llvm::Value *code_gen(DefNode *node);
	llvm::Value *code_gen(ReturnNode *node);
	llvm::Value *code_gen(AsgNode *node);

	llvm::Value *code_gen(ArrLookupNode *node);
	llvm::Value *code_gen(PropertyLookupNode *node);
	llvm::Value *code_gen(ArrCreateNode *node);

	llvm::Value *code_gen(IfNode *node);
	llvm::Value *code_gen(ForNode *node);
	llvm::Value *code_gen(ContinueNode *node);
	llvm::Value *code_gen(BreakNode *node);

	bool is_last_stmt_br(BodyNode *node);

	llvm::orc::ThreadSafeModule get_module();
	DGen *d_gen;
private:
	std::unique_ptr<llvm::LLVMContext> ctx;
	std::unique_ptr<llvm::Module> mod;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	LLVMCtx get_ctx();
	llvm::Value *get_address(ASTNode *node);
	FunctionNode *func;
};

#endif //D_GEN_CODEGENVISITOR_H
