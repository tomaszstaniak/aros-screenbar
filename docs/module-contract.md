# Module contract

Status: experimental design, revision 0.1. This document distinguishes the
implemented built-in interface from the target extension contract. It is not a
stable external SDK or a guarantee of binary compatibility.

## Design rule

A module supplies state and actions. ScreenBar owns presentation, placement,
interaction and screen views. Persistent system modules and indicators registered
by running applications use the same semantic contract.

## What exists today

The standalone executable has three built-ins: Screens, Audio and Clock.
[`src/module.h`](../src/module.h) is the authoritative source-level interface:

```c
enum SbModuleId { SB_MODULE_SCREENS, SB_MODULE_AUDIO, SB_MODULE_CLOCK };
enum SbAction { SB_ACTION_NONE, SB_ACTION_SHOW_SCREENS, SB_ACTION_OPEN_AHI };
enum SbIcon { SB_ICON_NONE, SB_ICON_SCREENS, SB_ICON_AUDIO };
struct SbModuleState {
    enum SbModuleId id;
    enum SbIcon icon;
    int available;
    char text[32];
};
void sb_modules_update(struct SbModuleState states[3], const char *clock_text);
enum SbAction sb_module_action(enum SbModuleId id);
```

`sb_modules_update` initializes the three states; text is copied and terminated,
with at most 31 bytes of content. `sb_module_action` maps Screens to SHOW_SCREENS,
Audio to OPEN_AHI, and Clock or unknown IDs to NONE. There are no callbacks,
registration functions, external-provider transport or plugin loader yet.

The host currently assumes three slots and known icon types. `available` is
reserved state vocabulary; disabled/unavailable rendering is not implemented.
A fourth built-in still requires changing the slot/layout assumptions. Do not
mistake this small implementation for the complete contract below.

To contribute a built-in now, change module state/action mapping, integrate its
layout/action in the host, add behavior tests and build against a matching AROS
SDK. This is ordinary source contribution, not installation of an external module.
See [build instructions](development.md).

## Target semantic contract

The requirements below guide the next interfaces; field encodings and function
names are intentionally not frozen.

| Part | Module provides | Host responsibility |
|---|---|---|
| Identity | Stable module ID, display name, module version, required contract version | Reject incompatible required versions |
| Instance | Instance ID and producer-session identity | Separate concurrent producers and discard stale updates |
| Presentation | Icon identifier, bounded text, tooltip, optional progress | Font, colors, icon metrics, spacing, clipping and overflow |
| State | Ready/unavailable/error, current values, optional diagnostic, revision | Replace snapshots atomically and expose truthful status |
| Actions | Stable action IDs, labels and enabled states | Mouse/keyboard handling and shared dropdown presentation |
| Settings | Typed keys, defaults, constraints | Validate and apply through common preferences |
| Lifecycle | Register/start, configure, publish, unregister/stop acknowledgement | Create/destroy views and cancel their pending work |

One producer can supply views on several screens. Opening another screen must
not create another data collector by default. A view is temporary; persistent
settings belong to module/instance identities, not Screen pointers.

The initial presentation vocabulary should remain small: icon, text, icon+text
and bounded progress. Arbitrary drawing callbacks and custom preference code
are outside the first external contract.

## Ownership and execution

- The host copies published strings and menu data. It never stores borrowed
  provider-owned buffers after an update finishes.
- Providers receive opaque, validated context where needed, not raw Screen,
  Window or RastPort pointers.
- Data acquisition and application actions run outside drawing. A slow provider
  must not block screen rendering or dropdown input.
- No provider callback, script or blocking IPC runs under Intuition render locks.
- Updates include instance/session identity and revision. Late updates from a
  removed or restarted producer are discarded.
- Every attachment has a detach path. Removing a view closes its popup and
  releases view resources before screen destruction.
- Requests, responses and shared buffers need explicit ownership and completion
  rules. A timeout alone never authorizes freeing an in-flight message.

Exact size limits, supported text encoding, asynchronous transport, cancellation
and shutdown handshake must be specified and tested before an external SDK ships.

## Permanent and application-driven modules

Permanent modules are enabled in preferences and start with ScreenBar.
Application items register while their producer runs and unregister on exit.
A background application can retain its item after its main window closes.

Example lifecycle:

```text
player starts -> registers item -> publishes playback state
user selects Pause -> host sends action -> player publishes new state
player quits -> unregisters -> host removes views and popups
```

A host restart requires live applications to rediscover and re-register with a
new session. Unexpected producer exit or stall needs a defined liveness policy;
a quiet indicator is not proof that its process has died. Preferences stores
show/hide and placement policy, not stale running registrations.

## Languages and delivery

The core and first built-ins use C. The recommended external form is an ordinary
AROS executable using a small C-facing SDK. C++ is welcome behind that boundary;
STL objects, exceptions and C++ class layouts must not cross it. Build separate
binaries for supported CPU/ABI combinations.

This executable-provider SDK is planned, not available today. A program becomes
an extension by implementing registration, state, actions and lifecycle; being
an executable alone is insufficient. Process separation on AROS is not a promise
of memory isolation or crash containment.

Lua may later be supported by a runner that translates script state/actions into
this same contract. It should not introduce another rendering API. Interpreter
selection, available bindings, execution budgets and blocking-call policy remain
open. Lua is not required for the current build.

## Compatibility policy and SDK readiness

Revision 0.x may change. A future SDK must publish its contract version, supported
platforms, header/helper library, example provider, packaging instructions and
migration notes. Optional capabilities should be negotiated; unsupported required
capabilities must fail explicitly.

Before external support, verify registration/update/action/unregister, two
instances, stale responses, screen closure, producer failure, host restart and
settings validation. Each ABI is independently verified. Neither MorphOS plugin
compatibility nor a stable binary ABI is promised.
