#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>

#include "BuildError.h"
#include "DGenJIT.h"
#include <memory>

#include <stdexcept>

llvm::ExitOnError ExitOnErr;

extern "C" void cb_fn(uint32_t *arr) {
	//TODO: test throw
	std::cout << "Hello, World!" << std::endl;
	std::cout << "got arr: " << arr[0] << " and " << arr[1] << std::endl;
//	throw std::runtime_error("throw runtime error");
}

extern "C" void char_fn(unsigned char c) {
	std::cout << "got char " << c << " " << (int)c << std::endl;
}

extern "C" double printd(double X) {
	fprintf(stderr, "%f\n", X);
	return 0;
}

llvm::orc::ThreadSafeModule createDemoModule() {
	auto Context = std::make_unique<llvm::LLVMContext>();
	auto M = std::make_unique<llvm::Module>("test", *Context);
	// Create the add1 function entry and insert this entry into module M.  The
	// function will have a return type of "int" and take an argument of "int".
	llvm::Function *Add1F =
			llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(*Context),
											   {llvm::Type::getInt32Ty(*Context)}, false),
								   llvm::Function::ExternalLinkage, "add1", M.get());

	// Add a basic block to the function. As before, it automatically inserts
	// because of the last argument.
	llvm::BasicBlock *BB = llvm::BasicBlock::Create(*Context, "EntryBlock", Add1F);
	// Create a basic block builder with default parameters.  The builder will
	// automatically append instructions to the basic block 'BB'.
	llvm::IRBuilder<> builder(BB);
	// Get pointers to the constant `1'.
	llvm::Value *One = builder.getInt32(1);
	// Get pointers to the integer argument of the add1 function...
	assert(Add1F->arg_begin() != Add1F->arg_end()); // Make sure there's an arg
	llvm::Argument *ArgX = &*Add1F->arg_begin();          // Get the arg
	ArgX->setName("AnArg"); // Give it a nice symbolic name for fun.

	//test
	auto cb_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*Context), {llvm::Type::getInt32PtrTy(*Context)}, false);
	auto cb = M->getOrInsertFunction("cb_fn", cb_t);

	auto t = llvm::IntegerType::getInt32Ty(*Context);
//	auto arr_t = llvm::ArrayType::get(t, 2);

	auto var_arr = builder.CreateAlloca(t, builder.getInt32(2), "arr");
	auto el1_ptr = builder.CreateGEP(t, var_arr, builder.getInt64(0));
	builder.CreateStore(builder.getInt32(9000), el1_ptr);
	auto el2_ptr = builder.CreateGEP(t, var_arr, builder.getInt64(1));
	builder.CreateStore(builder.getInt32(50), el2_ptr);
	builder.CreateCall(cb, {var_arr});

	auto str_ptr = builder.CreateGlobalStringPtr("heystr", "str name");
	auto str_char_ptr = builder.CreateGEP(builder.getInt8Ty(), str_ptr, builder.getInt64(1));
	auto str_char = builder.CreateLoad(builder.getInt8Ty(), str_char_ptr);
	auto char_fn_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*Context), {llvm::Type::getInt8Ty(*Context)}, false);
	auto char_fn = M->getOrInsertFunction("char_fn", char_fn_t);
	builder.CreateCall(char_fn, {str_char});

	//end test

	// Create the add instruction, inserting it into the end of BB.
	llvm::Value *Add = builder.CreateAdd(One, ArgX);
	// Create the return instruction and add it to the basic block
	builder.CreateRet(Add);

	M->print(llvm::errs(), nullptr);

	return llvm::orc::ThreadSafeModule(std::move(M), std::move(Context));
}

void antlr_test();

int main(int argc, char *argv[]) {
	// Initialize LLVM.
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();

//	cantFail(LLJITBuilder().create())->addIRModule());


	auto J = cantFail(DGenJIT::Create());
	auto M = createDemoModule();
	cantFail(J->addModule(std::move(M)));
	// Look up the JIT'd function, cast it to a function pointer, then call it.
	auto Add1Addr = ExitOnErr(J->lookup("add1"));
	auto Add1 = (int(*)(int))Add1Addr.getAddress();
	try {
		int Result = Add1(42);
		llvm::outs() << "add1(42) = " << Result << "\n";
	} catch (const std::runtime_error &err) {
		std::cout << "runtime error" << std::endl;
		std::cout << err.what() << std::endl;
	}

	auto sinSymbol = ExitOnErr(J->lookup("printd"));
	ExitOnErr(J->lookup("cb_fn"));
	llvm::outs() << sinSymbol.getAddress() << "\n";
	try {
		antlr_test();
	} catch (const BuildError &err) {
		std::cout << "errors" << std::endl;
		for (const auto &e: err.errors) {
			std::cout << e.pos.line << ":" << e.pos.col << " " << e.msg << std::endl;
		}
	}
//	catch (const std::exception &err) {
//		std::cout << "error" << std::endl;
//		std::cout << err.what() << std::endl;
//	}
//	J->getExecutionSession().dump(outs());
	return 0;
}
