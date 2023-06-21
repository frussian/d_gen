//
// Created by Anton on 27.05.2023.
//

#include "DGenJIT.h"

DGenJIT::DGenJIT(std::unique_ptr<llvm::orc::ExecutionSession> ES, llvm::orc::JITTargetMachineBuilder JTMB,
				 llvm::DataLayout DL)
		: ES(std::move(ES)), DL(std::move(DL)), Mangle(*this->ES, this->DL),
		  ObjectLayer(*this->ES,
					  []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
		  CompileLayer(*this->ES, ObjectLayer,
					   std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(JTMB))),
		  MainJD(this->ES->createBareJITDylib("<main>")) {
	MainJD.addGenerator(
			cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
					this->DL.getGlobalPrefix())));
}

DGenJIT::~DGenJIT() {
	if (auto Err = ES->endSession())
		ES->reportError(std::move(Err));
}

llvm::Expected<std::unique_ptr<DGenJIT>> DGenJIT::Create() {
	auto EPC = llvm::orc::SelfExecutorProcessControl::Create();
	if (!EPC)
		return EPC.takeError();

	auto ES = std::make_unique<llvm::orc::ExecutionSession>(std::move(*EPC));

	llvm::orc::JITTargetMachineBuilder JTMB(
			ES->getExecutorProcessControl().getTargetTriple());

	auto DL = JTMB.getDefaultDataLayoutForTarget();
	if (!DL)
		return DL.takeError();

	return std::make_unique<DGenJIT>(std::move(ES), std::move(JTMB),
									 std::move(*DL));
}

llvm::Error DGenJIT::addModule(llvm::orc::ThreadSafeModule TSM, llvm::orc::ResourceTrackerSP RT) {
	if (!RT)
		RT = MainJD.getDefaultResourceTracker();
	return CompileLayer.add(RT, std::move(TSM));
}

llvm::Expected<llvm::JITEvaluatedSymbol> DGenJIT::lookup(llvm::StringRef Name) {
	return ES->lookup({&MainJD}, Mangle(Name.str()));
}

const llvm::DataLayout &DGenJIT::getDataLayout() const { return DL; }

llvm::orc::JITDylib &DGenJIT::getMainJITDylib() { return MainJD; }
