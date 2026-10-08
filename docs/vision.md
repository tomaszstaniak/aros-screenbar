# ScreenBar vision

AROS ScreenBar should make useful system state and application actions available
in the screen title bar, with a consistent appearance and predictable interaction.
A user should be able to change screens and still find the same controls.

## The experience we want

A small permanent set shows essentials such as time, audio and screen selection.
Running applications can add temporary items: a player's controls, a transfer's
progress or a background service's status. When the producer exits, its item goes
away. Users control which items appear and in what order.

A click opens a compact dropdown anchored to the item. Keyboard navigation works
consistently. ScreenBar handles fonts, colors, spacing and states so extensions
feel like parts of one environment. Under tight space, optional items move into
overflow while native screen gadgets remain usable.

The screen title bar remains a shared system surface. ScreenBar must cooperate
with menus, depth switching and screen dragging, and release its views when screens
close. More indicators are useful only when this foundation is reliable.

## One system, two delivery paths

The standalone version makes incremental improvements available on existing AROS
distributions without requiring a patched system. Native Intuition/Decoration
integration should eventually provide correct screen lifetime, placement and input.

Both paths share module state, actions and presentation rules. Compatibility-mode
limits must be visible; native integration must remain independently testable.
Upstream acceptance is an aim, not a prerequisite for experimenting or shipping
standalone improvements.

## A small foundation for extensions

The host renders; modules supply data and actions. Permanent system providers and
application indicators use [one contract](module-contract.md). Start with built-ins,
then prove an executable-provider SDK with an example extension. An optional Lua
runner can follow once lifecycle and scheduling work reliably.

Preferences should offer module enablement, ordering, per-app visibility and typed
settings, with predictable Use, Save and Cancel behavior. Users should not have to
edit source code to configure their bar.

## Development sequence

1. Shared built-ins and consistent dropdown behavior.
2. A fourth module to prove extensibility, minimal preferences and reversible
   distribution packaging.
3. Native attachment with screen/input lifecycle and regression verification.
4. Application registration and an experimental external-provider SDK.
5. Additional system providers and optional scripting based on actual need.

The standalone and native paths can progress concurrently. Mainline ABI v1 and
ABIv11 are separate validation targets; support is claimed only after testing.
No calendar dates or upstream acceptance are promised by this sequence.

## Where we are now

The experimental standalone implementation has Clock, an AHI settings shortcut
and public-screen selection. It has a frameless chooser and shared built-in state.
It is verified on x86_64 ABIv11 and attaches to Workbench and its optional demo.

It does not yet provide global volume control, preferences, external modules,
application registration, universal screen attachment or verified mainline v1
support. Outside clicks currently reach the underlying window when dismissing the
chooser. These are explicit gaps, not completed features.

## How we judge progress

Adding a module should eventually require provider code and metadata, without
changing the renderer or dropdown engine. Multiple screen views should share one
data source. Removing a module or closing a screen should leave no orphan popup or
pending view work. Slow providers should not stall interaction. Installation and
removal should preserve user configuration and existing system behavior.

The project succeeds when these properties hold for useful real extensions,
not when the bar merely contains more icons.
