# Testbed change requests from the A/B harness

Things the harness would benefit from but that require touching testbed / build
source. Not implemented -- for discussion (per the rule: save a note, decide
later). Already granted & done: the `file` field on the `screenshot` event.

## 0. Injected cursor position for drags & sliders -- DONE

Drags used to be impossible: VGUI polls the live cursor via `App::getCursorPos`
-> `Surface::GetMousePos`, which returned `SDL_GetGlobalMouseState` (the real
pointer, unmoved by injected motion). Fixed by an **ignore-mouse** mode:
`GetMousePos` returns the last injected cursor position instead. It is on
automatically in headless (a hidden window can't get the real pointer anyway),
and a non-headless script toggles it at runtime via the event server:
`{"type":"ignoremouse","enabled":true}` -> drag events -> `{... false}`. The
harness `drag` helper does this bracketing for you. `window-move`,
`window-resize` and `slider-drag` scenarios now work (Slider drag verified: thumb
moves, value reaches 46).

## 1. Open an app by name, not by icon pixel coordinates

Cases currently double-click desktop icons at hardcoded grid coordinates (see
README). That's brittle: it breaks if the icon order, grid, or window size
changes, and it only reaches apps that have a desktop icon. An event-server
command like `{"type":"open","app":"ScrollPanel"}` that looked the MiniApp up by
`getName()` and opened it directly would make cases robust and readable. This is
the single biggest fragility in the suite.

## 2. An "idle/settled" signal instead of fixed sleeps

The harness sprinkles `settle` (fixed `sleep`s) to wait for layout/animation to
finish before screenshotting. A way to ask the app "are you idle?" (e.g. an
event that responds once no repaint is pending, or a frame counter readable over
HTTP) would remove the guesswork and speed the suite up.

## 3. Deterministic / query-able window placement

New frames are positioned by the Desktop (cascade). If proprietary and FreeVGUI
ever place windows differently, every windowed case diffs wholesale rather than
showing the real widget difference. Not a problem today (placement matches), but
if it drifts we'd want either a way to force a frame to a fixed position for
tests, or to read back a frame's rect to normalize.

## 4. Offscreen / headless rendering -- DONE

Implemented as the testbed `-headless` switch (creates the window with
`SDL_WINDOW_HIDDEN`): rendering, screenshots and injected input all still work,
but no window is shown and focus is never stolen. The harness passes it by
default (set `VGUI_HEADLESS=0` to watch). For a machine with no display at all
(CI), also export `SDL_VIDEODRIVER=offscreen` -- verified working.

## 5. Screenshot a specific frame/panel, not the whole screen

Whole-screen diffs mix desktop, taskbar, clock and the window under test. Being
able to capture just a named panel's bounds would give tighter, more meaningful
diffs (and would sidestep #3). Lower priority.

## 6. Modifier state on injected key events -- DONE

The key event now takes an optional `mod` field: a string (`"ctrl"`,
`"shift alt"`) or an array (`["ctrl","shift"]`). `sdl_eventserver.cpp`
`parse_keymod` scans it and stamps `ev.key.mod`, so the app-glue hotkeys that
read it (`Ctrl+B` build mode, `Ctrl+R`/`Ctrl+E` overlays) fire under scripted
input.

Key subtlety (checked: nothing reads live SDL state -- no `SDL_GetModState` /
`SDL_GetKeyboardState` anywhere, and VGUI never sees SDL events at all). There are
two independent consumers of "is a modifier held":

- **Our app-glue hotkeys** read the *event's* `ev.key.mod` -> the `mod` field
  covers them.
- **VGUI's own logic** (shifted `getKeyCodeChar`, `Shift+Tab` auto-nav, build-mode
  `Ctrl+C`) tracks modifiers from the `KEY_LSHIFT`/`KEY_LCONTROL` *key events* it
  receives, not from any flag -- so a script must also inject those modifier keys.

The `chord` helper does both: it presses the modifier key(s) as real events (for
VGUI) *and* sets the tap's `mod` field (for the glue) -- `chord b ctrl`,
`chord Tab shift`, `chord c "ctrl shift"`. `type_text` now brackets capitals with
`Left Shift` on its own, so uppercase just works. Verified live: `Ctrl+R` outlines
every panel; `Ctrl+B`+click selects a widget (build mode); `Shift+Tab` wraps focus
backwards to the last field; `type_text "Hello VGUI"` renders the capitals. New
cases: `buildgroup-mode`, `focusnav-shifttab`, `textentry-caps`.

Still open, lower priority: **clipboard contents**. BuildGroup's Paste button
reads `App::getClipboardText`; testing it deterministically needs a way to seed
the clipboard (an event, or an `SDL_SetClipboardText` at startup). `Ctrl+V` into a
text widget has the same dependency.
