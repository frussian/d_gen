//
// Created by Anton on 27.05.2023.
//

#ifndef D_GEN_LLVMCTX_H
#define D_GEN_LLVMCTX_H

struct LLVMCtx {
	llvm::LLVMContext *ctx;
	llvm::Module *mod;
	llvm::IRBuilder<> *builder;
};

#endif //D_GEN_LLVMCTX_H
