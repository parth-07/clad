#!/usr/bin/env bash
sudo apt-get update && sudo apt-get -y install clang-${1} libclang-${1}-dev llvm-${1}-tools llvm-${1}-dev
sudo -H pip install lit