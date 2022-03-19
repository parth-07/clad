FROM gitpod/workspace-full

ENV PATH ${PATH}:/workspace/clad/scripts
COPY ./scripts/install_dependencies .
RUN ./install_dependencies.sh 10