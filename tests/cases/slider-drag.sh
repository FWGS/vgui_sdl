# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Slider and drag the horizontal thumb to the right. Checks the thumb moves
# and the value Label updates. Needs ignore-mouse (headless).
scenario() {
	open_icon 28 322
	drag 72 100 220 100
	settle
}
