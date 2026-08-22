# SPDX-License-Identifier: LGPL-3.0-or-later
# Open Text, focus the password-style hidden TextEntry and type into it. Checks
# setTextHidden masks the characters (asterisks) rather than showing them.
scenario() {
	open_icon 28 262
	click 150 174
	settle
	type_text "secret"
	settle
}
