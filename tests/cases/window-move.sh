# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Buttons and drag its title bar. Exercises frame dragging, Desktop repaint
# of the vacated area and moved-window redraw. Needs ignore-mouse (headless).
scenario() {
	open_icon 28 202
	drag 150 66 340 170
	settle
}
