#!/usr/bin/env lua

-- example.lua
--
-- Created in 2010 by Ulrik Sverdrup <ulrik.sverdrup@gmail.com>
-- This work is placed in the public domain.

require "minigtk"
require "keybinder"

function callback(keystring, user_data)
  print("In callback", keystring, keybinder.get_current_event_time())
  print("Userdata: ", user_data)
  minigtk.main_quit()
end

minigtk.init()
keybinder.bind("<Control>A", callback, {1,2,3})
print("Press <Control>A to activate keybinding and quit");
minigtk.main()
