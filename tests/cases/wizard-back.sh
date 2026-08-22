# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Wizard, go Next to the Options page, then click "<< Back". Checks reverse
# page traversal (Back button appears past page 1) and the switching-to-back
# signal ("switch: leaving Options back", page Welcome).
scenario() {
	open_icon 128 22
	click 180 265
	settle
	click 103 265
	settle
}
