# SPDX-License-Identifier: LGPL-3.0-or-later
# Open TabPanel, switch to "Buttons", then back to "Labels". Checks a second tab
# switch restores the first tab's content (StackLayout of Labels).
scenario() {
	open_icon 28 142
	click 150 97
	settle
	click 95 97
	settle
}
