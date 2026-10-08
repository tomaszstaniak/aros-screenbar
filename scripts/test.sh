#!/bin/sh
set -e
base=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
mkdir -p "$base/build/host-tests"
for name in layout modules dropdown; do
 sources=""
 case "$name" in modules) sources="$base/src/module.c";; dropdown) sources="$base/src/dropdown.c";; esac
 "${CC:-cc}" -Wall -Wextra -Werror -I"$base/src" "$base/tests/test_$name.c" $sources -o "$base/build/host-tests/test_$name"
 "$base/build/host-tests/test_$name"
 echo "$name tests passed"
done
