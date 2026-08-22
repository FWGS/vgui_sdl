# SPDX-License-Identifier: LGPL-3.0-or-later
# Open FocusNav and click the first field. Checks focus acquisition and the
# focus-change signal ("focus: first").
scenario() {
	open_icon 128 262
	click 130 128
	settle
}
