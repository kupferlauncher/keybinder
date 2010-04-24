/* lminigtk.c
 * Created in 2010 by Ulrik Sverdrup <ulrik.sverdrup@gmail.com>
 *
 * This work is placed in the public domain.
 */

#include <gtk/gtk.h>

#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

static int init (lua_State *L)
{
  int argc = 0;
  char **argv = NULL;
  lua_pushnumber(L, gtk_init_check(&argc, &argv));
  return 1;
}

static int main (lua_State *L)
{
  gtk_main();
  return 0;
}

static int main_quit (lua_State *L)
{
  gtk_main_quit();
  return 0;
}

static const luaL_reg minigtk[] = {
  {"init", init},
  {"main", main},
  {"main_quit", main_quit},
  {NULL, NULL}
};

LUALIB_API int luaopen_minigtk (lua_State *L)
{
  luaL_register(L, "minigtk", minigtk);
  return 1;
}

