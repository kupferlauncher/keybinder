libkeybinder
============

**keybinder** is a library for registering global keyboard shortcuts.
Keybinder works with GTK-based applications using the X Window System.

The library contains ``keybinder-3.0``

* A C library, ``libkeybinder``
* Gobject-Introspection (gir)  generated bindings

Compile and Installation Notes
------------------------------

The package ``keybinder`` compiles against GTK+ 2 while the package
``keybinder-3.0`` compiles against GTK+ 3. The two packages can be
installed in parallel.

NOTE: the old “``keybinder``” is in the branch ``keybinder-legacy`` on
github. ``keybinder-3.0`` is the active development.

Both packages support generating gobject-introspection (gir) bindings,
even if these bindings are more relevant for the future, hence the GTK+
3 package. For ``keybinder-3.0``, the gir bindings are strongly
  recommended over any other language bindings.

If you want to compile with reduced library linking, configure using::

    CC="cc -Wl,--as-needed" ./configure

Build Requirements
------------------

    * GTK+ 3.0 (``keybinder-3.0``)
    * gobject-introspection
    * gtk-doc 1.14

History
-------

This library originates in `Tomboy`_ and has been widely reused without
having a separate release. This package has taken the python bindings
from the `Deskbar Applet`_ project, and broken it out to a standalone
project. The library was subsequently rewritten in major parts.

.. _Tomboy:            http://projects.gnome.org/tomboy/
.. _`Deskbar Applet`:  http://projects.gnome.org/deskbar-applet/

.. vim: ft=rst tw=72
