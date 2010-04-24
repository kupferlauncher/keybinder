/* lkeybinder.c
 * Copyright (C) 2010 Ulrik Sverdrup <ulrik.sverdrup@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdbool.h>

#include <keybinder.h>

#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

/* @lkeybinder_reg_key is a "ref" into the registry where we store
 * a mapping from keystring to callback function in a table.
 */
static int lkeybinder_reg_key;

static void lkeybinder_check_init () {
  static int did_init;
  if (!did_init) {
    keybinder_init();
    did_init = true;
  }
}

static void lkeybinder_handler (const char *keystring, void *user_data) {
  lua_State *L = user_data;
  /* get table */
  lua_rawgeti(L, LUA_REGISTRYINDEX, lkeybinder_reg_key);
  lua_pushstring(L, keystring);
  lua_rawget(L, -2);
  /* call callback with the keystring as first argument */
  lua_pushstring(L, keystring);
  if (lua_pcall(L, 1, 0, 0)) {
    fprintf(stderr, "Error in callback:\n\t%s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

/* lkeybinder library function 'bind' */
static int lkeybinder_bind (lua_State *L)
{
  int success;
  const char *keystr = luaL_checkstring(L, 1);
  if (!lua_isfunction(L, 2)) {
    return luaL_argerror(L, 2, "is not a function");
  }
  lkeybinder_check_init();
  lua_settop(L, 2);
  /* put table into stack pos 3 */
  lua_rawgeti(L, LUA_REGISTRYINDEX, lkeybinder_reg_key);
  lua_pushstring(L, keystr);
  lua_rawget(L, -2);
  if (!lua_isnil(L, -1)) {
    success = false;
  } else {
    success = keybinder_bind(keystr, lkeybinder_handler, L);
    if (success) {
      lua_pushstring(L, keystr);
      lua_pushvalue(L, 2);
      lua_rawset(L, 3);
    }
  }

  lua_pushboolean(L, success);
  return 1;
}

/* lkeybinder library function 'unbind' */
static int lkeybinder_unbind (lua_State *L)
{
  const char *keystr = luaL_checkstring(L, 1);
  keybinder_unbind(keystr, lkeybinder_handler);
  /* get table */
  lua_rawgeti(L, LUA_REGISTRYINDEX, lkeybinder_reg_key);
  lua_pushstring(L, keystr);
  lua_pushnil(L);
  lua_rawset(L, -3);
  return 0;
}

/* lkeybinder library function 'get_current_event_time' */
static int lkeybinder_current_event_time (lua_State *L)
{
  lua_pushnumber(L, keybinder_get_current_event_time());
  return 1;
}

static const luaL_reg keybinderlib[] = {
  {"bind", lkeybinder_bind},
  {"unbind", lkeybinder_unbind},
  {"get_current_event_time", lkeybinder_current_event_time},
  {NULL, NULL}
};

LUALIB_API int luaopen_keybinder (lua_State *L)
{
  luaL_register(L, "keybinder", keybinderlib);
  lua_newtable(L);
  lkeybinder_reg_key = luaL_ref(L, LUA_REGISTRYINDEX);
  return 1;
}
