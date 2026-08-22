# SPDX-License-Identifier: LGPL-3.0-or-later
# Open BuildGroup, enter build mode (Ctrl+B), then drag the "Drag me" button to a
# new spot. Checks build mode's mouse drag repositions the widget. Needs
# ignore-mouse (headless). FreeVGUI has no build mode -> DIFF.
scenario() {
	open_icon 128 202
	chord b ctrl
	settle
	drag 115 176 210 176
	settle
}
