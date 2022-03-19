FROM gitpod/workspace-full

RUN sudo apt-get update &&\
    sudo apt-get -y install\
    clang-8 libclang-8-dev llvm-8-tools llvm-8-dev\
    clang-9 libclang-9-dev llvm-9-tools llvm-9-dev\
    clang-10 libclang-10-dev llvm-10-tools llvm-10-dev\
    clang-11 libclang-11-dev llvm-11-tools llvm-11-dev\
    clang-12 libclang-12-dev llvm-12-tools llvm-12-dev &&\
    sudo rm -rf /var/lib/apt/lists/*

RUN sudo -H pip install lit