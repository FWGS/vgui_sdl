# SPDX-License-Identifier: LGPL-3.0-or-later
# Open BuildGroup, enter build mode (Ctrl+B), select a widget, generate layout
# code (Ctrl+C -> clipboard), leave build mode (Ctrl+B), then click "Paste" to
# dump the clipboard into the EditPanel. Exercises the build-mode code generator
# end-to-end via the app's own paste (so it needs no clipboard access from the
# harness -- and SDL's clipboard does round-trip even headless). Ctrl+C emits a
# <buildname>->setBounds(...) line per registered widget. FreeVGUI has no build
# mode -> DIFF/empty.
scenario() {
	open_icon 128 202
	chord b ctrl
	settle
	click 115 176
	settle
	chord c ctrl
	settle
	chord b ctrl
	settle
	click 93 415
	settle
}
