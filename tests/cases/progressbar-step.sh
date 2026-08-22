# SPDX-License-Identifier: LGPL-3.0-or-later
# Open ProgressBar and click "+" three times. Each ActionSignal steps the bar's
# progress up by one segment (starts at 3), so the fill grows.
scenario() {
	open_icon 78 22
	click 135 135
	click 135 135
	click 135 135
	settle
}
