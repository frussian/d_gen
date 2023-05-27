//
// Created by Anton on 26.05.2023.
//

#ifndef D_GEN_SYMBOL_H
#define D_GEN_SYMBOL_H

#include "type.h"
#include "Position.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include "LLVMCtx.h"

class Symbol {
public:
	Position pos;
	Type type;
	std::string name;
	bool is_input = false;
	llvm::AllocaInst *alloca = nullptr;

	//get_val: function call or simple llvm value
	//set_val: llvm alloca for ident and

	virtual llvm::Value *code_gen(LLVMCtx ctx);

	llvm::AllocaInst *create_alloca(LLVMCtx ctx);

	static std::shared_ptr<Symbol> create_symbol(Position pos, Type type, std::string name, bool is_input = false);
protected:
	static llvm::Value *get_ptr(void *ptr, LLVMCtx ctx);

	explicit Symbol(Position pos, Type type, std::string name, bool is_input = false);

	static llvm::Type *map_type_to_llvm_type(Type type, LLVMCtx ctx);
};

class ArraySym: public Symbol {
public:
	std::vector<std::shared_ptr<Symbol>> arr;

	//init when first access to "len" or some element
	int inited_size = -1;

	//when access to element generate it with new Symbol
	//and insert at corresponding position but don't initialize it
	//if it's an array
	//int a[][]
	//a[2] - dont initialize
	//a[2][2] - initilize integer
	explicit ArraySym(Position pos, Type type, std::string name, bool is_input = false);
};

class NumberSym: public Symbol {
public:
	std::optional<int> num;
	llvm::Value *code_gen(LLVMCtx ctx) override;

	explicit NumberSym(Position pos, Type type, std::string name, bool is_input = false);

	static llvm::Type *get_cb_func_type(llvm::LLVMContext *ctx);
};

//TODO:
class StringSym: public ArraySym {
public:
	explicit StringSym(Position pos, Type type, std::string name, bool is_input = false);
	//todo: override generation of json value
};


#endif //D_GEN_SYMBOL_H
