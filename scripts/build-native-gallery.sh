#!/bin/sh
set -e
base=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
compiler_override=${SCREENBAR_CC_ABIV11-}
sysroot_override=${SCREENBAR_SDK_ABIV11-}

if [ -f "$base/local.env" ]; then
    . "$base/local.env"
fi

compiler=${compiler_override:-${SCREENBAR_CC_ABIV11-}}
sysroot=${sysroot_override:-${SCREENBAR_SDK_ABIV11-}}

[ -n "$compiler" ] && [ -n "$sysroot" ] || {
    echo "Set SCREENBAR_CC_ABIV11 and SCREENBAR_SDK_ABIV11 for the patched v11 SDK" >&2
    exit 1
}
case "$compiler" in /*) ;; *) compiler="$base/$compiler" ;; esac
case "$sysroot" in /*) ;; *) sysroot="$base/$sysroot" ;; esac
[ -x "$compiler" ] && [ -d "$sysroot/include" ] || {
    echo "Compiler or patched SDK is missing" >&2
    exit 1
}

mkdir -p "$base/build/native-v11"
"$compiler" --sysroot="$sysroot" -O2 -Wall -Wextra -Werror -I"$base/src" \
    -o "$base/build/native-v11/ScreenBarNativeGallery" \
    "$base/examples/native_widget_gallery.c" "$base/src/widget_visual.c"
