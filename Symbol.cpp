//
// Created by Anton on 26.05.2023.
//

#include "Symbol.h"
#include "utils/assert.h"

Symbol::Symbol(Position pos, Type type, std::string name, bool is_input):
	pos(pos), type(type), name(std::move(name)), is_input(is_input) {}

llvm::Value *Symbol::code_gen(LLVMCtx /*ctx*/) {
	return nullptr;
}

llvm::Value *Symbol::get_ptr(void *ptr, LLVMCtx ctx) {
	auto ptr_int = ctx.builder->getInt64((uint64_t)ptr);
	return llvm::ConstantExpr::getIntToPtr(ptr_int, ctx.builder->getInt8PtrTy());
}

std::shared_ptr<Symbol> Symbol::create_symbol(Position pos, Type type, std::string name, bool is_input) {
	switch (type.getCurrentType()) {
		case TypeKind::INT:
			return std::shared_ptr<Symbol>(new NumberSym(pos, type, std::move(name), is_input));
		case TypeKind::STRING:
			return std::shared_ptr<Symbol>(new StringSym(pos, type, std::move(name), is_input));
			break;
		case TypeKind::CHAR:
			break;
		case TypeKind::BOOL:
			break;
		case TypeKind::ARR:
			break;
		case TypeKind::INVALID:
			break;
	}
	return nullptr;
}

llvm::AllocaInst *Symbol::create_alloca(LLVMCtx ctx) {
	auto t = map_type_to_llvm_type(type, ctx);
	alloca = ctx.builder->CreateAlloca(t, nullptr, name);
	return alloca;
}

llvm::Type *Symbol::map_type_to_llvm_type(Type type, LLVMCtx ctx) {
	llvm::Type *llvm_t = nullptr;
	auto scalar_t = type.types->at(type.types->size()-1);
	switch (scalar_t) {
		case TypeKind::INT:
			llvm_t = llvm::IntegerType::getInt32Ty(*ctx.ctx);
			break;
		case TypeKind::STRING:
			llvm_t = llvm::IntegerType::getInt8PtrTy(*ctx.ctx);
			break;
		case TypeKind::CHAR:
		case TypeKind::BOOL:
			llvm_t = llvm::IntegerType::getInt8Ty(*ctx.ctx);
			break;
		default:
			ASSERT(false, "unexpected type " + type.to_string() + ", expected scalar type");
	}

	for (int i = 0; i < type.length()-1; i++) {
		llvm_t = llvm_t->getPointerTo();
	}

	return llvm_t;
}

int32_t get_val_int(Symbol *) {

}

void set_val_arr(void *arr, int *idxs, int len, void *val) {

}

extern "C" int32_t num_node_gen(NumberSym *sym) {
	if (!sym->num.has_value()) {
		//todo: use c++ 11 rand lib
		sym->num = std::rand() % 200;
	}
	std::cout << "get num " << *sym->num << std::endl;
	return *sym->num;
}

llvm::Value *NumberSym::code_gen(LLVMCtx ctx) {
	auto cb = ctx.mod->getOrInsertFunction("num_node_gen", get_cb_func_type(ctx.ctx));
	auto ptr = Symbol::get_ptr(this, ctx);
	return ctx.builder->CreateCall(cb, {ptr});
}

llvm::FunctionType *NumberSym::get_cb_func_type(llvm::LLVMContext *ctx) {
	return llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx), {llvm::Type::getInt8PtrTy(*ctx)}, false);
}

NumberSym::NumberSym(Position pos, Type type, std::string name, bool is_input): Symbol(pos, type, name, is_input) {}


ArraySym::ArraySym(Position pos, Type type, std::string name, bool is_input): Symbol(pos, type, name, is_input) {}

StringSym::StringSym(Position pos, Type type, std::string name, bool is_input) : ArraySym(pos, type, name, is_input) {}
