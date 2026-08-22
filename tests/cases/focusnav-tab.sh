# SPDX-License-Identifier: LGPL-3.0-or-later
# Open FocusNav, focus the first field, then press Tab to move focus to the next
# field ("focus: second"). Checks FocusNavGroup keyboard traversal.
scenario() {
	open_icon 128 262
	click 130 128
	settle 0.2
	send '{"type":"key","key":"Tab","down":true}'
	send '{"type":"key","key":"Tab","down":false}'
	settle
}
