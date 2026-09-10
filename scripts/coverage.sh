#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${BUILD_DIR:-${ROOT}/build-coverage}"
mkdir -p "${BUILD}"
BUILD="$(cd "${BUILD}" && pwd)"
cmake -S "${ROOT}" -B "${BUILD}" -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER="${CXX:-clang++}" -DKDTREEPP_BUILD_TESTS=ON \
  -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
  -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"
cmake --build "${BUILD}"
rm -f "${BUILD}"/coverage-*.profraw
LLVM_PROFILE_FILE="${BUILD}/coverage-%p.profraw" ctest --test-dir "${BUILD}" --output-on-failure
llvm-profdata merge -sparse "${BUILD}"/coverage-*.profraw -o "${BUILD}/coverage.profdata"
llvm-cov report "${BUILD}/kdtreepp-regression" -instr-profile="${BUILD}/coverage.profdata"
llvm-cov show "${BUILD}/kdtreepp-regression" -instr-profile="${BUILD}/coverage.profdata" \
  --format=html -output-dir="${BUILD}/coverage"
echo "HTML report: ${BUILD}/coverage/index.html"
