# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Buttons then click its close (X) caption button. The window and its
# taskbar button should both disappear.
scenario() {
	open_icon 28 202
	click 275 66
	settle
}
