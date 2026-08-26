#!/usr/bin/env bash

# Renames the template project: CMake targets, aliases, the debug macro,
# option mentions and the C++ namespace — in one command.
#
# Usage:
#   ./scripts/rename.sh NewProjectName [--dry-run]
#
# The current project name is parsed from the project() call in
# CMakeLists.txt, so the script can be run repeatedly.

set -euo pipefail

die() {
    echo "error: $*" >&2
    exit 1
}

dry_run=false
new_name=""

for arg in "$@"; do
    case "$arg" in
        -n|--dry-run) dry_run=true ;;
        -h|--help)
            echo "Usage: $0 NewProjectName [--dry-run]"
            echo ""
            echo "Renames the template: project name, CMake targets/aliases,"
            echo "debug macro and the C++ namespace (TestCpp -> NewName,"
            echo "TESTCPP -> NEWNAME, testcpp -> newname)."
            exit 0
            ;;
        -*)
            die "unknown option: $arg"
            ;;
        *)
            if [ -n "$new_name" ]; then
                die "exactly one project name expected"
            fi
            new_name="$arg"
            ;;
    esac
done

[ -n "$new_name" ] || die "project name is required (see --help)"

if ! [[ "$new_name" =~ ^[A-Za-z][A-Za-z0-9_-]*$ ]]; then
    die "'$new_name' must start with a letter and contain only" \
        "letters, digits, '_' and '-'"
fi

root="$(dirname "$(dirname "$(readlink -f "$0")")")"

cmake_lists="$root/CMakeLists.txt"
[ -f "$cmake_lists" ] || die "CMakeLists.txt not found next to the script"

# The project() call may be multi-line: join lines, take the FIRST
# occurrence of "project(" followed by an identifier.
old_name="$(tr '\n' ' ' < "$cmake_lists" \
    | grep -oE 'project\([[:space:]]*[A-Za-z0-9_-]+' \
    | head -n1 \
    | sed 's/^project([[:space:]]*//')"
[ -n "$old_name" ] || die "could not parse the project() name from CMakeLists.txt"

if [ "$old_name" = "$new_name" ]; then
    echo "Project is already named '$new_name', nothing to do."
    exit 0
fi

# Identifier forms of both names ('-' and other separators become '_').
sanitize_upper() {
    printf '%s' "$1" | tr '[:lower:]' '[:upper:]' | sed 's/[^A-Z0-9]/_/g'
}
sanitize_lower() {
    printf '%s' "$1" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9]/_/g'
}

old_upper="$(sanitize_upper "$old_name")"
old_lower="$(sanitize_lower "$old_name")"
new_upper="$(sanitize_upper "$new_name")"
new_lower="$(sanitize_lower "$new_name")"

cd "$root"

# Collect text files to touch. rename.sh itself must never be rewritten.
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    mapfile -t files < <(
        git ls-files \
            -- '*.txt' '*.json' '*.jsonc' '*.md' '*.hpp' '*.cpp' \
               ':(exclude)scripts/rename.sh'
    )
else
    mapfile -t files < <(
        find . \( -iname '*.hpp' -o -iname '*.cpp' \) \
            -not -path './build/*' -not -path './.cache/*' -type f
    )
    files+=("CMakeLists.txt" "CMakePresets.json" "README.md")
fi

if [ "${#files[@]}" -eq 0 ]; then
    die "no files matched for renaming"
fi

echo "Renaming template:"
echo "  project : $old_name    -> $new_name"
echo "  macro   : $old_upper   -> $new_upper"
echo "  namesp. : $old_lower   -> $new_lower"
echo ""

changed=0
for file in "${files[@]}"; do
    [ -f "$file" ] || continue

    # Leading \b only: a trailing one would miss compound identifiers
    # such as TESTCPP_ENABLE_LTO or TestCpp_exe.
    if ! grep -qE "\b(${old_upper}|${old_name}|${old_lower})" "$file"; then
        continue
    fi

    echo "  $file"
    changed=$((changed + 1))
    if $dry_run; then
        continue
    fi

    sed -i \
        -e "s/\b${old_upper}/${new_upper}/g" \
        -e "s/\b${old_name}/${new_name}/g" \
        -e "s/\b${old_lower}/${new_lower}/g" \
        "$file"
done

if [ "$changed" -eq 0 ]; then
    echo "Nothing matched; no changes made." >&2
    exit 1
fi

if $dry_run; then
    echo ""
    echo "Dry run: no files were modified."
    exit 0
fi

echo ""
echo "Done. Follow-up steps:"
echo "  1. rm -rf build/*          # old caches reference stale target names"
echo "  2. mv <repo-dir> ...       # rename the repository directory yourself"
echo "  3. ./scripts/format.sh     # keep formatting consistent"
