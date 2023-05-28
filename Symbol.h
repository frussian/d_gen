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

	static std::unordered_map<uint8_t*, uint8_t> allocated_vals;

	virtual llvm::Value *code_gen(LLVMCtx ctx);

	llvm::AllocaInst *create_alloca(LLVMCtx ctx);

	static std::shared_ptr<Symbol> create_symbol(Position pos, Type type, std::string name, bool is_input = false);

	static llvm::Type *map_type_to_llvm_type(Type type, LLVMCtx ctx);

	virtual int get_sizeof();
	static llvm::Value *get_ptr(void *ptr, LLVMCtx ctx);
protected:
	explicit Symbol(Position pos, Type type, std::string name, bool is_input = false);
};

class ArraySym: public Symbol {
public:
	std::vector<std::shared_ptr<Symbol>> arr;
	//init when first access to "len" or some element
	std::optional<int> inited_size;

	//when access to element generate it with new Symbol
	//and insert at corresponding position but don't initialize it
	//if it's an array
	//int a[][]
	//a[2] - dont initialize
	//a[2][2] - initilize integer
	explicit ArraySym(Position pos, Type type, std::string name, bool is_input = false);
	llvm::Value *code_gen(LLVMCtx ctx) override;
	llvm::Value *code_gen_idx(std::vector<llvm::Value*> &idx, LLVMCtx ctx);
	int get_size();
	std::shared_ptr<Symbol> get_pointed_type_elem();
	int get_sizeof() override;

	static llvm::FunctionType *get_cb_func_type(llvm::Type *ret_type, llvm::LLVMContext *ctx);
};

class NumberSym: public Symbol {
public:
	std::optional<int> num;
	llvm::Value *code_gen(LLVMCtx ctx) override;

	explicit NumberSym(Position pos, Type type, std::string name, bool is_input = false);

	int get_sizeof() override;

	static llvm::FunctionType *get_cb_func_type(llvm::LLVMContext *ctx);
};

class CharSym: public Symbol {
public:
	std::optional<char> ch;
	llvm::Value *code_gen(LLVMCtx ctx) override;

	explicit CharSym(Position pos, Type type, std::string name, bool is_input = false);

	int get_sizeof() override;

	static llvm::FunctionType *get_cb_func_type(llvm::LLVMContext *ctx);
};

class BoolSym: public Symbol {
public:
	std::optional<bool> val;
	llvm::Value *code_gen(LLVMCtx ctx) override;

	explicit BoolSym(Position pos, Type type, std::string name, bool is_input = false);

	int get_sizeof() override;

	static llvm::FunctionType *get_cb_func_type(llvm::LLVMContext *ctx);
};

class StringSym: public ArraySym {
public:
	explicit StringSym(Position pos, Type type, std::string name, bool is_input = false);
	//todo: override generation of json value
};


#endif //D_GEN_SYMBOL_H
