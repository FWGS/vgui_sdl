# SPDX-License-Identifier: LGPL-3.0-or-later
# Open BuildGroup, enter build mode (Ctrl+B), select the "Drag me" button, then
# nudge it right with the arrow key several times. Checks build mode's keyboard
# nudge moves the selected widget. (FreeVGUI has no build mode -> DIFF.)
scenario() {
	open_icon 128 202
	chord b ctrl
	settle
	click 115 176
	settle
	key Right
	key Right
	key Right
	key Right
	key Right
	settle
}
