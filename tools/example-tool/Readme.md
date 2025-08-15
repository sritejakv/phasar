# A simple PhASAR-based tool 

This tool takes an llvm-ir file and an output directory as inputs and outputs control-flow graphs of all the functions in the IR file into the output folder.

## The LLVM IR should be extracted with the following flags
If using clang compiler:
 - C++: -std=c++17 -fno-discard-value-names -emit-llvm -w
 - C: -fno-discard-value-names -emit-llvm -w

Otherwise with gclang compiler:
 - LLVM_BITCODE_GENERATION_FLAGS: -std=c++17 -fno-discard-value-names -g


## Usage

```
build/tools/myphasartool <path/to/ir_file> <output_directory>

# An example
build/tools/myphasartool test/llvm_test_code/basic/module.ll ./outputs

```