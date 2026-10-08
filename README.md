# AROS ScreenBar

**A shared home for system status and application controls across AROS screens.**

AROS ScreenBar aims to turn the screen title bar into a consistent, extensible
part of the desktop: a place where essential controls stay within reach as you
move between screens, and running applications can bring their status and actions.

The goal is a bar that grows with what you are doing. A music player can add
playback controls, a file transfer can show progress, and a background service
can expose its state. Permanent system modules and application-provided items
share the same visual language and interaction rules. Users choose what appears;
developers contribute through one module contract.

## What we are building toward

- **Continuity across screens:** familiar controls and status wherever you work.
- **A bar that follows activity:** application items appear while their producers
  run and disappear when they are no longer needed.
- **A coherent extension system:** shared presentation, dropdowns and settings,
  with modules supplying data and actions.
- **User control:** module selection, ordering and per-application visibility
  through a dedicated preferences program.
- **A practical path into AROS:** standalone delivery for existing distributions
  alongside native system integration, targeting mainline and ABIv11.

This is the target experience. The [vision](docs/vision.md) explains the direction;
the [module contract](docs/module-contract.md) describes the extension model and
which interfaces already exist.

## Current prototype

The first implementation exercises clock, audio settings and screen selection
through a standalone borderless overlay. It is a starting point for the broader
system described above.

![AROS ScreenBar with the screen chooser open](docs/images/screenbar-dropdown.png)

The current prototype's screen chooser on its demonstration screen.

### Available today

- Live `HH:MM` clock using the screen font and colors.
- Speaker icon opens `SYS:Prefs/AHI`.
- Frameless screen chooser with highlighted selection and current-screen marker.
- Mouse selection, Up/Down, Return or keypad Enter, and Escape.
- Keyboard scrolling when the list exceeds the available height.
- Optional second public screen for trying screen switching.

```sh
ScreenBar --demo
```

Click the screen button again, press Escape in the chooser, or move focus away to
close it. Escape while the bar is focused, or Ctrl-C in its launching Shell, exits
ScreenBar. `--seconds N` limits a run; `--log PATH` records events.

## Status

The current standalone prototype is verified on x86_64 AROS ABIv11. Mainline ABI v1 has not
been verified. Only Workbench and the optional demo receive a bar; other public
screens can be selected. Unregistered private application screens are excluded. Registered screens that
become temporarily private may appear but cannot be selected.

The speaker is an AHI preferences shortcut, not a global volume control. No
autostart is installed. Outside clicks dismiss the chooser on focus loss and
also reach the underlying window. Overlay clicks can change focus; this is not
a system-integrated menu or a complete desktop service.

- [Vision and direction](docs/vision.md)
- [Developer module contract](docs/module-contract.md)
- [Build and tests](docs/development.md)
- [Architecture and limitations](docs/architecture.md)
