FROM gitpod/workspace-full

ENV PATH ${PATH}:/workspace/clad/scripts
RUN sudo apt-get update && sudo apt-get -y install\
    clang-10 libclang-10-dev llvm-10-tools llvm-10-dev &&\
    sudo rm -rf /var/lib/apt/lists/*
RUN sudo -H pip install lit