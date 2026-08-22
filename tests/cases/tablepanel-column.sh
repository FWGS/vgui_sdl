# SPDX-License-Identifier: LGPL-3.0-or-later
# Open TablePanel and change its selection properties: turn "Row select" and
# "Cell select" off (leaving only column selection on), then click a cell. Checks
# the property toggles take effect -- the whole column highlights light-blue
# (columnSelected branch) instead of a navy cell.
scenario() {
	open_icon 78 322
	click 95 297
	click 235 297
	settle
	click 115 156
	settle
}
