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

## Visual contract POC

`examples/native_widget_gallery.c` shows four host-rendered items: a
theme-colored monochrome mask, an alpha-composited color icon, a three-frame
animated icon and a 108-pixel-wide progress item. The sample synthesizes the
pixels in memory; it tests rendering and per-item geometry, not the final asset
file or provider wire formats. Build it against the matching patched SDK, which
must contain the new Intuition prototypes and CyberGraphX headers:

```sh
SCREENBAR_CC_ABIV11=/path/to/x86_64-aros-gcc \
SCREENBAR_SDK_ABIV11=/path/to/patched-v11-sdk \
  sh scripts/build-native-gallery.sh
```

The executable is written to `build/native-v11/ScreenBarNativeGallery`. Run it
on a system with the matching Intuition 50.11 library and
`cybergraphics.library`; it removes its gadgets when it exits. A timeout in
seconds may be passed as the first argument; Ctrl-C also exits. The gallery
shows a theme-colored monochrome mark, an alpha-color icon, a changing
three-frame icon and a labeled progress item. The animated strip and full
desktop capture are in `docs/evidence/`.

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

The custom Intuition library builds and exports all three calls. Host tests
cover animation-frame timing, animated-bar geometry and preferred-width
selection. On an x86_64 ABIv11 scratch VM, Intuition 50.11 booted and the gallery
rendered the mono, alpha-color, animated and progress examples over the native
bar. Two captures confirmed that the animated icon changes frames. Ctrl-C
removed the gadgets and restored the bar; the removal path redraws the full
screen-bar layer to clear old pixels. The original Intuition 50.10 library was
restored from backup and confirmed after reboot. This validates the standalone
POC on ABIv11 only; it does not establish a mainline ABI v1 interface.
