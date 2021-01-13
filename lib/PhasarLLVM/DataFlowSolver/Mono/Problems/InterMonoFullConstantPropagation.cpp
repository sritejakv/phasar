/******************************************************************************
 * Copyright (c) 2020 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert, Linus Jungemann and others
 *****************************************************************************/

#include <algorithm>
#include <ostream>
#include <unordered_map>
#include <utility>

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"

#include "phasar/DB/ProjectIRDB.h"
#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"
#include "phasar/PhasarLLVM/DataFlowSolver/Mono/Problems/InterMonoFullConstantPropagation.h"
#include "phasar/PhasarLLVM/Pointer/LLVMPointsToInfo.h"
#include "phasar/PhasarLLVM/TypeHierarchy/LLVMTypeHierarchy.h"
#include "phasar/Utils/BitVectorSet.h"
#include "phasar/Utils/LLVMShorthands.h"

using namespace std;
using namespace psr;

namespace psr {

InterMonoFullConstantPropagation::InterMonoFullConstantPropagation(
    const ProjectIRDB *IRDB, const LLVMTypeHierarchy *TH,
    const LLVMBasedICFG *ICF, const LLVMPointsToInfo *PT,
    std::set<std::string> EntryPoints)
    : IntraMonoFullConstantPropagation(IRDB, TH, ICF, PT, EntryPoints),
      InterMonoProblem<IntraMonoFullConstantPropagationAnalysisDomain>(
          IRDB, TH, ICF, PT, EntryPoints) {}

InterMonoFullConstantPropagation::mono_container_t
InterMonoFullConstantPropagation::merge(
    const InterMonoFullConstantPropagation::mono_container_t &Lhs,
    const InterMonoFullConstantPropagation::mono_container_t &Rhs) {
  // TODO implement
  return {};
}

bool InterMonoFullConstantPropagation::equal_to(
    const InterMonoFullConstantPropagation::mono_container_t &Lhs,
    const InterMonoFullConstantPropagation::mono_container_t &Rhs) {
  return IntraMonoFullConstantPropagation::equal_to(Lhs, Rhs);
}

std::unordered_map<InterMonoFullConstantPropagation::n_t,
                   InterMonoFullConstantPropagation::mono_container_t>
InterMonoFullConstantPropagation::initialSeeds() {
  return IntraMonoFullConstantPropagation::initialSeeds();
}

InterMonoFullConstantPropagation::mono_container_t
InterMonoFullConstantPropagation::normalFlow(
    InterMonoFullConstantPropagation::n_t Inst,
    const InterMonoFullConstantPropagation::mono_container_t &In) {
  return IntraMonoFullConstantPropagation::normalFlow(Inst, In);
}

InterMonoFullConstantPropagation::mono_container_t
InterMonoFullConstantPropagation::callFlow(
    InterMonoFullConstantPropagation::n_t CallSite,
    InterMonoFullConstantPropagation::f_t Callee,
    const InterMonoFullConstantPropagation::mono_container_t &In) {
  InterMonoFullConstantPropagation::mono_container_t Out;

  // Map the actual parameters into the formal parameters
  if (llvm::isa<llvm::CallInst>(CallSite) ||
      llvm::isa<llvm::InvokeInst>(CallSite)) {
    llvm::ImmutableCallSite CS(CallSite);
    // early exit; varargs not handled yet
    if (CS.getNumArgOperands() == 0 || Callee->isVarArg()) {
      return Out;
    }
    vector<const llvm::Value *> Actuals;
    vector<const llvm::Value *> Formals;
    // Set up the actual parameters
    for (unsigned idx = 0; idx < CS.getNumArgOperands(); ++idx) {
      Actuals.push_back(CS.getArgOperand(idx));
    }
    // Set up the formal parameters
    for (unsigned idx = 0; idx < Callee->arg_size(); ++idx) {
      Formals.push_back(Callee->getArg(idx));
    }
    // Perform mapping
    for (unsigned idx = 0; idx < Actuals.size(); ++idx) {
      auto Search = In.find(Actuals[idx]);
      if (Search != In.end()) {
        Out.insert({Formals[idx], Search->second});
      }
      // check for integer literals
      if (const auto *ConstInt =
              llvm::dyn_cast<llvm::ConstantInt>(Actuals[idx])) {
        std::cout << "Found literal!\n";
        Out.insert({Formals[idx], ConstInt->getSExtValue()});
      }
    }
  }
  // TODO: Handle globals
  /*
  if (llvm::isa<llvm::GlobalVariable>(source)) {
    res.insert(source);
  }*/
  return Out;
}

InterMonoFullConstantPropagation::mono_container_t
InterMonoFullConstantPropagation::returnFlow(
    InterMonoFullConstantPropagation::n_t CallSite,
    InterMonoFullConstantPropagation::f_t Callee,
    InterMonoFullConstantPropagation::n_t ExitStmt,
    InterMonoFullConstantPropagation::n_t RetSite,
    const InterMonoFullConstantPropagation::mono_container_t &In) {
  InterMonoFullConstantPropagation::mono_container_t Out;

  if (const auto *Return = llvm::dyn_cast<llvm::ReturnInst>(ExitStmt)) {
    if (Return->getReturnValue()->getType()->isIntegerTy()) {
      // Return value is integer literal
      if (auto ConstInt =
              llvm::dyn_cast<llvm::ConstantInt>(Return->getReturnValue())) {
        Out.insert({CallSite, ConstInt->getSExtValue()});
      } else {
        // handle return of integer variable
        auto Search = In.find(Return->getReturnValue());
        if (Search != In.end()) {
          std::cout << "Found const return variable\n";
          Out.insert({CallSite, Search->second});
        }
      }
      // handle Global Variables
      // TODO:handle globals
    }
  }
  return Out;
}

InterMonoFullConstantPropagation::mono_container_t
InterMonoFullConstantPropagation::callToRetFlow(
    InterMonoFullConstantPropagation::n_t CallSite,
    InterMonoFullConstantPropagation::n_t RetSite,
    std::set<InterMonoFullConstantPropagation::f_t> Callees,
    const InterMonoFullConstantPropagation::mono_container_t &In) {
  return In;
}

void InterMonoFullConstantPropagation::printNode(
    std::ostream &OS, InterMonoFullConstantPropagation::n_t Inst) const {
  IntraMonoFullConstantPropagation::printNode(OS, Inst);
}

void InterMonoFullConstantPropagation::printDataFlowFact(
    std::ostream &OS, InterMonoFullConstantPropagation::d_t Fact) const {
  IntraMonoFullConstantPropagation::printDataFlowFact(OS, Fact);
}

void InterMonoFullConstantPropagation::printFunction(
    std::ostream &OS, InterMonoFullConstantPropagation::f_t Fun) const {
  IntraMonoFullConstantPropagation::printFunction(OS, Fun);
}

void InterMonoFullConstantPropagation::printContainer(
    std::ostream &OS,
    InterMonoFullConstantPropagation::mono_container_t Con) const {
  for (const auto &[Var, Val] : Con) {
    OS << "<" << llvmIRToString(Var) << ", " << Val << ">, ";
  }
}

} // namespace psr
