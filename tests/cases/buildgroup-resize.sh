# SPDX-License-Identifier: LGPL-3.0-or-later
# Open BuildGroup, enter build mode (Ctrl+B), select the "Drag me" button, then
# Shift+Down / Shift+Right to grow it (build mode maps Shift+arrows to resize).
# Exercises modifier+arrow in build mode. FreeVGUI has no build mode -> DIFF.
scenario() {
	open_icon 128 202
	chord b ctrl
	settle
	click 115 176
	settle
	chord Down shift
	chord Down shift
	chord Right shift
	chord Right shift
	settle
}
