#!/usr/bin/env bash

set -eu
set -o pipefail

# https://clang.llvm.org/extra/clang-tidy/

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd ${DIR}/..

CLANG_TIDY_SCRIPT="${DIR}/run-clang-tidy.py"

# build the compile_commands.json file if it does not exist
if [[ ! -f build/compile_commands.json ]]; then
    # the build automatically puts the compile commands in the ./build directory
    make
fi

# change into the build directory so that clang-tidy can find the files
# at the right paths (since this is where the actual build happens)
cd build
${CLANG_TIDY_SCRIPT}
