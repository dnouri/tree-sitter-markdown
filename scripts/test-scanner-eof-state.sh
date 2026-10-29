#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=$(mktemp -d "$root/.scanner-eof-state.XXXXXX")
trap 'rm -rf "$build_dir"' EXIT HUP INT TERM
ulimit -c 0

"${CC:-cc}" -std=c11 -I"$root/tree-sitter-markdown/src" \
    "$root/tree-sitter-markdown/test/scanner-eof-state.c" \
    -o "$build_dir/scanner-eof-state"
"$build_dir/scanner-eof-state"
