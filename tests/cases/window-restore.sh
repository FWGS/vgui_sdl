# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Window Controls, minimize it (frame hides, taskbar button remains), then
# click its taskbar button to restore. Checks stock TaskBar restores/raises a
# minimized frame on its button click.
scenario() {
	open_icon 128 322
	click 297 66
	settle
	click 140 460
	settle
}
