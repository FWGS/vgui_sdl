# SPDX-License-Identifier: LGPL-3.0-or-later
# Open the standalone ScrollBar demo (col 4, row 2) and click each bar's arrow
# button a few times so the value labels move off 0. (Thumb-dragging a standalone
# ScrollBar misreports its value -- it is built to be embedded in a ScrollPanel --
# so the arrows are the reliable way to drive it.)
scenario() {
	open_icon 178 82
	for i in 1 2 3 4 5; do click 272 127; done   # horizontal right arrow
	for i in 1 2 3 4 5; do click 68 255; done     # vertical down arrow
}
