#include <iostream>
#include <memory>

#include "gtest/gtest.h"

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/raw_ostream.h"

#include "phasar/Config/Configuration.h"
#include "phasar/DB/ProjectIRDB.h"
#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"
#include "phasar/PhasarLLVM/Passes/ValueAnnotationPass.h"
#include "phasar/PhasarLLVM/Pointer/LLVMPointsToSet.h"
#include "phasar/PhasarLLVM/TypeHierarchy/LLVMTypeHierarchy.h"
#include "phasar/Utils/LLVMIRToSrc.h"
#include "phasar/Utils/LLVMShorthands.h"
#include "phasar/Utils/Logger.h"

using namespace std;
using namespace psr;

/* ============== TEST FIXTURE ============== */

class LLVMIRToSrcTest : public ::testing::Test {
protected:
  const std::string PathToLlFiles =
      PhasarConfig::getPhasarConfig().PhasarDirectory() +
      "build/test/llvm_test_code/llvmIRtoSrc/";

  unique_ptr<ProjectIRDB> IRDB;
  unique_ptr<LLVMTypeHierarchy> TH;
  unique_ptr<LLVMPointsToSet> PT;
  unique_ptr<LLVMBasedICFG> ICFG;

  LLVMIRToSrcTest() = default;
  ~LLVMIRToSrcTest() override = default;

  void initialize(const std::vector<std::string> &IRFiles) {
    IRDB = make_unique<ProjectIRDB>(IRFiles, IRDBOptions::WPA);
    TH = make_unique<LLVMTypeHierarchy>(*IRDB);
    PT = make_unique<LLVMPointsToSet>(*IRDB);
    set<string> entry_points = {"main"};
    ICFG = make_unique<LLVMBasedICFG>(*IRDB, CallGraphAnalysisType::OTF,
                                      entry_points, TH.get(), PT.get());
  }

  void SetUp() override {
    boost::log::core::get()->set_logging_enabled(false);
    ValueAnnotationPass::resetValueID();
  }

  void TearDown() override {}
}; // Test Fixture

// TEST_F(LLVMIRToSrcTest, HandleInstructions) {
//   Initialize({pathToLLFiles + "function_call_cpp_dbg.ll"});
//   auto Fmain = ICFG->getMethod("main");
//   for (auto &BB : *Fmain) {
//     for (auto &I : BB) {
//       if (llvm::isa<llvm::StoreInst>(&I) ||
//           (llvm::isa<llvm::CallInst>(&I) &&
//            !llvm::isa<llvm::DbgValueInst>(&I) &&
//            !llvm::isa<llvm::DbgDeclareInst>(&I)) ||
//           llvm::isa<llvm::LoadInst>(&I)) {
//         std::cout << '\n'
//                   << llvmIRToString(&I) << "\n  --> "
//                   << llvmInstructionToSrc(&I) << std::endl;
//       }
//     }
//   }
// }

// TEST_F(LLVMIRToSrcTest, HandleFunctions) {
//   Initialize({pathToLLFiles + "multi_calls_cpp_dbg.ll"});
//   for (auto F : IRDB->getAllFunctions()) {
//     // F->print(llvm::outs());
//     // llvm::outs() << '\n';
//     std::cout << '\n' << llvmFunctionToSrc(F) << std::endl;
//   }
// }

// TEST_F(LLVMIRToSrcTest, HandleGlobalVariable) {
//   Initialize({pathToLLFiles + "global_01_cpp_dbg.ll"});
//   for (auto &GV :
//        IRDB->getModule(pathToLLFiles + "global_01_cpp_dbg.ll")->globals()) {
//     std::cout << '\n' << llvmGlobalValueToSrc(&GV) << std::endl;
//   }
// }

// TEST_F(LLVMIRToSrcTest, HandleAlloca) {
//   Initialize({pathToLLFiles + "function_call_cpp_dbg.ll"});
//   for (auto A : IRDB->getAllocaInstructions()) {
//     std::cout << '\n'
//               << llvmIRToString(A) << "\n  --> " << llvmValueToSrc(A)
//               << std::endl;
//   }
// }

int main(int Argc, char **Argv) {
  ::testing::InitGoogleTest(&Argc, Argv);
  return RUN_ALL_TESTS();
}
