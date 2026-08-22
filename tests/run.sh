#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-3.0-or-later
# A/B screenshot harness: run each test case against the proprietary vgui.so and
# FreeVGUI, then diff the two screenshots.
#
#   ./run.sh                 # run every case in cases/
#   ./run.sh buttons         # run cases/buttons.sh only
#   ./run.sh desktop buttons # run a subset
#
# Output lands in results/:
#   results/proprietary/<case>.png   results/free/<case>.png
#   results/diff/<case>.png          (red = differing pixels)
#   results/montage/<case>.png       (proprietary | free | diff, side by side)
#   results/<backend>.log            (last run's stdout/stderr per backend)
#
# See README.md for the case format and knobs (FAKETIME_WHEN, EVENT_PORT, ...).

source "$( dirname "${BASH_SOURCE[0]}" )/lib.sh"

# collect requested cases (basenames) or default to all
declare -a CASES
if (( $# )); then
	for name in "$@"; do
		CASES+=( "$TESTS_DIR/cases/${name%.sh}.sh" )
	done
else
	for f in "$TESTS_DIR"/cases/*.sh; do
		CASES+=( "$f" )
	done
fi

mkdir -p "$RESULTS_DIR"/{proprietary,free,diff,montage,logs}

# rebuild + install by default -- 'waf install' depends on the build target, and
# waf skips anything unchanged (won't even touch installed files), so this is
# cheap when nothing moved and keeps out/ in sync with fast FreeVGUI iterations.
# VGUI_BUILD=0 to skip and run whatever is already in out/.
if [[ "${VGUI_BUILD:-1}" == 1 ]]; then
	log "building (waf install --destdir=$OUT_DIR) ..."
	if ! ( cd "$REPO_DIR" && ./waf install --destdir="$OUT_DIR" ) >"$RESULTS_DIR/logs/build.log" 2>&1; then
		log "error: build failed -- see $RESULTS_DIR/logs/build.log"
		tail -20 "$RESULTS_DIR/logs/build.log" >&2
		exit 1
	fi
fi

if [[ ! -x "$OUT_DIR/vgui_test" ]]; then
	log "error: $OUT_DIR/vgui_test not found -- run './waf install --destdir=out' first (or unset VGUI_BUILD=0)"
	exit 1
fi
if [[ ! -f "$FAKETIME_LIB" ]]; then
	log "error: libfaketime not found at $FAKETIME_LIB (set FAKETIME_LIB=...)"
	exit 1
fi
for backend in "${BACKENDS[@]}"; do
	if [[ ! -f "$OUT_DIR/$backend/vgui.so" ]]; then
		log "error: $OUT_DIR/$backend/vgui.so missing"
		exit 1
	fi
done

# parallelism: run this many cases at once, each on its own event port so the
# instances don't collide. VGUI_JOBS=1 for the old sequential behaviour.
JOBS="${VGUI_JOBS:-4}"
PORT_BASE="${VGUI_EVENT_PORT:-4938}"

# on exit/interrupt sweep any instances left behind (workers normally self-clean)
trap 'kill_stragglers' EXIT INT TERM

# run one case against one backend, leaving the screenshot at $shot.
# returns 0 on a captured screenshot, 1 if the backend died / never drew.
run_one() {
	local case_file="$1" backend="$2" shot="$3"
	CUR_CASE="$( basename "$case_file" .sh )"   # names the per-run log

	# per-case knobs, reset before sourcing so cases don't leak into each other
	WIDTH=640
	HEIGHT=480
	unset -f scenario 2>/dev/null || true
	scenario() { :; }   # default: bare desktop
	# shellcheck disable=SC1090
	source "$case_file"

	launch_backend "$backend" "$WIDTH" "$HEIGHT"
	if ! wait_ready; then
		warn "$backend/$(basename "$case_file"): event server never came up"
		stop_backend
		return 1
	fi

	scenario         # drive the UI (each event blocks until ingested+rendered)

	if ! backend_alive; then
		warn "$backend/$(basename "$case_file"): instance crashed during scenario"
		stop_backend
		return 1
	fi

	local ok=0
	shoot "$shot" || ok=1
	stop_backend
	return $ok
}

# run a whole case (both backends, diff, montage) on its own event port and
# write one result row to $rowfile. meant to run in a background subshell, so
# all state it touches (EVENT_PORT, VGUI_PID, scenario) is subshell-local.
run_case() {
	local idx="$1" case_file="$2"
	local rowfile="$RESULTS_DIR/logs/row-$( printf '%03d' "$idx" ).txt"

	EVENT_PORT=$(( PORT_BASE + idx ))       # unique port per case
	trap 'stop_backend' EXIT                # clean this worker's instance if killed

	local name; name="$( basename "$case_file" .sh )"

	if [[ ! -f "$case_file" ]]; then
		printf '%-18s %-12s %-12s %-12s %s\n' "$name" - - - NOFILE > "$rowfile"
		return
	fi

	local prop_shot="$RESULTS_DIR/proprietary/$name.png"
	local free_shot="$RESULTS_DIR/free/$name.png"
	local diff_shot="$RESULTS_DIR/diff/$name.png"
	local mont_shot="$RESULTS_DIR/montage/$name.png"

	local prop_ok=OK free_ok=OK
	run_one "$case_file" proprietary "$prop_shot" || prop_ok=FAIL
	run_one "$case_file" free        "$free_shot" || free_ok=FAIL

	# stale diff/montage from a previous run (e.g. a case that now MATCHes)
	rm -f "$diff_shot" "$mont_shot"

	local pixels="-" verdict="-" ae rc
	if [[ "$prop_ok" == OK && "$free_ok" == OK ]]; then
		# AE metric = count of differing pixels, computed without writing a diff
		# (null:). compare exits 1 when they differ and 2 on error (size mismatch)
		ae="$( compare -metric AE "$prop_shot" "$free_shot" null: 2>&1 )"
		rc=$?
		if (( rc >= 2 )); then
			pixels="?"; verdict="ERROR"
		elif [[ "${ae%% *}" == "0" ]]; then
			pixels="0"; verdict="MATCH"   # passed: no diff/montage needed
		else
			pixels="${ae%% *}"; verdict="DIFF"
			# only failures get a diff image + side-by-side montage
			compare -metric AE "$prop_shot" "$free_shot" "$diff_shot" >/dev/null 2>&1 || true
			magick montage \
				-label proprietary "$prop_shot" \
				-label free "$free_shot" \
				-label "diff ($pixels px)" "$diff_shot" \
				-tile 3x1 -geometry +6+6 -background '#202020' -fill '#dddddd' \
				"$mont_shot" 2>/dev/null || true
		fi
	else
		verdict="SKIP"
	fi

	printf '%-20s %-12s %-12s %-12s %s\n' "$name" "$prop_ok" "$free_ok" "$pixels" "$verdict" > "$rowfile"

	# live progress: count completed row files (approximate under races, but
	# fine for a running tally) and print this case's verdict as it lands
	local done; done="$( ls "$RESULTS_DIR"/logs/row-*.txt 2>/dev/null | wc -l )"
	local px=""; [[ "$pixels" != "-" ]] && px=" ($pixels)"
	log "$( printf '[%2d/%d] %-18s %s%s' "$done" "${#CASES[@]}" "$name" "$verdict" "$px" )"
}

rm -f "$RESULTS_DIR"/logs/row-*.txt
kill_stragglers   # clear leftovers before we start claiming ports

log "running ${#CASES[@]} cases, $JOBS at a time (event ports $PORT_BASE-$(( PORT_BASE + ${#CASES[@]} - 1 )))..."

idx=0
for case_file in "${CASES[@]}"; do
	run_case "$idx" "$case_file" &
	(( idx++ )) || true
	# throttle to JOBS concurrent workers
	while (( $(jobs -rp | wc -l) >= JOBS )); do wait -n; done
done
wait

printf '%-20s %-12s %-12s %-12s %s\n' CASE PROPRIETARY FREE PIXELS VERDICT
printf '%s\n' "----------------------------------------------------------------------------"
cat "$RESULTS_DIR"/logs/row-*.txt 2>/dev/null

# fail if any row's verdict is not MATCH
fail=0
grep -qvw MATCH "$RESULTS_DIR"/logs/row-*.txt 2>/dev/null && fail=1

printf '%s\n' "----------------------------------------------------------------------------"
if (( fail )); then
	log "done with differences/failures -- see $RESULTS_DIR/montage/"
else
	log "all cases matched"
fi
exit $fail
