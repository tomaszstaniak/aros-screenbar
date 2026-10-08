#!/bin/sh
set -e
base=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
abi=${1:-abiv11}
case "$abi" in abiv11) compiler_key=SCREENBAR_CC_ABIV11; sdk_key=SCREENBAR_SDK_ABIV11;; abiv1) compiler_key=SCREENBAR_CC_ABIV1; sdk_key=SCREENBAR_SDK_ABIV1;; *) echo "Use abiv11 or abiv1" >&2; exit 2;; esac
# Preserve explicit environment overrides while reading the optional local file.
eval "compiler=\${$compiler_key-}; sysroot=\${$sdk_key-}"
if [ -f "$base/local.env" ]; then . "$base/local.env"; fi
if [ -z "$compiler" ]; then eval "compiler=\${$compiler_key-}"; fi
if [ -z "$sysroot" ]; then eval "sysroot=\${$sdk_key-}"; fi
[ -n "$compiler" ] && [ -n "$sysroot" ] || { echo "Set $compiler_key and $sdk_key in local.env or environment" >&2; exit 1; }
case "$compiler" in /*) ;; *) compiler="$base/$compiler";; esac
case "$sysroot" in /*) ;; *) sysroot="$base/$sysroot";; esac
[ -x "$compiler" ] && [ -d "$sysroot/include" ] || { echo "Compiler or SDK missing" >&2; exit 1; }
mkdir -p "$base/build/$abi"
"$compiler" --sysroot="$sysroot" -O2 -Wall -Wextra -Werror -o "$base/build/$abi/ScreenBar" "$base/src/screenbar.c" "$base/src/module.c" "$base/src/dropdown.c" "$base/src/popup.c" "$base/src/registry.c" "$base/src/item_layout.c" "$base/src/server.c"

"$compiler" --sysroot="$sysroot" -O2 -Wall -Wextra -Werror -I"$base/include" -o "$base/build/$abi/ScreenBarCounter" "$base/examples/counter.c" "$base/src/provider.c"

"$compiler" --sysroot="$sysroot" -O2 -Wall -Wextra -Werror -I"$base/include" -o "$base/build/$abi/ScreenBarSDKTest" "$base/tests/provider_smoke.c" "$base/src/provider.c"

"$compiler" --sysroot="$sysroot" -O2 -Wall -Wextra -Werror -I"$base/include" -c "$base/tests/sdk_cpp.cpp" -o "$base/build/$abi/sdk_cpp.o"
"$compiler" --sysroot="$sysroot" -O2 -Wall -Wextra -Werror -o "$base/build/$abi/ScreenBarCppTest" "$base/build/$abi/sdk_cpp.o" "$base/src/provider.c"

"$compiler" --sysroot="$sysroot" -O2 -Wall -Wextra -Werror -I"$base/include" -I"$base/src" -o "$base/build/$abi/ScreenBarServerTest" "$base/tests/server_smoke.c" "$base/src/server.c" "$base/src/registry.c"
