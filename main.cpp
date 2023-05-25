#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>
#ifndef LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H
#define LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include "BuildError.h"
#include <memory>

namespace llvm::orc {

		class KaleidoscopeJIT {
		public:
			std::unique_ptr<ExecutionSession> ES;

			DataLayout DL;
			MangleAndInterner Mangle;

			RTDyldObjectLinkingLayer ObjectLayer;
			IRCompileLayer CompileLayer;

			JITDylib &MainJD;

		public:
			KaleidoscopeJIT(std::unique_ptr<ExecutionSession> ES,
							JITTargetMachineBuilder JTMB, DataLayout DL)
					: ES(std::move(ES)), DL(std::move(DL)), Mangle(*this->ES, this->DL),
					  ObjectLayer(*this->ES,
								  []() { return std::make_unique<SectionMemoryManager>(); }),
					  CompileLayer(*this->ES, ObjectLayer,
								   std::make_unique<ConcurrentIRCompiler>(std::move(JTMB))),
					  MainJD(this->ES->createBareJITDylib("<main>")) {
				MainJD.addGenerator(
						cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(
								this->DL.getGlobalPrefix())));
			}

			~KaleidoscopeJIT() {
				if (auto Err = ES->endSession())
					ES->reportError(std::move(Err));
			}

			static Expected<std::unique_ptr<KaleidoscopeJIT>> Create() {
				auto EPC = SelfExecutorProcessControl::Create();
				if (!EPC)
					return EPC.takeError();

				auto ES = std::make_unique<ExecutionSession>(std::move(*EPC));

				JITTargetMachineBuilder JTMB(
						ES->getExecutorProcessControl().getTargetTriple());

				auto DL = JTMB.getDefaultDataLayoutForTarget();
				if (!DL)
					return DL.takeError();

				return std::make_unique<KaleidoscopeJIT>(std::move(ES), std::move(JTMB),
														 std::move(*DL));
			}

			const DataLayout &getDataLayout() const { return DL; }

			JITDylib &getMainJITDylib() { return MainJD; }

			Error addModule(ThreadSafeModule TSM, ResourceTrackerSP RT = nullptr) {
				if (!RT)
					RT = MainJD.getDefaultResourceTracker();
				return CompileLayer.add(RT, std::move(TSM));
			}

			Expected<JITEvaluatedSymbol> lookup(StringRef Name) {
				return ES->lookup({&MainJD}, Mangle(Name.str()));
			}
		};

	} // end namespace llvm

#endif // LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H


//using namespace llvm::orc;

llvm::ExitOnError ExitOnErr;

extern "C" void cb_fn() {
	std::cout << "hello world!" << std::endl;
}

extern "C" double printd(double X) {
	fprintf(stderr, "%f\n", X);
	return 0;
}

llvm::orc::ThreadSafeModule createDemoModule(llvm::orc::ExecutionSession &ES, const llvm::DataLayout &DL) {
	auto Context = std::make_unique<llvm::LLVMContext>();
	auto M = std::make_unique<llvm::Module>("test", *Context);
	// Create the add1 function entry and insert this entry into module M.  The
	// function will have a return type of "int" and take an argument of "int".
	llvm::Function *Add1F =
			llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(*Context),
											   {llvm::Type::getInt32Ty(*Context)}, false),
								   llvm::Function::ExternalLinkage, "add1", M.get());

	auto cb_t = llvm::FunctionType::get(llvm::Type::getVoidTy(*Context), false);
	llvm::orc::MangleAndInterner mng(ES, DL);
	auto cb = M->getOrInsertFunction(*mng("cb_fn"), cb_t);

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

	builder.CreateCall(cb);

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


	auto J = cantFail(llvm::orc::KaleidoscopeJIT::Create());
	auto M = createDemoModule(*J->ES, J->getDataLayout());
	cantFail(J->addModule(std::move(M)));
	// Look up the JIT'd function, cast it to a function pointer, then call it.
	auto Add1Addr = ExitOnErr(J->lookup("add1"));
	auto Add1 = (int(*)(int))Add1Addr.getAddress();
	int Result = Add1(42);
	llvm::outs() << "add1(42) = " << Result << "\n";

	auto sinSymbol = ExitOnErr(J->lookup("printd"));
	ExitOnErr(J->lookup("cb_fn"));
	llvm::outs() << sinSymbol.getAddress() << "\n";
	try {
		antlr_test();
	} catch (const BuildError &err) {
		std::cout << "errors" << std::endl;
		for (const auto &e: err.errors) {
			std::cout << e.msg << std::endl;
		}
	} catch (const std::exception &err) {
		std::cout << "error" << std::endl;
		std::cout << err.what() << std::endl;
	}
//	J->getExecutionSession().dump(outs());
	return 0;
}
