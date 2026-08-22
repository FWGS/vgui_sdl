# SPDX-License-Identifier: LGPL-3.0-or-later
# Open TablePanel and click a cell. With cell/row/column selection all on, this
# checks the selection highlight (navy cell over grey row) TablePanel paints via
# getCellRenderer's cellSelected branch.
scenario() {
	open_icon 78 322
	click 115 156
	settle
}
