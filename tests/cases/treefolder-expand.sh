# SPDX-License-Identifier: LGPL-3.0-or-later
# Open TreeFolder and click the "Audio" caption label to expand that node. The
# toggle handler lives on the label, not the +/- box (which is pure decoration),
# so this reveals Audio's Volume/Devices leaves.
scenario() {
	open_icon 178 142
	click 115 212
	settle
}
