# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Text, focus the first TextEntry and type a mixed-case string. Checks
# uppercase rendering: type_text brackets capitals with Left Shift, which VGUI
# tracks to produce the shifted glyph.
scenario() {
	open_icon 28 262
	click 150 118
	settle
	type_text "Hello VGUI"
	settle
}
