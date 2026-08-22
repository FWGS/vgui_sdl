# SPDX-License-Identifier: LGPL-3.0-or-later
# Open the Menu demo, pop the sized menu, then click its "Open" MenuItem. Checks
# the item's ActionSignal fires ("picked: Open") and the menu hides again.
scenario() {
	open_icon 78 202
	click 92 100
	settle
	click 120 155
	settle
}
