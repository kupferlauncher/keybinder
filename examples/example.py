#!/usr/bin/env python
"""
example.py

Created in 2010 by Ulrik Sverdrup <ulrik.sverdrup@gmail.com>
This work is placed in the public domain.
"""
import gtk
import keybinder

def callback(user_data):
	print "Handling", user_data
	gtk.main_quit()

if __name__ == '__main__':
	keystr = "<Ctrl>A"
	keybinder.bind(keystr, callback, "Keystring %s (user data)" % keystr)
	print "Press", keystr, "to handle keybinding and quit"
	gtk.main()
