# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Slider and drag the vertical thumb down. Checks the vertical Slider tracks
# and its value Label updates (the horizontal one is covered by slider-drag).
# Needs ignore-mouse (headless).
scenario() {
	open_icon 28 322
	drag 72 155 72 205
	settle
}
