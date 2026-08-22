# SPDX-License-Identifier: LGPL-3.0-or-later
# Open TablePanel and double-click a cell to start editing. Checks
# startCellEditing spawns an inline TextEntry over the cell.
scenario() {
	open_icon 78 322
	dblclick 115 156
	settle
}
