# SPDX-License-Identifier: LGPL-3.0-or-later
# Open TablePanel and drag the Name|Size header divider left to narrow the Name
# column; the table columns follow the header. Needs ignore-mouse (headless).
scenario() {
	open_icon 78 322
	drag 168 99 120 99
	settle
}
