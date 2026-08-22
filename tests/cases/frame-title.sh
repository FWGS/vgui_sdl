# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Window Controls, click the Title field, append " Edited" and click "Set".
# Checks the title TextEntry + SetTitleSignal -> the frame caption repaints with
# the new title.
scenario() {
	open_icon 128 322
	click 150 118
	settle
	type_text " Edited"
	click 265 118
	settle
}
