# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Window Controls and uncheck "Menu" under "Caption buttons visible". Checks
# setMenuButtonVisible(false) removes the top-left system (menu) caption button.
scenario() {
	open_icon 128 322
	click 65 365
	settle
}
