# SPDX-License-Identifier: LGPL-3.0-or-later
# Open the Menu demo and pop the "Stock" menu (constructed too short on purpose).
# Checks the Menu-never-auto-grows quirk: the last items spill below its border.
scenario() {
	open_icon 78 202
	click 212 100
	settle
}
