//
// Created by Anton on 26.05.2023.
//

#include "Symbol.h"
#include "utils/assert.h"

extern "C" uint8_t *arr_rand_gen(ArraySym *arr);
extern "C" int8_t bool_rand_gen(BoolSym *sym);
extern "C" int32_t num_rand_gen(NumberSym *sym);
extern "C" int8_t char_rand_gen(CharSym *sym);

static void fill_dest(std::shared_ptr<Symbol> sym, uint8_t *dest) {
	auto pointed_sizeof = sym->get_sizeof();
	if (auto num = std::dynamic_pointer_cast<NumberSym>(sym)) {
		auto v = num_rand_gen(num.get());
		memcpy(dest, &v, pointed_sizeof);
	} else if (auto bool_v = std::dynamic_pointer_cast<BoolSym>(sym)) {
		auto v = bool_rand_gen(bool_v.get());
		memcpy(dest, &v, pointed_sizeof);
	} else if (auto char_v = std::dynamic_pointer_cast<CharSym>(sym)) {
		auto v = char_rand_gen(char_v.get());
		memcpy(dest, &v, pointed_sizeof);
	} else if (auto arr_v = std::dynamic_pointer_cast<ArraySym>(sym)) {
		auto v = arr_rand_gen(arr_v.get());
		memcpy(dest, &v, pointed_sizeof);
	} else {
		ASSERT(false, "unexpected type when constructing arr");
	}
}

std::unordered_map<uint8_t*, uint32_t> Symbol::allocated_vals;

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
	std::shared_ptr<std::vector<TypeKind>> str_t;

	switch (type.getCurrentType()) {
		case TypeKind::INT:
			return std::shared_ptr<Symbol>(new NumberSym(pos, type, std::move(name), is_input));
		case TypeKind::STRING:
			str_t = std::make_shared<std::vector<TypeKind>>();
			str_t->push_back(TypeKind::ARR);
			str_t->push_back(TypeKind::CHAR);
			return std::shared_ptr<Symbol>(new StringSym(pos, Type(str_t), std::move(name), is_input));
		case TypeKind::CHAR:
			return std::shared_ptr<Symbol>(new CharSym(pos, type, std::move(name), is_input));
		case TypeKind::BOOL:
			return std::shared_ptr<Symbol>(new BoolSym(pos, type, std::move(name), is_input));
		case TypeKind::ARR:
			return std::shared_ptr<Symbol>(new ArraySym(pos, type, std::move(name), is_input));
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

int Symbol::get_sizeof() {
	return 0;
}

std::string Symbol::serialize() {
	return "";
}

extern "C" int32_t num_rand_gen(NumberSym *sym) {
	if (!sym->num.has_value()) {
		//todo: use c++ 11 rand lib
		sym->num = std::rand() % 200;
	}
	std::cout << "get num " << *sym->num << std::endl;
	return *sym->num;
}

llvm::Value *NumberSym::code_gen(LLVMCtx ctx) {
	auto cb = ctx.mod->getOrInsertFunction("num_rand_gen", get_cb_func_type(ctx.ctx));
	auto ptr = Symbol::get_ptr(this, ctx);
	return ctx.builder->CreateCall(cb, {ptr});
}

llvm::FunctionType *NumberSym::get_cb_func_type(llvm::LLVMContext *ctx) {
	return llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx), {llvm::Type::getInt8PtrTy(*ctx)}, false);
}

NumberSym::NumberSym(Position pos, Type type, std::string name, bool is_input): Symbol(pos, type, name, is_input) {}

int NumberSym::get_sizeof() {
	return sizeof(uint32_t);
}

std::string NumberSym::serialize() {
	return std::to_string(num_rand_gen(this));
}

ArraySym::ArraySym(Position pos, Type type, std::string name, bool is_input): Symbol(pos, type, name, is_input) {}

extern "C" void get_val_arr(ArraySym *arr, int *idxs, int len, uint8_t *dest) {
	int i;
	for (i = 0; i < len - 1; i++) {
		auto arr_size = arr->get_size();
		if (idxs[i] >= arr_size) {
			throw std::runtime_error("out of bounds");
		}
		arr = dynamic_cast<ArraySym*>(arr->arr[idxs[i]].get());
	}

	auto arr_size = arr->get_size();
	if (idxs[i] >= arr_size) {
		throw std::runtime_error("out of bounds");
	}
	auto sym = arr->arr[idxs[i]];
	fill_dest(sym, dest);
}

extern "C" uint8_t *arr_rand_gen(ArraySym *arr) {
	int size = arr->get_size();
	int pointed_sizeof = arr->get_pointed_type_elem()->get_sizeof();
	auto *data = static_cast<uint8_t *>(malloc(size * pointed_sizeof));

	//TODO: store all mallocs to free them at the end
	Symbol::allocated_vals[data] = size;

	for (int i = 0; i < arr->arr.size(); i++) {
		auto val = arr->arr[i];
		fill_dest(val, data + i * pointed_sizeof);
	}

	return data;
}

llvm::Value *ArraySym::code_gen(LLVMCtx ctx) {
	//for deep copy
	auto ret_type = map_type_to_llvm_type(type, ctx);
	auto cb = ctx.mod->getOrInsertFunction("arr_rand_gen", get_cb_func_type(ret_type, ctx.ctx));
	auto ptr = Symbol::get_ptr(this, ctx);
	return ctx.builder->CreateCall(cb, {ptr});
}

llvm::FunctionType *ArraySym::get_cb_func_type(llvm::Type *ret_type, llvm::LLVMContext *ctx) {
	return llvm::FunctionType::get(ret_type, {llvm::Type::getInt8PtrTy(*ctx)}, false);
}

int ArraySym::get_size() {
	if (!inited_size.has_value()) {
		//TODO: change modulo
		inited_size = std::abs(std::rand() % 10);
		arr.reserve(*inited_size);
		for (int i = 0; i < *inited_size; i++) {
			arr.push_back(get_pointed_type_elem());
		}
	}
	std::cout << "get size " << *inited_size << std::endl;
	return *inited_size;
}

std::shared_ptr<Symbol> ArraySym::get_pointed_type_elem() {
	auto pointed_type = type.dropType();
	return create_symbol(pos, pointed_type, "", true);
}

int ArraySym::get_sizeof() {
	return sizeof(uint8_t*);
}

llvm::Value *ArraySym::code_gen_idx(std::vector<llvm::Value *> &idx, LLVMCtx ctx) {
	auto t = llvm::IntegerType::getInt32Ty(*ctx.ctx);
	auto var_arr = ctx.builder->CreateAlloca(t, ctx.builder->getInt32(idx.size()), "arr_idxs");
	for (int i = 0; i < idx.size(); i++) {
		auto el_ptr = ctx.builder->CreateGEP(t, var_arr, ctx.builder->getInt64(i));
		ctx.builder->CreateStore(idx[i], el_ptr);
	}

	auto referenced_t = type;
	for (int i = 0; i < idx.size(); i++) {
		referenced_t = referenced_t.dropType();
	}

	auto dest_val = ctx.builder->CreateAlloca(map_type_to_llvm_type(referenced_t, ctx), nullptr, "dest_val");

	//void (ArrSymbol *, int *idxs, int len, uint8_t *dest)
	auto idx_cb_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx.ctx),
										  {llvm::Type::getInt8PtrTy(*ctx.ctx),
										   llvm::Type::getInt32PtrTy(*ctx.ctx),
										   llvm::Type::getInt32Ty(*ctx.ctx),
										   dest_val->getAllocatedType()->getPointerTo()}, false);

	auto idx_cb = ctx.mod->getOrInsertFunction("get_val_arr", idx_cb_t);

	ctx.builder->CreateCall(idx_cb, {get_ptr(this, ctx), var_arr, ctx.builder->getInt32(idx.size()), dest_val});

	return ctx.builder->CreateLoad(dest_val->getAllocatedType(), dest_val);
}

std::string ArraySym::serialize() {
	std::string tmp = "[";

	//force to generate array
	get_size();

	for (auto &sym: arr) {
		tmp += sym->serialize() + ",";
	}
	tmp[tmp.size()-1] = ']';

	return tmp;
}

StringSym::StringSym(Position pos, Type type, std::string name, bool is_input):
	ArraySym(pos, type, std::move(name), is_input) {}

std::string StringSym::serialize() {
	std::string tmp;

	tmp += "\"";

	//to force generation of arr
	get_size();

	for (auto &el: arr) {
		auto char_sym = std::dynamic_pointer_cast<CharSym>(el);
		//TODO: escape char for json format
		tmp += char_rand_gen(char_sym.get());
	}

	tmp += "\"";
	return tmp;
}

extern "C" int8_t char_rand_gen(CharSym *sym) {
	if (!sym->ch.has_value()) {
		//todo: use c++ 11 rand lib
		//generating chars from 32 to 126
		int r = std::abs(std::rand() % 94);
		sym->ch = 32 + r;
	}
//	std::cout << "get char " << *sym->ch << "(" << (int)*sym->ch << ")" << std::endl;
	return *sym->ch;
}

llvm::Value *CharSym::code_gen(LLVMCtx ctx) {
	auto cb = ctx.mod->getOrInsertFunction("char_rand_gen", get_cb_func_type(ctx.ctx));
	auto ptr = Symbol::get_ptr(this, ctx);
	return ctx.builder->CreateCall(cb, {ptr});
}

CharSym::CharSym(Position pos, Type type, std::string name, bool is_input) : Symbol(pos, type, name, is_input) {}

llvm::FunctionType *CharSym::get_cb_func_type(llvm::LLVMContext *ctx) {
	return llvm::FunctionType::get(llvm::Type::getInt8Ty(*ctx), {llvm::Type::getInt8PtrTy(*ctx)}, false);
}

int CharSym::get_sizeof() {
	return sizeof(uint8_t);
}

std::string CharSym::serialize() {
	char c = char_rand_gen(this);

	return std::to_string(c);
}

extern "C" int8_t bool_rand_gen(BoolSym *sym) {
	if (!sym->val.has_value()) {
		//todo: use c++ 11 rand lib
		sym->val = std::abs(std::rand() % 2);
	}
	std::cout << "get bool " << *sym->val << std::endl;
	return (int8_t)*sym->val;
}

llvm::Value *BoolSym::code_gen(LLVMCtx ctx) {
	auto cb = ctx.mod->getOrInsertFunction("bool_rand_gen", get_cb_func_type(ctx.ctx));
	auto ptr = Symbol::get_ptr(this, ctx);
	return ctx.builder->CreateCall(cb, {ptr});
}

BoolSym::BoolSym(Position pos, Type type, std::string name, bool is_input) : Symbol(pos, type, name, is_input) {}

llvm::FunctionType *BoolSym::get_cb_func_type(llvm::LLVMContext *ctx) {
	return llvm::FunctionType::get(llvm::Type::getInt8Ty(*ctx), {llvm::Type::getInt8PtrTy(*ctx)}, false);
}

int BoolSym::get_sizeof() {
	return sizeof(uint8_t);
}

std::string BoolSym::serialize() {
	std::string val = "false";
	if (bool_rand_gen(this)) {
		val = "true";
	}
	return val;
}

