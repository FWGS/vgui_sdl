# SPDX-License-Identifier: LGPL-3.0-or-later
# Open BuildGroup, enter build mode (Ctrl+B), then RIGHT-drag on empty client
# area. Build mode creates a new Label spanning the drag rect. Probes a
# right-button drag through the event server. FreeVGUI has no build mode -> DIFF.
scenario() {
	open_icon 128 202
	chord b ctrl
	settle
	motion 120 250
	send '{"type":"button","button":"right","down":true}'
	motion 150 260
	motion 200 275
	send '{"type":"button","button":"right","down":false}'
	settle
}
