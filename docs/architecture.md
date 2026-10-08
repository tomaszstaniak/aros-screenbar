# Architecture

`src/module.c` supplies shared built-in state and action mapping. The same state
feeds both screen views. `src/screenbar.c` owns the overlay windows and renders
icons/text using host-controlled geometry, screen fonts and bar pens.

`src/dropdown.c` is a platform-neutral selection/scrolling model. `src/popup.c`
owns the Intuition dropdown, public-screen snapshots, input and refresh handling.
Names are copied under the public-screen-list lock and reacquired before switching.
There are no borrowed foreign screen pointers in the dropdown model. Temporarily
private registered screens can remain listed; a failed lock is reported visibly.

The chooser uses a borderless window with hover highlighting, press/release
selection, keyboard navigation and scroll indicators. It redraws after expose.
Oversized public-screen names are skipped; the list is bounded to 32 entries.
Mouse-wheel scrolling and unrestricted lists are not implemented.

## Attachment limits

No Intuition or Decoration patch is installed. The standalone overlay attaches to
Workbench and an optional owned demo screen. It does not attach to private app
screens. A narrow screen that cannot fit the bar is rejected instead of covering
the system depth gadget. Other font/theme/display combinations need verification.

AROS toolbox input routing is disabled, so the prototype uses ordinary borderless
windows. They can change focus and may be covered by system menus. Dismissal on
focus loss does not consume an outside click. Safe modal input and native screen
lifetime integration require further platform work.

The clock is cached per view and the event loop polls every five ticks. The module
interface is internal and experimental; no external SDK or plugin loader exists.
Application-provided indicators and preferences are future capabilities.

Close external windows using ScreenBarDemo before exit. Visitors can prevent the
owned demo screen from closing; ScreenBar reports that condition and returns
nonzero rather than reporting clean teardown.
