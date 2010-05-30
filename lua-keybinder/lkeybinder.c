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

/* Callback closure with upvalues:
 * 1  Lua callback function
 * 2  Keystring
 * 3  User data
 */
static int lkeybinder_closure (lua_State *L)
{
  /* Call callback f(keystring, user_data) */
  lua_pushvalue(L, lua_upvalueindex(1));
  lua_pushvalue(L, lua_upvalueindex(2));
  lua_pushvalue(L, lua_upvalueindex(3));
  lua_call(L, 2, 0);
  return 0;
}

static void lkeybinder_handler (const char *keystring, void *user_data) {
  lua_State *L = user_data;
  /* get table */
  lua_rawgeti(L, LUA_REGISTRYINDEX, lkeybinder_reg_key);
  lua_pushstring(L, keystring);
  lua_rawget(L, -2);
  /* call callback closure */
  if (lua_pcall(L, 0, 0, 0)) {
    fprintf(stderr, "Error in callback:\n\t%s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

/* lkeybinder library function 'bind' */
static int lkeybinder_bind (lua_State *L)
{
  int success;
  int table_index;
  const char *keystr = luaL_checkstring(L, 1);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  /* argument 3 is nil/none or user data */
  lua_settop(L, 3);

  lkeybinder_check_init();
  /* get RefTable */
  lua_rawgeti(L, LUA_REGISTRYINDEX, lkeybinder_reg_key);
  table_index = lua_gettop(L);

  lua_pushstring(L, keystr);
  lua_rawget(L, table_index);
  if (!lua_isnil(L, -1)) {
    success = false;
  } else {
    success = keybinder_bind(keystr, lkeybinder_handler, L);
    if (success) {
      lua_pushstring(L, keystr);
      /* Create a closure with three upvalues
       * RefTable[keystr] = <closure>
       */
      lua_pushvalue(L, 2);
      lua_pushstring(L, keystr);
      lua_pushvalue(L, 3);
      lua_pushcclosure(L, lkeybinder_closure, 3);
      lua_rawset(L, table_index);
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
  /* RefTable[keystring] = nil */
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
