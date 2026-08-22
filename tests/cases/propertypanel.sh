# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Property Panel and expand the full build-mode property tree a Button's
# createPropertyPanel() produced: Properties > { Panel, Label, Button }. Sub-
# folders are expanded bottom-up (each click target stays put as rows insert
# below it). Compares FreeVGUI's property inspector against proprietary --
# folder/leaf names and the Label folder's inline setText TextEntry editor row.
scenario() {
	open_icon 178 262
	click 107 135
	settle
	click 120 216
	settle
	click 115 189
	settle
	click 115 162
	settle
}
