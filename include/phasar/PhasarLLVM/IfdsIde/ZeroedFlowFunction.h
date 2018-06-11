/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

/*
 * ZeroedFlowFunctions.h
 *
 *  Created on: 09.09.2016
 *      Author: pdschbrt
 */

#ifndef ANALYSIS_IFDS_IDE_ZEROEDFLOWFUNCTION_H_
#define ANALYSIS_IFDS_IDE_ZEROEDFLOWFUNCTION_H_

#include "FlowFunction.h"
#include <set>
using namespace std;
namespace psr {

template <typename D> class ZeroedFlowFunction : public FlowFunction<D> {
private:
  shared_ptr<FlowFunction<D>> delegate;
  D zerovalue;

public:
  ZeroedFlowFunction(shared_ptr<FlowFunction<D>> ff, D zv)
      : delegate(ff), zerovalue(zv) {}
  set<D> computeTargets(D source) override {
    if (source == zerovalue) {
      set<D> result = delegate->computeTargets(source);
      result.insert(zerovalue);
      return result;
    } else {
      return delegate->computeTargets(source);
    }
  }
};
} // namespace psr

#endif /* ANALYSIS_IFDS_IDE_ZEROEDFLOWFUNCTION_HH_ */
