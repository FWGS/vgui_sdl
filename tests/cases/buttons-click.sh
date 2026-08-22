# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Buttons, click "Click me" three times. Checks the action signal fires and
# the counter Label repaints ("Clicked 3 times").
scenario() {
	open_icon 28 202
	click 90 118
	click 90 118
	click 90 118
	settle
}
