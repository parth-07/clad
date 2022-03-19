if ! command -v clang-${1} &> /dev/null
then
  echo "Installing dependencies."
  install_dependencies.sh ${1}
fi

mkdir -p /workspace/builds/build-${1}
cd /workspace/builds/build-${1}
mkdir obj inst
cd obj
cmake -G Ninja ../../../clad -DClang_DIR=/usr/lib/llvm-${1} -DLLVM_DIR=/usr/lib/llvm-${1} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../inst -DLLVM_EXTERNAL_LIT="`which lit`"
ninja install