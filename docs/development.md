---
type: guide
updated: 2026-10-08
---

# Development

## Build

A matching AROS GCC and SDK are required. Copy `local.env.example` to ignored
`local.env`, set the compiler/sysroot, then run:

```sh
scripts/build.sh abiv11
scripts/test.sh
```

Environment overrides have precedence over local-file values. Relative values in
`local.env` resolve from the repository root. `scripts/build.sh abiv1` uses its own
compiler/SDK and output directory; a successful v11 build does not verify v1.

The build produces `build/<abi>/ScreenBar` and `ScreenBarCounter`, plus
`ScreenBarSDKTest`, `ScreenBarServerTest` and a C++ link-check executable. No strip, installation or deployment
is performed. Run on an ABI-matched system with `ScreenBar --demo`; stop with
Escape or Ctrl-C. Host tests cover layout boundaries, module state/actions, dropdown selection,
scrolling, hit regions and key mapping. Intuition behavior requires runtime tests.

## Building an application provider

Compile your program with `include/` on its include path and link
`src/provider.c`, using the same compiler and SDK as the host:

```sh
"$AROS_CC" --sysroot="$AROS_SDK" -O2 -Wall -Wextra -Werror -Iinclude \
  -o MyProvider my-provider.c src/provider.c
```

See [Counter](../examples/counter.c) and the [module contract](module-contract.md).
Keep acquisition and actions in your application loop. Update even when visible
text stays unchanged so the registration lease stays live. On orderly exit, close
the client before its owning task ends. Install/run a separate executable for each
supported CPU/ABI; no Lua runner or binary plugin loader is required.

## Target-side SDK checks

Run ScreenBar in another Shell, then:

```sh
ScreenBarSDKTest RAM:sdk-test.log
Type RAM:sdk-test.log
```

The test covers two identities, copy validation, duplicate registration, updates,
unregistration, lease expiry and re-registration. It ends by itself and records
PASS/FAIL lines. UI actions and host restart need separate runtime checks.

With ScreenBar stopped, `ScreenBarServerTest RAM:server-test.log` checks duplicate
host rejection and replies to requests queued during normal server shutdown.
`ScreenBarCppTest` checks opening and closing a client from C++ code.
