#!/bin/sh
set -e
base=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
abi=${1:-abiv11}
case "$abi" in
 abiv11) compiler=${SCREENBAR_CC_ABIV11-}; sdk=${SCREENBAR_SDK_ABIV11-};;
 abiv1) compiler=${SCREENBAR_CC_ABIV1-}; sdk=${SCREENBAR_SDK_ABIV1-};;
 *) echo "Use abiv11 or abiv1" >&2; exit 2;;
esac
[ -x "$compiler" ] && [ -d "$sdk/include" ] || {
 echo "Set ABI-matched SCREENBAR_CC and SCREENBAR_SDK variables" >&2; exit 1;
}
mkdir -p "$base/build/$abi"
"$compiler" --sysroot="$sdk" -O2 -Wall -Wextra -Werror \
 -o "$base/build/$abi/ScreenBarNativeProbe" "$base/examples/native_probe.c"
