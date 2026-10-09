# Module contract

Status: experimental design, revision 0.2. This document distinguishes the
implemented built-in interface from the target extension contract. It is not a
stable external SDK or a guarantee of binary compatibility.

## Design rule

A module supplies state and actions. ScreenBar owns presentation, placement,
interaction and screen views. Persistent system modules and indicators registered
by running applications use the same semantic contract.

## Experimental executable-provider SDK

[`include/screenbar.h`](../include/screenbar.h) and
[`screenbar-protocol.h`](../include/screenbar-protocol.h) define protocol version 1.
It is an experimental source interface: build the provider and host for the same
CPU and AROS ABI. Binary layouts and the 0.x SDK may change.

A host accepts up to eight external instances. It owns all presentation. The first
provider vocabulary is a generic icon, a label, optional text and one optional
action. The provider supplies its action label; an empty label means no action.
The host draws its dropdown and delivers `SB_ACTION_PRIMARY` on selection.

```c
struct SbClient client;
if (!sb_client_open(&client)) return 1;
int status = sb_client_register(&client, "My app", "Ready", "Refresh");
/* While registered, update regularly and handle the returned action. */
unsigned action = 0;
if (status == SB_OK)
    status = sb_client_update(&client, "My app", "Ready", &action);
/* Perform application work outside ScreenBar's renderer. */
sb_client_close(&client);
```

This abbreviated example shows API calls; the
[Counter example](../examples/counter.c) supplies the loop, discovery/re-registration,
action handling and orderly shutdown needed by a real provider.

Labels and action labels hold at most 31 printable ASCII bytes; text holds 63.
Every string must be NUL-terminated. Control characters, non-ASCII bytes and
oversized strings are rejected. The SDK copies caller text into each request;
the host copies the request into its own registry before replying.

`sb_client_open` creates a task-owned reply port. `register` creates an instance;
`update` advances its revision and receives a pending action; `unregister` removes
it. `close` unregisters and deletes the reply port. Calls on a client are serial
and must remain on the owning task; do not share it between tasks or reinitialize
an open client. C++ callers use the C linkage declared by the header. Compile the
helper as C; C++ objects and exceptions do not cross the interface.

### Transport and ownership

The named Exec port is `AROS.ScreenBar.1`. Discovery and sending run under
Forbid/Permit; allocation and waiting do not. Clients send one request at a time
and wait for ReplyMsg. The host never sends unsolicited messages and retains no
client message, reply-port or task pointer after replying. Actions are coalesced:
repeated selections before the next update produce one primary action, rather
than an unbounded queue. They are delivered once, not retried automatically.

**A request and its reply port must remain alive until the reply arrives.**
No timeout permits freeing an in-flight request. Normal host shutdown removes its
public port first, drains queued requests with `SB_STOPPED`, then releases it.
A provider must finish an outstanding call before unregistering or exiting.

The current transport targets trusted, cooperative local providers using the
matched SDK. It is not a security boundary or an SMP safety guarantee. Forced
host termination while a request is in flight can leave the caller waiting
indefinitely. Forced producer termination can invalidate an in-flight request.
Neither case is covered by this SDK's cooperative shutdown guarantee. Leases
remove stale UI state; they do not reclaim messages or provide crash containment.

### Discovery and lifecycle

Publish at least once per second; the host removes an item after five seconds
without a valid update, using its monotonic EClock time. Each host has an EClock
session identity, each registration a nonreused token, and each update an
increasing revision. Session/token mismatches and removed items return `SB_STALE`;
old revisions return `SB_REVISION`. Expiry closes the item's dropdown.

If the host is absent, calls return `SB_ABSENT`. After absence, `SB_STALE` or
`SB_STOPPED`, a live application should retry registration at a modest interval.
A host restart creates a new session. `SB_FULL` means capacity is reached;
`SB_VERSION` and `SB_INVALID` indicate incompatible requests or invalid data.
`SB_OK` is zero. A successful unregister clears the client's identity.

System built-ins retain the IDs in [`src/module.h`](../src/module.h); the host now
uses a dynamic view list instead of three fixed layout slots. Typed settings,
custom icons, progress, multiple actions and Lua are future additions to the
semantic contract below.

## Target semantic contract

The requirements below guide the next interfaces; field encodings and function
names are intentionally not frozen.

| Part | Module provides | Host responsibility |
|---|---|---|
| Identity | Stable module ID, display name, module version, required contract version | Reject incompatible required versions |
| Instance | Instance ID and producer-session identity | Separate concurrent producers and discard stale updates |
| Presentation | Icon content and kind, bounded text, tooltip, optional progress, preferred width | Shared placement, hit regions, text style, clipping and overflow |
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

### Icons, animation and widget width

The contract must support three icon kinds:

- **Monochrome:** a transparent one-bit image whose visible pixels use a
  screen-bar detail pen, so it follows the active screen theme.
- **Color:** a transparent full-color image with alpha, retaining its authored
  colors when drawn on the screen bar.
- **Animated:** a bounded sequence of monochrome or color frames. Each frame has
  a duration; the sequence declares whether it loops or stops on its last frame.

The host advances animation frames and schedules redraws. Providers do not run
an update loop just to animate an icon, and they do not draw directly into an
Intuition RastPort. The host owns the validated image data for as long as it is
displayed; providers may release their source buffers after the update is
accepted. The wire encoding, supported source file formats and resource limits
must be specified by the SDK before its first stable release.

Each widget view may declare its preferred width in pixels for the target
screen mode. That width determines its layout allocation and hit region. When
the available strip cannot fit the declared width, the host moves the item to
overflow rather than silently squeezing or clipping it. If a widget omits a
width, the host derives one from its content. Icon height is fitted to the
screen-bar content area while preserving aspect ratio.

These rules keep layout and input consistent while allowing each widget to
choose its own visual asset, animation and width. The first prototype should
validate monochrome recoloring, alpha compositing, frame timing, cleanup on
unregister and overflow for a wide widget before freezing the wire format.

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

The initial limits and cooperative shutdown are defined above. A future asynchronous
transport needs separate cancellation and completion rules.

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
new session. The experimental SDK expires UI state after five seconds without updates;
that is not proof that a process has died. Preferences stores
show/hide and placement policy, not stale running registrations.

## Languages and delivery

The core and first built-ins use C. The recommended external form is an ordinary
AROS executable using a small C-facing SDK. C++ is welcome behind that boundary;
STL objects, exceptions and C++ class layouts must not cross it. Build separate
binaries for supported CPU/ABI combinations.

The executable-provider SDK above implements the first subset. A program becomes
an extension by implementing registration, state, actions and lifecycle; being
an executable alone is insufficient. Process separation on AROS is not a promise
of memory isolation or crash containment.

This contract revision does not define an installer, package format or default
module installation directory. Those are separate delivery decisions and are
not prerequisites for settling the widget interface.

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
