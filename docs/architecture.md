---
type: reference
updated: 2026-10-08
---

# Architecture

`src/screenbar.c` owns screen views, presentation and input. `src/item_layout.c`
allocates a right-hand strip outside the title envelope and native right-relative
gadgets. Excess items appear in overflow. A change in geometry or item identity
closes and recreates the overlay, replying to its old events before replacing it;
old click coordinates cannot select a newly placed item.

`src/registry.c` owns copied provider snapshots, tokens, revisions, pending actions
and leases. `src/server.c` exposes the named Exec port. `src/provider.c` is the
client helper. One registry feeds both views; providers do not receive drawing
objects. See the [module contract](module-contract.md) for protocol and ownership.

`src/dropdown.c` models selection and scrolling. `src/popup.c` renders public-screen,
overflow and action lists. Screen names are copied under the public-screen-list
lock and reacquired before switching. Provider menus retain tokens and close if
their provider disappears. Registered screens that are temporarily private can
remain listed; a failed lock is reported visibly.

The chooser supports hover, same-row press/release selection, keyboard navigation,
scroll indicators and expose redraw. Screen lists are bounded to 32 entries;
oversized names are skipped. Mouse-wheel scrolling is not implemented.

## Standalone attachment

No Intuition or Decoration patch is installed. Views attach to Workbench and an
optional owned demo screen. Other public screens can be selected; private app
screens do not receive views. An insufficient strip hides the overlay rather than
covering the title or native gadget. Preferences and autostart are not installed.

The default title exclusion is a centered, font-measured envelope with 12 pixels
of padding. It preserves the existing title position; it does not recenter system
text or inspect private theme layout. `--title-right X` explicitly sets the right
edge of the reserved title region for an off-center theme. The native integration
track needs decoration-supplied geometry. Other themes and modes need independent
runtime checks.

The event loop polls every five ticks. These are ordinary borderless windows;
clicking them can change focus. Outside-click dismissal on focus loss also passes
the click to the underlying window. Native menu handling and screen lifetime
integration remain platform work.

Close external windows on ScreenBarDemo before host exit. Visitors can prevent
that owned screen from closing; failure is logged and returns nonzero.
