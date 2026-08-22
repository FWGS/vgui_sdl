# SPDX-License-Identifier: LGPL-3.0-or-later
# Open ListPanel and click a row to select it. Checks row selection highlight.
# (FreeVGUI currently renders no rows, so this is expected to diff.)
scenario() {
	open_icon 78 262
	click 100 140
	settle
}
