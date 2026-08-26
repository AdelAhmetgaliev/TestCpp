#!/usr/bin/env bash

set -euo pipefail

mode="format"

for arg in "$@"; do
    case "$arg" in
        --check) mode="check" ;;
        -h|--help)
            echo "Usage: $0 [--check]"
            echo "  no arguments — formats the source files"
            echo "  --check       — verifies formatting (for CI)"
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg" >&2
            exit 1
            ;;
    esac
done

if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found in PATH (clang-format >= 19 required)" >&2
    exit 1
fi

# Inside a work tree: tracked files plus untracked-but-not-ignored ones
# (so freshly created sources are formatted before their first `git add`);
# outside a work tree fall back to find.
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    mapfile -d '' sources < <(
        git ls-files -z --cached --others --exclude-standard \
            -- '*.hpp' '*.cpp' ':(exclude)third_party/*'
    )
else
    mapfile -d '' sources < <(
        find . \( -iname '*.hpp' -o -iname '*.cpp' \) \
            -not -path './build/*' -not -path './.cache/*' \
            -not -path './third_party/*' -print0
    )
fi

if [ "${#sources[@]}" -eq 0 ]; then
    echo "No C++ sources found"
    exit 0
fi

if [ "$mode" = "check" ]; then
    clang-format --dry-run --Werror "${sources[@]}"
else
    clang-format -i "${sources[@]}"
fi
