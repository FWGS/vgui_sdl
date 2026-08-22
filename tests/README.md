# A/B screenshot tests

Runs the `vgui_test` desktop against **both** VGUI backends -- the proprietary
`vgui.so` and **FreeVGUI** -- under identical conditions, screenshots each, and
diffs the pair. This is the behavior-comparison testbed described in the top
`CLAUDE.md`: the same scripted scenario is replayed against both libraries and
any pixel difference is flagged.

## Requirements

- The subprojects wired up so `./waf install --destdir=out` produces
  `out/vgui_test`, `out/proprietary/vgui.so` and `out/free/vgui.so`.
- 32-bit **libfaketime** at `/usr/lib/libfaketime.so.1` (overridable, see below).
- **ImageMagick 7** (`magick`, `compare`) on `PATH`.

`run.sh` runs `waf install` itself before each run (waf skips anything unchanged,
so it's cheap and keeps `out/` in sync with fast FreeVGUI iterations). Set
`VGUI_BUILD=0` to skip it and run whatever is already installed.

## Running

```sh
cd tests
./run.sh                 # every case in cases/
./run.sh scrollpanel     # one case
./run.sh desktop buttons # a subset
```

Output (all under `results/`, git-ignored):

| path | contents |
|------|----------|
| `results/proprietary/<case>.png` | screenshot from the proprietary lib |
| `results/free/<case>.png`        | screenshot from FreeVGUI |
| `results/diff/<case>.png`        | ImageMagick diff, red = differing pixels (**DIFF cases only**) |
| `results/montage/<case>.png`     | proprietary \| free \| diff, side by side (**DIFF cases only**) |
| `results/logs/<backend>-<case>.log` | that run's stdout/stderr (per case) |

The summary table reports, per case, whether each backend ran, the differing
pixel count (ImageMagick `AE`), and a verdict: `MATCH` (0 px), `DIFF`, `SKIP` (a
backend failed to run/draw), or `ERROR` (e.g. size mismatch). Exit status is
non-zero if anything didn't `MATCH`.

## How it stays deterministic

- **Software renderer** (`-software`) -- avoids GPU/driver pixel variance.
- **Headless** (`-headless`) -- the window is created hidden, so runs don't flash
  windows or steal focus; screenshots still work. On by default; `VGUI_HEADLESS=0`
  to watch a run. On a display-less machine also `export SDL_VIDEODRIVER=offscreen`.
- **Frozen wall clock** -- `libfaketime` is `LD_PRELOAD`ed with an *absolute*
  `FAKETIME` (no leading `@`), which stops the clock, so the taskbar time is
  identical between runs. `FAKETIME_DONT_FAKE_MONOTONIC=1` keeps SDL's monotonic
  timing real, so the event loop and double-click detection still work.
- **Scripted input** -- the HTTP event server (`sdl_eventserver.cpp`) replays
  the same clicks/motions for both backends. In headless mode a port that can't
  be bound is fatal, so a stale/duplicate instance can't be mistaken for the one
  under test.
- **Synchronous replies** -- an injected event's HTTP reply is withheld until the
  main loop has ingested it and rendered the result; a screenshot reply until the
  PNG is on disk. So the harness never sleeps -- it just waits on the reply. This
  is what makes it both fast (~9s for the full suite) and flake-free under
  parallelism, since there is no timing guess to lose under load.
- **Fixed screenshot names** -- the `screenshot` event takes a `file` field, so
  each run writes straight to its result path.

The clock being frozen means wall-clock timing between runs is irrelevant, so
cases can run in parallel (each on its own port) without affecting each other's
pixels. Within a case the two backends run one after another on that case's port.

## Writing a case

A case is `cases/<name>.sh` defining a `scenario()` function. It may set `WIDTH`
/ `HEIGHT` (default 640x480). Helpers from `lib.sh`:

- `click X Y [N]` -- click (N=2 double-clicks)
- `dblclick X Y`, `motion X Y`
- `open_icon X Y` -- double-click a desktop icon and wait for its window
- `type_text "..."` -- type via key events; lowercase/digits/space go straight
  through and UPPERCASE letters are bracketed with `Left Shift` (other shifted
  symbols aren't mapped -- use `chord`)
- `chord KEY [MODS]` -- a modifier chord: holds the modifier key(s), taps `KEY`,
  releases. `MODS` is a space-separated list of `ctrl`/`shift`/`alt` (e.g.
  `chord b ctrl`, `chord Tab shift`, `chord c "ctrl shift"`). It presses the
  modifiers as real key events (so VGUI's own logic sees them held) *and* sets the
  key event's `mod` field (so app-glue hotkeys like `Ctrl+B` fire). See below.
- `settle` -- wait for the UI to render a couple of frames (a `sync` round-trip)
- `send '<json>'` -- POST a raw event

- `drag X1 Y1 X2 Y2` -- press, move, release (frame moves/resizes, slider drags)

The `key` event takes an optional `mod` field -- a string (`"ctrl"`,
`"shift alt"`) or array (`["ctrl","shift"]`) -- which stamps SDL's `ev.key.mod`.
That drives the app-glue hotkeys (`Ctrl+B` build mode, `Ctrl+R`/`Ctrl+E`
overlays). VGUI's *own* modifier tracking (shifted characters, `Shift+Tab`
traversal, build-mode `Ctrl+C`) instead comes from the `Left Shift`/`Left Ctrl`
key events themselves, so `chord` injects both -- prefer it over a raw `mod`.

`drag` needs "ignore-mouse" mode, where the testbed reports the injected cursor
to VGUI instead of the real pointer (which injected motion can't move). Headless
turns this on automatically; otherwise the helper brackets the drag with
`{"type":"ignoremouse","enabled":true/false}` events. To do it by hand, send
those around your own motion/button events.

The runner launches the backend, waits for the event server, calls `scenario()`,
then screenshots -- do **not** take the screenshot yourself.

```sh
# cases/scrollpanel.sh
scenario() {
    open_icon 28 82
}
```

### Desktop icon coordinates (640x480)

Icons fill a **column-major** grid. Column x-centers ~`28 / 78 / 128 / 178`, row
y-centers ~`22 / 82 / 142 / 202 / 262 / 322`. Registration order (see
`sdl_viewport.cpp` `CreateViewport`) fills column 1 top-to-bottom, then column 2:

| # | app | x,y | | # | app | x,y | | # | app | x,y | | # | app | x,y |
|---|-----|-----|-|---|-----|-----|-|---|-----|-----|-|---|-----|-----|
| 1 | Label       | 28,22  | | 7  | ProgressBar  | 78,22  | | 13 | Wizard       | 128,22  | | 19 | HeaderPanel | 178,22  |
| 2 | ScrollPanel | 28,82  | | 8  | ImagePanel   | 78,82  | | 14 | ConfigWizard | 128,82  | | 20 | ScrollBar   | 178,82  |
| 3 | TabPanel    | 28,142 | | 9  | MessageBox   | 78,142 | | 15 | Border       | 128,142 | | 21 | TreeFolder  | 178,142 |
| 4 | Button      | 28,202 | | 10 | Menu         | 78,202 | | 16 | BuildGroup   | 128,202 | | 22 | States        | 178,202 |
| 5 | TextEntry   | 28,262 | | 11 | ListPanel    | 78,262 | | 17 | FocusNav     | 128,262 | | 23 | Property Panel | 178,262 |
| 6 | Slider      | 28,322 | | 12 | TablePanel   | 78,322 | | 18 | Frame        | 128,322 |

Not in stock VGUI1 (zero symbols in `vgui.so`, so untestable): **ComboBox**,
**PropertySheet**/**PropertyPage**, **MenuBar**, **Splitter** -- these are VGUI2
concepts. The **States** app is the widget state matrix (enabled/disabled columns
for Button/Toggle/Check/Radio); `states-hover`/`states-pressed`/`states-focused`
drive the enabled Button through the remaining states. (Note: stock Button has no
hover rollover, so `states-hover` == normal.)

The Start button is at ~`25,459`. Verify coordinates by eyeballing the
`results/*/` screenshots -- a wrong coordinate just opens the wrong (or no)
window, which the montage makes obvious.

## Knobs (environment)

| var | default | meaning |
|-----|---------|---------|
| `FAKETIME_LIB`  | `/usr/lib/libfaketime.so.1` | 32-bit libfaketime path |
| `FAKETIME_WHEN` | `2020-01-01 13:37:42`       | frozen wall-clock time |
| `VGUI_EVENT_PORT` | `4938` | event-server port |
| `VGUI_OUT_DIR`  | `../out` | install dir to run from |
| `VGUI_RESULTS_DIR` | `./results` | output dir |
| `VGUI_HEADLESS` | `1` | hidden window; `0` to watch runs |
| `VGUI_BUILD`    | `1` | `waf install` before running; `0` to skip |
| `VGUI_JOBS`     | `4` | cases to run in parallel; `1` = sequential |
| `VGUI_EVENT_PORT` | `4938` | base event port; case *i* uses base+*i* |

## Parallelism

Cases run `VGUI_JOBS` at a time, each on its own event-server port (`base + case
index`), so instances never collide. This is the main speedup. A port that can't
be bound is fatal in headless mode (see below), so an accidental collision fails
loudly instead of silently cross-talking to another instance.

Replies are synchronous (see "How it stays deterministic"), so there is no
per-case sleep to slip under load -- parallelism doesn't add flakes and you can
raise `VGUI_JOBS` freely. Two back-to-back full runs are byte-identical. If a case
ever flips between runs, re-run it alone (`./run.sh <case>`) to confirm.
