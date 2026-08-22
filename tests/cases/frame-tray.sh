# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Window Controls and click the tray caption button (".", leftmost of the
# right-hand group), wired to spawn a MessageBox ("tray button pressed!").
scenario() {
	open_icon 128 322
	click 275 66
	settle
}
