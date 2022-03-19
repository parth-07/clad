FROM gitpod/workspace-full

USER root

ENV PATH ${PATH}:/workspace/clad/scripts:/workspace/scripts
RUN install-packages \
    clang-11 libclang-11-dev llvm-11-tools llvm-11-dev
RUN sudo -H pip install lit
