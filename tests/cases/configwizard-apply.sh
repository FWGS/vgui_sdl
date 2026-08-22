# SPDX-License-Identifier: LGPL-3.0-or-later
# Open ConfigWizard and click "Apply". Checks the Apply button's ActionSignal
# fires and the status Label updates ("Apply pressed").
scenario() {
	open_icon 128 82
	click 268 323
	settle
}
