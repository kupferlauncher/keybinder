#!/usr/bin/env lua

-- example_gi.lua
--
-- Created in 2011 by Ulrik Sverdrup <ulrik.sverdrup@gmail.com>
-- This work is placed in the public domain.
--
-- requires lgi  http://gitorious.org/lgi

local lgi = require 'lgi'
local Gtk = lgi.require('Gtk', '2.0')
local Keybinder = lgi.require('Keybinder', '0.0')

local function callback(keystring)
  print("In callback for", keystring)
  print("Event time:", Keybinder.get_current_event_time())
  Keybinder.unbind(keystring)
  Gtk.main_quit()
end

Gtk.init()
Keybinder.init()
Keybinder.bind("<Control>A", callback)
print("Press <Control>A to activate keybinding and quit");
Gtk.main()
