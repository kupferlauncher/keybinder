#!/usr/bin/env python3
"""
example-gi-py3.py

Looked at a pull request that was built for py2.x, but
overwrote the original py instead of making a separate example.
I wouldn't have accepted that pull request either.

The keybinder.init() part wasn't in the original example.

aking1012.com@gmail.com

public domain
"""

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Keybinder', '3.0')

from gi.repository import Gtk
from gi.repository import Keybinder

def callback(keystr, user_data):
    print ("Handling", user_data)
    print ("Event time:", Keybinder.get_current_event_time())
    Gtk.main_quit()

if __name__ == '__main__':
    keystr = "<Ctrl><Alt>M"
    Keybinder.init()
    Keybinder.bind(keystr, callback, "keystring %s (user data)" % keystr)
    print ("Press", keystr, "to handle keybinding and quit")
    Gtk.main()