# SPDX-License-Identifier: LGPL-3.0-or-later
# Open ListPanel, move over the list and scroll the wheel down. Bundled widgets
# ignore the wheel in stock vgui.so, so the test drives the ScrollBar via the
# demo's own InputSignal; checks the list scrolls (later items come into view).
scenario() {
	open_icon 78 262
	motion 120 100
	settle
	wheel -3
	settle
}
