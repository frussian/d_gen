//
// Created by Anton on 29.05.2023.
//

#ifndef D_GEN_CODEGENZ3VISITOR_H
#define D_GEN_CODEGENZ3VISITOR_H

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include <memory>

#include "ast.h"

#include <z3++.h>

class CodegenVisitor;
class CodegenZ3Visitor;

extern "C" void z3_gen(CodegenZ3Visitor *visitor, ASTNode *cond, PrecondNode *pre_cond);

class CodegenZ3Visitor {
	friend void ::z3_gen(CodegenZ3Visitor *visitor, ASTNode *cond, PrecondNode *pre_cond);
public:
	explicit CodegenZ3Visitor(llvm::LLVMContext *ctx,
							  llvm::Module *mod,
							  llvm::IRBuilder<> *builder, CodegenVisitor *cg_visitor);
	llvm::Value *prepare_eval_ctx(ASTNode *cond, PrecondNode *pre_cond);
	llvm::Value *prepare_eval_ctx(IdentNode *node);
	llvm::Value *prepare_eval_ctx(PropertyLookupNode *node);
	llvm::Value *prepare_eval_ctx(ArrLookupNode *node);

	z3::expr gen_expr(BoolNode *node);
	z3::expr gen_expr(CharNode *node);
	z3::expr gen_expr(NumberNode *node);
	z3::expr gen_expr(IdentNode *node);
	z3::expr gen_expr(PropertyLookupNode *node);
	z3::expr gen_expr(ArrLookupNode *node);
	z3::expr gen_expr(BinOpNode *node);

	//TODO: property lookup
private:
	llvm::LLVMContext *ctx;
	llvm::Module *mod;
	llvm::IRBuilder<> *builder;
	CodegenVisitor *cg_vis;

	z3::context z3_ctx;
	std::unordered_map<Symbol*, int> syms_to_expr_id;
    z3::expr_vector exprs;
	void start_z3_gen(ASTNode *cond, PrecondNode *pre_cond);

	z3::expr get_expr_from_void(void *ptr, Type type);

	LLVMCtx get_ctx();
	static bool traverse_ast_cb(ASTNode *node, std::any ctx);
};


#endif //D_GEN_CODEGENZ3VISITOR_H
