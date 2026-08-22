# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Text, focus the multi-line EditPanel, type a line, press Enter, type more.
# Checks EditPanel accepts Enter as a newline and renders two lines.
scenario() {
	open_icon 28 262
	click 150 240
	settle
	type_text "line one"
	key Return
	type_text "line two"
	settle
}
