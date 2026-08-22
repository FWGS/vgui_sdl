# SPDX-License-Identifier: LGPL-3.0-or-later
# shared helpers for the VGUI A/B screenshot harness (sourced by run.sh).
#
# Runs vgui_test against two backends -- the proprietary vgui.so and FreeVGUI --
# under a frozen wall clock (libfaketime) and the software renderer, so that a
# given scenario produces byte-comparable screenshots. See README.md.

set -u

TESTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_DIR="$( cd "$TESTS_DIR/.." && pwd )"

# where 'waf install --destdir=out' put vgui_test, vgui.so, fonts, icons
OUT_DIR="${VGUI_OUT_DIR:-$REPO_DIR/out}"
RESULTS_DIR="${VGUI_RESULTS_DIR:-$TESTS_DIR/results}"

# 32-bit libfaketime; an *absolute* FAKETIME (no leading @) freezes the clock,
# so the taskbar time is identical between the two runs. Keep the monotonic
# clock real, else SDL event timing / double-click detection breaks.
FAKETIME_LIB="${FAKETIME_LIB:-/usr/lib/libfaketime.so.1}"
FAKETIME_WHEN="${FAKETIME_WHEN:-2020-01-01 13:37:42}"

# runs are sequential, so a single port is fine; override to avoid a clash with
# a manually-running instance
EVENT_PORT="${VGUI_EVENT_PORT:-4938}"

# headless by default: the window is created hidden so runs don't flash windows
# or steal focus. Set VGUI_HEADLESS=0 to watch a run. For a display-less machine
# (CI), also export SDL_VIDEODRIVER=offscreen -- it passes through to the child.
HEADLESS="${VGUI_HEADLESS:-1}"

# the two backends we compare, in report order
BACKENDS=( proprietary free )

VGUI_PID=""

log()  { printf '%s\n' "$*" >&2; }
warn() { printf 'WARN: %s\n' "$*" >&2; }

# POST one JSON event and block until the server says the work is done (the
# event was ingested + rendered, or the screenshot was written). max-time is
# generous: normal replies are milliseconds, but a stalled loop makes the server
# fall back to its internal timeout (~2s events / ~5s screenshots).
send() {
	curl -s --max-time 15 -d "$1" "http://127.0.0.1:$EVENT_PORT/" >/dev/null || true
}

# wait for the UI to settle. no longer a sleep: the "sync" event returns only
# after the loop renders a couple of frames. $1 is ignored (kept for callers).
settle() { send '{"type":"sync"}'; }

# block until an event round-trips *quickly* -- i.e. the main loop is live and
# rendering, not just the socket accepting. a short max-time is the test: during
# startup the server falls back to its ~2s timeout and this retries.
wait_ready() {
	local tries=0
	while (( tries < 100 )); do
		if curl -s --max-time 1 -d '{"type":"motion","x":-1,"y":-1}' \
			"http://127.0.0.1:$EVENT_PORT/" >/dev/null 2>&1; then
			return 0
		fi
		sleep 0.1
		(( tries++ )) || true
	done
	return 1
}

# wait for one pid to exit, escalating to SIGKILL. used instead of a global
# pkill so parallel workers (each with its own instance + port) don't kill each
# other -- a lingering instance would otherwise keep its port and get
# screenshotted instead, the cause of false diffs.
wait_gone() {
	local pid="$1" t=0
	while kill -0 "$pid" 2>/dev/null; do
		(( t == 30 )) && kill -9 "$pid" 2>/dev/null
		sleep 0.1
		(( t++ )) || true
		(( t >= 60 )) && break
	done
}

# one-time sweep of leftover instances from a previous run. safe only before any
# of this run's workers launch (it is global). the fatal-bind on a taken port is
# the backstop if a straggler survives.
kill_stragglers() {
	pkill -x vgui_test 2>/dev/null || true
	local t=0
	while pgrep -x vgui_test >/dev/null 2>&1 && (( t < 30 )); do
		sleep 0.1
		(( t++ )) || true
	done
}

# launch a backend. $1 backend name, $2 width, $3 height. sets VGUI_PID.
launch_backend() {
	local backend="$1" w="$2" h="$3"

	local headless=""
	[[ "$HEADLESS" == 1 ]] && headless="-headless"

	# exec so $! is vgui_test itself, not the subshell wrapper
	(
		cd "$OUT_DIR" || exit 127
		exec env \
			LD_PRELOAD="$FAKETIME_LIB" \
			FAKETIME="$FAKETIME_WHEN" \
			FAKETIME_DONT_FAKE_MONOTONIC=1 \
			VGUI_EVENT_PORT="$EVENT_PORT" \
			LD_LIBRARY_PATH="$OUT_DIR/$backend" \
			./vgui_test -software $headless -width "$w" -height "$h"
	) >"$RESULTS_DIR/logs/$backend-${CUR_CASE:-run}.log" 2>&1 &
	VGUI_PID=$!
	# drop it from the job table so a crash doesn't print an async
	# "Segmentation fault" line into the middle of the report
	disown "$VGUI_PID" 2>/dev/null || true
}

# is the launched instance still alive?
backend_alive() { [[ -n "$VGUI_PID" ]] && kill -0 "$VGUI_PID" 2>/dev/null; }

# stop the launched instance and wait until it is truly gone (by pid, so it is
# safe to run concurrently with other workers)
stop_backend() {
	[[ -n "$VGUI_PID" ]] || return 0
	kill "$VGUI_PID" 2>/dev/null || true
	wait_gone "$VGUI_PID"
	VGUI_PID=""
}

# --- convenience wrappers for use inside a case's scenario() -------------------

# click at x,y; optional 3rd arg = click count (2 = double-click)
click()    { send "{\"type\":\"click\",\"x\":$1,\"y\":$2,\"clicks\":${3:-1}}"; }
dblclick() { click "$1" "$2" 2; }
motion()   { send "{\"type\":\"motion\",\"x\":$1,\"y\":$2}"; }

# mouse wheel; $1 = delta (SDL sign: positive = scroll up). VGUI routes it to the
# panel under its tracked cursor, so move over the target with motion() first.
wheel() { send "{\"type\":\"wheel\",\"x\":0,\"y\":$1}"; }

# double-click a desktop icon to open its window. the double-click reply already
# waits for the window to be created and rendered. desktop icons sit on a
# column-major grid; at 640x480 the column x-centers are ~28/78/128 and the row
# y-centers ~22/82/142/202/262/322 (see README for the full map).
open_icon() { dblclick "$1" "$2"; }

# toggle whether GetMousePos reports the injected cursor. $1 = true|false.
ignore_mouse() { send "{\"type\":\"ignoremouse\",\"enabled\":$1}"; }

# press at x1,y1, move to x2,y2 in a few steps, release: a mouse drag. VGUI polls
# the real cursor while dragging, so ignore-mouse must be on for the injected
# motion to drive it. headless has it on already; otherwise bracket on/off.
drag() {
	local x1="$1" y1="$2" x2="$3" y2="$4" steps=6 i
	[[ "$HEADLESS" == 1 ]] || ignore_mouse true
	motion "$x1" "$y1"
	send '{"type":"button","down":true}'
	for (( i = 1; i <= steps; i++ )); do
		motion $(( x1 + ( x2 - x1 ) * i / steps )) $(( y1 + ( y2 - y1 ) * i / steps ))
	done
	send '{"type":"button","down":false}'
	[[ "$HEADLESS" == 1 ]] || ignore_mouse false
}

# tap a single named key (press + release), e.g. key Return / Backspace / Right.
# Use SDL scancode names for non-letters.
key() {
	send "{\"type\":\"key\",\"key\":\"$1\",\"down\":true}"
	send "{\"type\":\"key\",\"key\":\"$1\",\"down\":false}"
}

# a modifier chord: hold modifier key(s), tap $1, release. The modifiers are sent
# as real key events so VGUI sees them held (its own shortcut/traversal logic
# reads that), AND the tap carries a "mod" field so the app-glue hotkeys that read
# ev.key.mod (Ctrl+B build mode, Ctrl+R/E overlays) also fire. $2 is a
# space-separated modifier list: ctrl | shift | alt (e.g. chord b ctrl,
# chord Tab shift, chord c "ctrl shift").
declare -A _MOD_KEY=( [ctrl]="Left Ctrl" [shift]="Left Shift" [alt]="Left Alt" )
chord() {
	local key="$1" mods="${2:-}" m
	for m in $mods; do send "{\"type\":\"key\",\"key\":\"${_MOD_KEY[$m]}\",\"down\":true}"; done
	send "{\"type\":\"key\",\"key\":\"$key\",\"down\":true,\"mod\":\"$mods\"}"
	send "{\"type\":\"key\",\"key\":\"$key\",\"down\":false,\"mod\":\"$mods\"}"
	for m in $mods; do send "{\"type\":\"key\",\"key\":\"${_MOD_KEY[$m]}\",\"down\":false}"; done
}

# type a string via key events. Lowercase/digits/space go straight through;
# UPPERCASE letters are bracketed with Left Shift so VGUI produces the capital
# (it tracks shift from the key event, no mod field needed). Other shifted
# symbols aren't mapped -- use chord for those.
type_text() {
	local s="$1" i c
	for (( i = 0; i < ${#s}; i++ )); do
		c="${s:i:1}"
		if [[ "$c" == " " ]]; then
			send '{"type":"key","key":"Space","down":true}'
			send '{"type":"key","key":"Space","down":false}'
		elif [[ "$c" == [A-Z] ]]; then
			send '{"type":"key","key":"Left Shift","down":true}'
			send "{\"type\":\"key\",\"key\":\"${c,,}\",\"down\":true}"
			send "{\"type\":\"key\",\"key\":\"${c,,}\",\"down\":false}"
			send '{"type":"key","key":"Left Shift","down":false}'
		else
			send "{\"type\":\"key\",\"key\":\"$c\",\"down\":true}"
			send "{\"type\":\"key\",\"key\":\"$c\",\"down\":false}"
		fi
	done
}

# request a screenshot to an absolute path. $1 = path. the reply blocks until the
# file is written, so we only need to confirm it landed.
shoot() {
	local path="$1"
	rm -f "$path"
	send "{\"type\":\"screenshot\",\"file\":\"$path\"}"
	[[ -s "$path" ]]
}
