# SPDX-License-Identifier: LGPL-3.0-or-later
# Open BuildGroup, toggle build mode with Ctrl+B, then click the "Drag me"
# button. The chord's mod field fires the app-glue hotkey (sdl_app.cpp) so VGUI's
# BuildGroup engages; a click in build mode SELECTS the widget (dashed handles)
# instead of firing it. (FreeVGUI doesn't implement build mode -> DIFF.)
scenario() {
	open_icon 128 202
	chord b ctrl
	settle
	click 115 176
	settle
}
