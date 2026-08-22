# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Text, focus the multi-line EditPanel and type into it. Checks EditPanel
# focus, caret and character rendering (distinct from the single-line TextEntry).
scenario() {
	open_icon 28 262
	click 150 240
	settle
	type_text "edit panel"
	settle
}
