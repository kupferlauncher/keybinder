#!/usr/bin/python
"""
example_gi.py

Created in 2013 by Martin Kolman <martin.kolman@gmail.com>
This work is placed in the public domain.
"""
from gi.repository import Gtk
from gi.repository import Keybinder


def callback(key_string, user_data):
    """The callback gets the keystring pressed and any user provided data."""
    print "Pressed:", key_string
    print "Handling", user_data
    Gtk.main_quit()

if __name__ == '__main__':
    keystr = "<Ctrl>A"
    # the GI provided Keybinder needs
    # to be initialized before use
    Keybinder.init()
    Keybinder.bind(keystr, callback, "Keystring %s (user data)" % keystr)
    print "Press", keystr, "to handle keybinding and quit"
    Gtk.main()
