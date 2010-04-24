#!/usr/bin/env lua

-- example.lua
--
-- Created in 2010 by Ulrik Sverdrup <ulrik.sverdrup@gmail.com>
-- This work is placed in the public domain.

require "minigtk"
require "keybinder"

function callback(keystring)
  print("In callback", keystring, keybinder.get_current_event_time())
  minigtk.main_quit()
end

minigtk.init()
keybinder.bind("<Control>A", callback)
print("Press <Control>A to activate keybinding and quit");
minigtk.main()
