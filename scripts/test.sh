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

"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -I"$base/src" "$base/tests/test_registry.c" "$base/src/registry.c" -o "$base/build/host-tests/test_registry"
"$base/build/host-tests/test_registry"

"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -I"$base/src" "$base/tests/test_item_layout.c" "$base/src/item_layout.c" -o "$base/build/host-tests/test_item_layout"
"$base/build/host-tests/test_item_layout"
