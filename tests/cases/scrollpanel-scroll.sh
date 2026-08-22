# SPDX-License-Identifier: LGPL-3.0-or-later
# Open ScrollPanel and drag its vertical scrollbar thumb down. Checks the client
# scrolls (later items scroll into view). Needs ignore-mouse (headless).
scenario() {
	open_icon 28 82
	drag 252 98 252 175
	settle
}
