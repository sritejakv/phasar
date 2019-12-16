/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

#ifndef PHASAR_PHASARLLVM_IFDSIDE_PROBLEMS_IDELINEARCONSTANTANALYSIS_H_
#define PHASAR_PHASARLLVM_IFDSIDE_PROBLEMS_IDELINEARCONSTANTANALYSIS_H_

#include <map>
#include <set>
#include <string>

#include <phasar/PhasarLLVM/DataFlowSolver/IfdsIde/EdgeFunctionComposer.h>
#include <phasar/PhasarLLVM/DataFlowSolver/IfdsIde/IDETabulationProblem.h>

namespace llvm {
class Instruction;
class Function;
class StructType;
class Value;
} // namespace llvm

namespace psr {

class LLVMBasedICFG;
class LLVMTypeHierarchy;
class LLVMPointsToInfo;

class IDELinearConstantAnalysis
    : public IDETabulationProblem<const llvm::Instruction *,
                                  const llvm::Value *, const llvm::Function *,
                                  const llvm::StructType *, const llvm::Value *,
                                  int64_t, LLVMBasedICFG> {
private:
  // For debug purpose only
  static unsigned CurrGenConstant_Id;
  static unsigned CurrLCAID_Id;
  static unsigned CurrBinary_Id;

public:
  typedef const llvm::Value *d_t;
  typedef const llvm::Instruction *n_t;
  typedef const llvm::Function *m_t;
  typedef const llvm::StructType *t_t;
  typedef const llvm::Value *v_t;
  typedef LLVMBasedICFG i_t;
  // int64_t corresponds to llvm's type of constant integer
  typedef int64_t l_t;

  static const l_t TOP;
  static const l_t BOTTOM;

  IDELinearConstantAnalysis(const ProjectIRDB *IRDB,
                            const LLVMTypeHierarchy *TH,
                            const LLVMBasedICFG *ICF,
                            const LLVMPointsToInfo *PT,
                            std::set<std::string> EntryPoints = {"main"});

  ~IDELinearConstantAnalysis() override;

  struct LCAResult {
    LCAResult() = default;
    unsigned line_nr = 0;
    std::string src_code;
    std::map<std::string, l_t> variableToValue;
    std::vector<n_t> ir_trace;
    void print(std::ostream &os);
  };

  typedef std::map<std::string, std::map<unsigned, LCAResult>> lca_restults_t;

  void stripBottomResults(std::unordered_map<d_t, l_t> &res);

  // start formulating our analysis by specifying the parts required for IFDS

  FlowFunction<d_t>* getNormalFlowFunction(n_t curr,
                                                           n_t succ) override;

  FlowFunction<d_t>* getCallFlowFunction(n_t callStmt,
                                                         m_t destMthd) override;

  FlowFunction<d_t>* getRetFlowFunction(n_t callSite,
                                                        m_t calleeMthd,
                                                        n_t exitStmt,
                                                        n_t retSite) override;

  FlowFunction<d_t>*
  getCallToRetFlowFunction(n_t callSite, n_t retSite,
                           std::set<m_t> callees) override;

  FlowFunction<d_t>*
  getSummaryFlowFunction(n_t callStmt, m_t destMthd) override;

  std::map<n_t, std::set<d_t>> initialSeeds() override;

  d_t createZeroValue() const override;

  bool isZeroValue(d_t d) const override;

  // in addition provide specifications for the IDE parts

  EdgeFunction<l_t>*
  getNormalEdgeFunction(n_t curr, d_t currNode, n_t succ,
                        d_t succNode) override;

  EdgeFunction<l_t>* getCallEdgeFunction(n_t callStmt,
                                                         d_t srcNode,
                                                         m_t destinationMethod,
                                                         d_t destNode) override;

  EdgeFunction<l_t>*
  getReturnEdgeFunction(n_t callSite, m_t calleeMethod, n_t exitStmt,
                        d_t exitNode, n_t reSite, d_t retNode) override;

  EdgeFunction<l_t>*
  getCallToRetEdgeFunction(n_t callSite, d_t callNode, n_t retSite,
                           d_t retSiteNode, std::set<m_t> callees) override;

  EdgeFunction<l_t>*
  getSummaryEdgeFunction(n_t callStmt, d_t callNode, n_t retSite,
                         d_t retSiteNode) override;

  l_t topElement() override;

  l_t bottomElement() override;

  l_t join(l_t lhs, l_t rhs) override;

  EdgeFunction<l_t>* allTopFunction() override;

  // Custom EdgeFunction declarations

  class LCAEdgeFunctionComposer : public EdgeFunctionComposer<l_t> {
  public:
    LCAEdgeFunctionComposer(EdgeFunction<l_t>* F,
                            EdgeFunction<l_t>* G)
        : EdgeFunctionComposer<l_t>(F, G){};

    EdgeFunction<l_t>*
    composeWith(EdgeFunction<l_t>* secondFunction) override;

    EdgeFunction<l_t>*
    joinWith(EdgeFunction<l_t>* otherFunction) override;
  };

  class GenConstant : public EdgeFunction<l_t>,
                      public std::enable_shared_from_this<GenConstant> {
  private:
    const unsigned GenConstant_Id;
    const l_t IntConst;

  public:
    explicit GenConstant(l_t IntConst);

    l_t computeTarget(l_t source) override;

    EdgeFunction<l_t>*
    composeWith(EdgeFunction<l_t>* secondFunction) override;

    EdgeFunction<l_t>*
    joinWith(EdgeFunction<l_t>* otherFunction) override;

    bool equal_to(EdgeFunction<l_t>* other) const override;

    void print(std::ostream &OS, bool isForDebug = false) const override;
  };

  class LCAIdentity : public EdgeFunction<l_t>,
                      public std::enable_shared_from_this<LCAIdentity> {
  private:
    const unsigned LCAID_Id;

  public:
    explicit LCAIdentity();

    l_t computeTarget(l_t source) override;

    EdgeFunction<l_t>*
    composeWith(EdgeFunction<l_t>* secondFunction) override;

    EdgeFunction<l_t>*
    joinWith(EdgeFunction<l_t>* otherFunction) override;

    bool equal_to(EdgeFunction<l_t>* other) const override;

    void print(std::ostream &OS, bool isForDebug = false) const override;
  };

  class BinOp : public EdgeFunction<l_t>,
                public std::enable_shared_from_this<BinOp> {
  private:
    const unsigned EdgeFunctionID, Op;
    d_t lop, rop, currNode;

  public:
    BinOp(const unsigned Op, d_t lop, d_t rop, d_t currNode);

    l_t computeTarget(l_t source) override;

    EdgeFunction<l_t>*
    composeWith(EdgeFunction<l_t>* secondFunction) override;

    EdgeFunction<l_t>*
    joinWith(EdgeFunction<l_t>* otherFunction) override;

    bool equal_to(EdgeFunction<l_t>* other) const override;

    void print(std::ostream &OS, bool isForDebug = false) const override;
  };

  // Helper functions

  /**
   * The following binary operations are computed:
   *  - addition
   *  - subtraction
   *  - multiplication
   *  - division (signed/unsinged)
   *  - remainder (signed/unsinged)
   *
   * @brief Computes the result of a binary operation.
   * @param op operator
   * @param lop left operand
   * @param rop right operand
   * @return Result of binary operation
   */
  static l_t executeBinOperation(const unsigned op, l_t lop, l_t rop);

  static char opToChar(const unsigned op);

  bool isEntryPoint(std::string FunctionName) const;

  void printNode(std::ostream &os, n_t n) const override;

  void printDataFlowFact(std::ostream &os, d_t d) const override;

  void printMethod(std::ostream &os, m_t m) const override;

  void printEdgeFact(std::ostream &os, l_t l) const override;

  lca_restults_t getLCAResults(SolverResults<n_t, d_t, l_t> SR);

  void emitTextReport(std::ostream &os,
                      const SolverResults<n_t, d_t, l_t> &SR) override;
};

} // namespace psr

#endif
