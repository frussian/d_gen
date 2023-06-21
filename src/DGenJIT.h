//
// Created by Anton on 27.05.2023.
//

#ifndef D_GEN_DGENJIT_H
#define D_GEN_DGENJIT_H

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


#include <memory>

class DGenJIT {
private:
	std::unique_ptr<llvm::orc::ExecutionSession> ES;

	llvm::DataLayout DL;
	llvm::orc::MangleAndInterner Mangle;

	llvm::orc::RTDyldObjectLinkingLayer ObjectLayer;
	llvm::orc::IRCompileLayer CompileLayer;

	llvm::orc::JITDylib &MainJD;

public:
	DGenJIT(std::unique_ptr<llvm::orc::ExecutionSession> ES,
			llvm::orc::JITTargetMachineBuilder JTMB, llvm::DataLayout DL);

	~DGenJIT();

	static llvm::Expected<std::unique_ptr<DGenJIT>> Create();

	const llvm::DataLayout &getDataLayout() const;

	llvm::orc::JITDylib &getMainJITDylib();

	llvm::Error addModule(llvm::orc::ThreadSafeModule TSM, llvm::orc::ResourceTrackerSP RT = nullptr);

	llvm::Expected<llvm::JITEvaluatedSymbol> lookup(llvm::StringRef Name);
};

#endif //D_GEN_DGENJIT_H
