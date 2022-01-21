# syntax=docker/dockerfile:1
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive
ENV TZ America/Los_Angeles
ENV TERM xterm

RUN apt-get update

# Install basic dependencies
RUN apt-get install --no-install-recommends -y ninja-build cmake pip git python3 && \
    pip install lit

# Install llvm and clang
ARG LLVM_VERSION=10
ENV LLVM_HOME /usr/lib/llvm-${LLVM_VERSION}
RUN apt-get install --no-install-recommends -y clang-${LLVM_VERSION} \
    libclang-${LLVM_VERSION}-dev llvm-${LLVM_VERSION}-tools llvm-${LLVM_VERSION}-dev

ENV HOME /home/user

# Build and install clad
ENV CLAD_HOME ${HOME}/clad/inst
RUN mkdir -p ${HOME}/clad ${HOME}/clad/obj ${HOME}/clad/inst && \
    git clone https://github.com/vgvassilev/clad.git ${HOME}/clad/src
WORKDIR ${HOME}/clad/obj
RUN CC=${LLVM_HOME}/bin/clang CXX=${LLVM_HOME}/bin/clang++ \
    cmake -G "Ninja" ../src -DClang_DIR=${LLVM_HOME} -DLLVM_DIR=${LLVM_HOME} \
    -DCMAKE_INSTALL_PREFIX=../inst -DLLVM_EXTERNAL_LIT="`which lit`" && ninja install

# Set up build script to conveniently build and run files with clad enabled
COPY scripts/build build
RUN echo "LLVM_HOME: ${LLVM_HOME}\nCLAD_HOME: ${CLAD_HOME}" > .cladconfig.yaml && \
    pip3 install pyyaml && \
    chmod +x build
ENV PATH "${HOME}/clad/obj:${PATH}"

WORKDIR ${HOME}

ENTRYPOINT ["bash"]