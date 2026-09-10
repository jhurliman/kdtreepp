#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${BUILD_DIR:-${ROOT}/build}"
if [[ ! -f "${BUILD}/compile_commands.json" ]]; then
  make -C "${ROOT}" debug BUILD_DIR="${BUILD}"
fi
python3 "${ROOT}/scripts/run-clang-tidy.py" -p "${BUILD}"
