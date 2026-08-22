# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Window Controls, maximize it, then click the (now top-right) maximize
# button again. The test wires maximize as a fill/restore toggle, so the frame
# should return to its original size and position.
scenario() {
	open_icon 128 322
	click 317 66
	settle
	click 607 17
	settle
}
