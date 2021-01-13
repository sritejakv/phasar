/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

/*
 * InterMonoTaintAnalysis.h
 *
 *  Created on: 22.08.2018
 *      Author: richard leer
 */

#ifndef PHASAR_PHASARLLVM_MONO_PROBLEMS_INTERMONOTAINTANALYSIS_H_
#define PHASAR_PHASARLLVM_MONO_PROBLEMS_INTERMONOTAINTANALYSIS_H_

#include <map>
#include <set>
#include <string>

#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"
#include "phasar/PhasarLLVM/DataFlowSolver/Mono/InterMonoProblem.h"
#include "phasar/PhasarLLVM/Domain/AnalysisDomain.h"
#include "phasar/PhasarLLVM/Utils/TaintConfiguration.h"
#include "phasar/Utils/BitVectorSet.h"

namespace llvm {
class Instruction;
class Value;
class Function;
class StructType;
} // namespace llvm

namespace psr {

class LLVMPointsToInfo;
class LLVMTypeHierarchy;

struct InterMonoTaintAnalysisDomain : LLVMAnalysisDomainDefault {
  using mono_container_t = BitVectorSet<LLVMAnalysisDomainDefault::d_t>;
};

class InterMonoTaintAnalysis
    : public InterMonoProblem<InterMonoTaintAnalysisDomain> {
public:
  using n_t = InterMonoTaintAnalysisDomain::n_t;
  using d_t = InterMonoTaintAnalysisDomain::d_t;
  using f_t = InterMonoTaintAnalysisDomain::f_t;
  using t_t = InterMonoTaintAnalysisDomain::t_t;
  using v_t = InterMonoTaintAnalysisDomain::v_t;
  using i_t = InterMonoTaintAnalysisDomain::i_t;
  using mono_container_t = InterMonoTaintAnalysisDomain::mono_container_t;
  using ConfigurationTy = TaintConfiguration<d_t>;

  InterMonoTaintAnalysis(const ProjectIRDB *IRDB, const LLVMTypeHierarchy *TH,
                         const LLVMBasedICFG *ICF, const LLVMPointsToInfo *PT,
                         const TaintConfiguration<d_t> &TSF,
                         std::set<std::string> EntryPoints = {});

  ~InterMonoTaintAnalysis() override = default;

  mono_container_t merge(const mono_container_t &Lhs,
                         const mono_container_t &Rhs) override;

  bool equal_to(const mono_container_t &Lhs,
                const mono_container_t &Rhs) override;

  mono_container_t normalFlow(n_t Inst, const mono_container_t &In) override;

  mono_container_t callFlow(n_t CallSite, f_t Callee,
                            const mono_container_t &In) override;

  mono_container_t returnFlow(n_t CallSite, f_t Callee, n_t ExitStmt,
                              n_t RetSite, const mono_container_t &In) override;

  mono_container_t callToRetFlow(n_t CallSite, n_t RetSite,
                                 std::set<f_t> Callees,
                                 const mono_container_t &In) override;

  std::unordered_map<n_t, mono_container_t> initialSeeds() override;

  void printNode(std::ostream &OS, n_t Inst) const override;

  void printDataFlowFact(std::ostream &OS, d_t Fact) const override;

  void printFunction(std::ostream &OS, f_t Fun) const override;

  const std::map<n_t, std::set<d_t>> &getAllLeaks() const;

private:
  const TaintConfiguration<d_t> &TSF;
  std::map<n_t, std::set<d_t>> Leaks;
};

} // namespace psr

#endif
