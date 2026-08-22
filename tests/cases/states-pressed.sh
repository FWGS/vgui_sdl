# SPDX-License-Identifier: LGPL-3.0-or-later
# Pressed state: press and hold (no release) on the enabled Button. The
# screenshot is taken while it is held down (needs ignore-mouse, on in headless).
scenario() {
	open_icon 178 202
	motion 172 121
	send '{"type":"button","down":true}'
}
