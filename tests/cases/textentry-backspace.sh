# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Text, focus the first TextEntry, type "hello" then press Backspace twice.
# Checks Backspace deletes the last characters ("hel") and the caret follows.
scenario() {
	open_icon 28 262
	click 150 118
	settle
	type_text "hello"
	key Backspace
	key Backspace
	settle
}
