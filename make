#!/usr/bin/env bash

set -euo pipefail

debug="${1:-}"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${BUILD_DIR:-"${repo_root}/out/build"}"
if command -v cygpath >/dev/null 2>&1; then
  build_dir="$(cygpath -u "${build_dir}")"
fi

if [[ "${build_dir}" != /* ]]; then
  build_dir="${repo_root}/${build_dir}"
fi

"${repo_root}/build"

bin="${build_dir}/software_renderer.exe"

test -f "${bin}" || exit 1

if [[ -z "${debug}" ]]; then
  (
    cd "$(dirname "${bin}")"
    "./$(basename "${bin}")"
  )
else
  (
    cd "$(dirname "${bin}")"
    raddbg "$(basename "${bin}")"
  )
fi

