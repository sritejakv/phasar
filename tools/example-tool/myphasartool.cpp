/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

#include "phasar.h"
#include "phasar/Utils/IO.h"

#include <filesystem>
#include <string>

using namespace psr;

int main(int Argc, const char **Argv) {
  using namespace std::string_literals;

  if (Argc < 3 || !std::filesystem::exists(Argv[1]) ||
      std::filesystem::is_directory(Argv[1])) {
    llvm::errs() << "myphasartool\n"
                    "A small PhASAR-based example program\n\n"
                    "Usage: myphasartool <LLVM IR file> <Output folder>\n";
    return 1;
  }

  const std::filesystem::path OutputFolder(Argv[2]);
  if (!std::filesystem::exists(OutputFolder)) {
    // Create the directory
    if (std::filesystem::create_directory(OutputFolder)) {
        llvm::outs() << "Directory created: " << OutputFolder.string() << "\n";
    } else {
        llvm::outs() << "Failed to create directory: " << OutputFolder.string() << "\n";
    }
  } else {
      llvm::errs() << "Directory already exists: " << OutputFolder.string() << "\n";
      return 1;
  }

  LLVMProjectIRDB IRDB(Argv[1]);

  const LLVMBasedCFG CFG;
  for (const auto *Fun: IRDB.getAllFunctions()) {
    assert(Fun != nullptr && "Invalid function");
    if (Fun->hasName() && !Fun->isIntrinsic() && !Fun->isDeclaration()) {
      auto CFGJson = CFG.exportCFGAsSourceCodeJson(Fun);
      std::string OutputFile(OutputFolder.string() + "/" + Fun->getName().str() + ".json");
      writeTextFile(OutputFile, CFGJson.dump(2));
    }
  }

  return 0;
}
