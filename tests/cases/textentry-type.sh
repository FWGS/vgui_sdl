# SPDX-License-Identifier: LGPL-3.0-or-later
# Open TextEntry, focus the first field and type into it. Checks focus, caret and
# character rendering. (injected keys are unshifted, so lowercase only)
scenario() {
	open_icon 28 262
	click 150 118
	settle 0.2
	type_text "hello vgui"
	settle
}
