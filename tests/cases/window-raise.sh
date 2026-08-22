# SPDX-License-Identifier: LGPL-3.0-or-later
# Open two overlapping windows, then click the partially-hidden first one's title
# to raise it above the second.
scenario() {
	open_icon 28 202
	open_icon 28 82
	click 90 66
	settle
}
