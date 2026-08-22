# SPDX-License-Identifier: LGPL-3.0-or-later
# Open FocusNav, click the first field, then press Shift+Tab. VGUI's stock
# auto-nav maps Shift+Tab to requestFocusPrev, wrapping backwards to the last
# field ("focus: fifth"). The modifier is injected as a real Left Shift key so
# VGUI's own nav logic sees it held (no app-glue involved). Reverse of
# focusnav-tab.
scenario() {
	open_icon 128 262
	click 130 128
	settle
	chord Tab shift
	settle
}
