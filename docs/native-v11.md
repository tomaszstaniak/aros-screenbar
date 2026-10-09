# Native screen-bar gadgets on ABIv11

This experimental v11 path draws a BOOPSI screen gadget through the active
screen Decoration. The Decoration redraws its own bar background first and
Intuition then refreshes the screen-gadget list, so a module can draw text or
images without painting over the user's screen-bar theme.

## Source patch and build

`patches/aros-v11-native-screenbar.patch` targets AROS source commit
`fbc242c1046cffb5e5827a4626ee62bdd45664e9`. It adds three Intuition calls at
vectors 163–165 and advances the library to 50.11. Apply it only to that exact
source revision:

```sh
git checkout fbc242c1046cffb5e5827a4626ee62bdd45664e9
git apply --check patches/aros-v11-native-screenbar.patch
git apply patches/aros-v11-native-screenbar.patch
```

Build the x86_64 ABIv11 Intuition library using the normal AROS build setup.
The native clock example in `examples/native_clock.c` is compiled against the
matching generated headers and `libintuition.a`; it refuses to run against
Intuition older than 50.11. Keep the screen locked with `LockPubScreen()` while
the gadget is attached. Remove the gadget before freeing its BOOPSI object or
unlocking the screen.

The calls are:

- `AddScreenBarGadget(screen, gadget, position)` inserts the gadget into the
  screen gadget list and redraws the bar. It returns `TRUE` on success.
- `RefreshScreenBarGadget(screen, gadget)` redraws the Decoration and all screen
  gadgets, restoring the themed background before drawing again.
- `RemoveScreenBarGadget(screen, gadget)` waits for a held mouse button to be
  released, detaches the gadget, redraws, and returns its former list position;
  `(UWORD)-1` means failure.

These are experimental custom-library calls, not part of stock ABIv11 or the
mainline ABI v1 contract. Their vector allocation uses slots reserved in this
source tree and may need revision before upstreaming. Build the library and
applications from the same patched source and headers.

## Installing and restoring the library

Do this only on a disposable or backed-up ABIv11 system that matches the source
revision and architecture. The runtime replacement has not yet passed a VM
visual/input test, so keep a bootable recovery path available. Do not distribute
this library as a general v11 update.

Before replacing the system library, create a backup directory and save the
original file:

```text
MakeDir SYS:ScreenBarBackup
Copy SYS:Libs/intuition.library SYS:ScreenBarBackup/intuition.library
```

Verify that the backup exists and is non-empty, then copy the matching patched
x86_64 library to `SYS:Libs/intuition.library` and reboot. Keep the backup until
the patched system has booted and been checked. If startup fails, boot through a
recovery entry or from installation media, then restore the original library:

```text
Copy SYS:ScreenBarBackup/intuition.library SYS:Libs/intuition.library
```

Reboot after restoring. The original library remains on disk in
`SYS:ScreenBarBackup/intuition.library`; do not overwrite that backup when
repeating an installation. If the patched Workbench is usable, the same restore
copy can be issued from a Shell, followed by a reboot.

## Verification status

The custom Intuition library builds, exports all three calls, and the native
clock example compiles with `-Wall -Wextra -Werror`. A runtime check of themed
background rendering, mouse activation and removal is still pending. The
configured v11 VM pool currently reports its machines as `unknown`, so this
iteration has not installed or modified a guest library.
