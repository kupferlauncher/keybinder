"""
This is `keybinder` a python module for gtk-based applications
registering global key bindings.

This module originates in [Tomboy][tb] as has been widely reused without
having a separate release. This package has taken the python bindings
for Tomboy's keybinder from the [Deskbar Applet][da] project, and broken
it out to be a standalone module.

The module is licenced under the GNU General Public License v2 (or at
your option, any later version).

[tb]: http://projects.gnome.org/tomboy/
[da]: http://projects.gnome.org/deskbar-applet/

Homepage: http://kaizer.se/wiki/python-keybinder/
"""
# version will change when it's sensible, for now just hardcode
__version__ = "0"

# gtk has to be setup
# this is to avoid warning spew if you plainly
# import keybinder; normally applications always use gtk themselves
import gtk
from _keybinder import *
