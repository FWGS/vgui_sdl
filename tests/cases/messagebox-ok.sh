# SPDX-License-Identifier: LGPL-3.0-or-later
# Open MessageBox and click its "Ok" button. Checks the button's ActionSignal
# dismisses the box (it disappears, back to bare desktop).
scenario() {
	open_icon 78 142
	click 232 147
	settle
}
