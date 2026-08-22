# SPDX-License-Identifier: LGPL-3.0-or-later
# Open ConfigWizard and click its "Ok" button. Checks the button's ActionSignal
# fires and the status Label in the client area updates ("Ok pressed").
scenario() {
	open_icon 128 82
	click 133 323
	settle
}
