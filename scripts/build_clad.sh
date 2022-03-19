mkdir -p /workspace/builds/build-${1}
cd /workspace/builds/build-10
mkdir obj inst
cd obj
cmake -G Ninja ../../../clad -DClang_DIR=/usr/lib/llvm-${1} -DLLVM_DIR=/usr/lib/llvm-${1} -DCMAKE_INSTALL_PREFIX=../inst -DLLVM_EXTERNAL_LIT="`which lit`"
ninja install