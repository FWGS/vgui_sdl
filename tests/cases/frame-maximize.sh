# SPDX-License-Identifier: LGPL-3.0-or-later
# Open the Window Controls (Frame) demo and click its maximize caption button,
# which the test wires to fill the desktop. Discrete click -> works via harness.
scenario() {
	open_icon 128 322
	click 317 66
	settle
}
