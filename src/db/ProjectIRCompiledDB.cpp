#include "ProjectIRCompiledDB.hh"

ProjectIRCompiledDB::ProjectIRCompiledDB(
    const clang::tooling::CompilationDatabase &CompileDB) {
  for (auto file : CompileDB.getAllFiles()) {
    auto compilecommands = CompileDB.getCompileCommands(file);
    for (auto compilecommand : compilecommands) {
      vector<const char *> args;
      // save the filename
      source_files.insert(compilecommand.Filename);
      // prepare the compile command for the clang compiler
      args.push_back(
          compilecommand.CommandLine[compilecommand.CommandLine.size() - 1]
              .c_str());
      // create a diagnosticengine and the compiler instance that we use for
      // compilation
      clang::DiagnosticOptions DiagOpts;
      clang::TextDiagnosticPrinter *DiagPrinterClient =
          new clang::TextDiagnosticPrinter(llvm::errs(), &DiagOpts);
      llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(
          new clang::DiagnosticIDs());
      clang::DiagnosticsEngine Diags(DiagID, &DiagOpts, DiagPrinterClient,
                                     false);
      clang::CompilerInstance *ClangCompiler = new clang::CompilerInstance();
      ClangCompiler->createDiagnostics();
      // prepare CodeGenAction and Compiler invocation and compile!
      unique_ptr<clang::CodeGenAction> Action(new clang::EmitLLVMOnlyAction());
      unique_ptr<clang::CompilerInvocation> CI(new clang::CompilerInvocation);
      clang::CompilerInvocation::CreateFromArgs(*CI, &args[0],
                                                &args[0] + args.size(), Diags);
      ClangCompiler->setDiagnostics(&Diags);
      ClangCompiler->setInvocation(CI.get());
      if (!ClangCompiler->hasDiagnostics()) {
        cout << "compiler has no diagnostics engine" << endl;
      }
      if (!ClangCompiler->ExecuteAction(*Action)) {
        cout << "could not compile module!" << endl;
      }
      unique_ptr<llvm::Module> module = Action->takeModule();
      if (module != nullptr) {
        string name = module->getName().str();
        // check if module is alright
        bool broken_debug_info = false;
        if (llvm::verifyModule(*module, &llvm::errs(), &broken_debug_info)) {
          cout << "module is broken!\nabort!" << endl;
          DIE_HARD;
        }
        if (broken_debug_info) {
          cout << "debug info is broken" << endl;
        }
        contexts.insert(make_pair(
            name, unique_ptr<llvm::LLVMContext>(Action->takeLLVMContext())));
        modules.insert(make_pair(name, move(module)));
      } else {
        cout << "could not compile module!\nabort" << endl;
        DIE_HARD;
      }
    }
  }
}

ProjectIRCompiledDB::ProjectIRCompiledDB(const string Path,
                                         vector<const char *> CompileArgs) {
  source_files.insert(Path);
  if (Path.find(".ll") != Path.npos) {
    llvm::SMDiagnostic Diag;
    unique_ptr<llvm::LLVMContext> C(new llvm::LLVMContext);
    unique_ptr<llvm::Module> M = llvm::parseIRFile(Path, Diag, *C);
    bool broken_debug_info = false;
    if (llvm::verifyModule(*M, &llvm::errs(), &broken_debug_info)) {
      cout << "error: module not valid\n";
      DIE_HARD;
    }
    if (broken_debug_info) {
      cout << "caution: debug info is broken\n";
    }
    contexts.insert(make_pair(Path, move(C)));
    modules.insert(make_pair(Path, move(M)));
  } else {
    CompileArgs.insert(CompileArgs.begin(), Path.c_str());
    // create a diagnosticengine and the compiler instance that we use for
    // compilation
    clang::DiagnosticOptions DiagOpts;
    clang::TextDiagnosticPrinter *DiagPrinterClient =
        new clang::TextDiagnosticPrinter(llvm::errs(), &DiagOpts);
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(
        new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, &DiagOpts, DiagPrinterClient, false);
    clang::CompilerInstance *ClangCompiler = new clang::CompilerInstance();
    ClangCompiler->createDiagnostics();
    // prepare CodeGenAction and Compiler invocation and compile!
    unique_ptr<clang::CodeGenAction> Action(new clang::EmitLLVMOnlyAction());
    unique_ptr<clang::CompilerInvocation> CI(new clang::CompilerInvocation);
    clang::CompilerInvocation::CreateFromArgs(*CI, &CompileArgs[0],
                                              &CompileArgs[0] + CompileArgs.size(), Diags);
    ClangCompiler->setDiagnostics(&Diags);
    ClangCompiler->setInvocation(CI.get());
    if (!ClangCompiler->hasDiagnostics()) {
      cout << "compiler has no diagnostics engine" << endl;
    }
    if (!ClangCompiler->ExecuteAction(*Action)) {
      cout << "could not compile module!" << endl;
    }
    unique_ptr<llvm::Module> module = Action->takeModule();
    if (module != nullptr) {
      string name = module->getName().str();
      // check if module is alright
      bool broken_debug_info = false;
      if (llvm::verifyModule(*module, &llvm::errs(), &broken_debug_info)) {
        cout << "module is broken!\nabort!" << endl;
        DIE_HARD;
      }
      if (broken_debug_info) {
        cout << "debug info is broken" << endl;
      }
      contexts.insert(make_pair(
          name, unique_ptr<llvm::LLVMContext>(Action->takeLLVMContext())));
      modules.insert(make_pair(name, move(module)));
    } else {
      cout << "could not compile module!\nabort" << endl;
      DIE_HARD;
    }
  }
}

void ProjectIRCompiledDB::buildFunctionModuleMapping() {
  for (auto &entry : modules) {
    const llvm::Module *M = entry.second.get();
    for (auto &function : M->functions()) {
      if (!function.isDeclaration()) {
        functions[function.getName().str()] = M->getModuleIdentifier();
      }
    }
  }
}

void ProjectIRCompiledDB::buildGlobalModuleMapping() {
  for (auto& entry : modules) {
    const llvm::Module *M = entry.second.get();
    for (auto& global : M->globals()) {
      if (1==1) {
        globals[global.getName().str()] = M->getModuleIdentifier();
      }
    }
  }
}

void ProjectIRCompiledDB::buildIDModuleMapping() {
	// determine first instruction of module
	// determine last instruction of module (user reverse module iterator)
}

void ProjectIRCompiledDB::print() {
  cout << "modules:" << endl;
  for (auto &entry : modules) {
    cout << "front-end module: " << entry.first << endl;
    entry.second->dump();
  }
  cout << "functions:" << endl;
  for (auto entry : functions) {
    cout << entry.first << " defined in module " << entry.second << endl;
  }
}