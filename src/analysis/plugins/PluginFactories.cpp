/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

#include "PluginFactories.h"

// Maps for registering the plugins
map<string, unique_ptr<IFDSTabulationProblemPlugin> (*)(LLVMBasedICFG &,
                                                        vector<string>)>
    IFDSTabulationProblemPluginFactory;

map<string,
    unique_ptr<IDETabulationProblemPlugin> (*)(LLVMBasedICFG &, vector<string>)>
    IDETabulationProblemPluginFactory;

map<string, unique_ptr<IntraMonotoneProblemPlugin> (*)()>
    IntraMonotoneProblemPluginFactory;

map<string, unique_ptr<InterMonotoneProblemPlugin> (*)()>
    InterMonotoneProblemPluginFactory;

map<string,
    unique_ptr<ICFGPlugin> (*)(ProjectIRDB &, const vector<string> EntryPoints)>
    ICFGPluginFactory;