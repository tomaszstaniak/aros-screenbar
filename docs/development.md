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

The build produces `build/<abi>/ScreenBar`. No strip, installation or deployment
is performed. Run on an ABI-matched system with `ScreenBar --demo`; stop with
Escape or Ctrl-C. Host tests cover layout boundaries, module state/actions, dropdown selection,
scrolling, hit regions and key mapping. Intuition behavior requires runtime tests.
