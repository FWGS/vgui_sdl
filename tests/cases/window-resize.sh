# SPDX-License-Identifier: LGPL-3.0-or-later
# Open ScrollPanel and drag its bottom-right resize grip to enlarge the frame.
# Exercises the frame draggers + relayout. Needs ignore-mouse (headless).
scenario() {
	open_icon 28 82
	drag 273 213 360 300
	settle
}
