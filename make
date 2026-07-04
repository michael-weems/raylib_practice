#!/usr/bin/env bash

set -euo pipefail

debug="${1:-}"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"${repo_root}/build"

bin="${repo_root}/out/build/software_renderer.exe"

test -f "${bin}" || exit 1

if [[ -z "${debug}" ]]; then
  "${bin}"
else
  raddbg "${bin}"
fi

