# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Property Panel and expand just the "Properties" root, revealing the one
# sub-folder per hierarchy class (Panel/Label/Button) still collapsed. Checks the
# top-level structure of createPropertyPanel() independent of the deeper rows.
scenario() {
	open_icon 178 262
	click 107 135
	settle
}
