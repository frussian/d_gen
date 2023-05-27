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

class CodegenVisitor {
public:
	explicit CodegenVisitor();
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

	llvm::orc::ThreadSafeModule get_module();

	llvm::Value *code_gen();
private:
	std::unique_ptr<llvm::LLVMContext> ctx;
	std::unique_ptr<llvm::Module> mod;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	LLVMCtx get_ctx();
};

#endif //D_GEN_CODEGENVISITOR_H
