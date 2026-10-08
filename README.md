# AROS ScreenBar

AROS ScreenBar brings system controls and application indicators to the screen
title bar, available across AROS screens.

Built-in modules and items added by running applications use the same interface
and appearance. Users choose which items to show and how to arrange them.

## Planned features

- System controls and status indicators available across screens.
- Application items added when an application starts and removed when it exits.
- One module interface, with shared styling, menus and settings.
- Preferences for enabling, ordering and configuring modules and application items.
- Standalone packages for existing distributions and native integration with
  AROS mainline and ABIv11.

See the [vision](docs/vision.md) and [module contract](docs/module-contract.md).

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

## Documentation

- [Vision and direction](docs/vision.md)
- [Developer module contract](docs/module-contract.md)
- [Build and tests](docs/development.md)
- [Architecture and limitations](docs/architecture.md)
