/* bind.c
 * Copyright (C) 2008 Alex Graveley
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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "bind.h"

/* Uncomment the next line to print a debug trace. */
/* #define DEBUG */

#ifdef DEBUG
#  define TRACE(x) x
#else
#  define TRACE(x) do {} while (FALSE);
#endif

#define MODIFIERS_ERROR -1
#define MODIFIERS_NONE 0

/* Group to use: Which of configured keyboard Layouts
 * Since grabbing a key blocks its use, we can't grab the corresponding
 * (physical) keys for alternative layouts.
 * 
 * Because of this, we interpret all keys relative to the default
 * keyboard layout.
 *
 * For example, if you bind "w", the physical W key will respond to
 * the bound key, even if you switch to a keyboard layout where the W key
 * types a different letter.
 */
#define WE_ONLY_USE_ONE_GROUP 0


typedef struct _Binding {
	BindkeyHandler  handler;
	gpointer              user_data;
	char                 *keystring;
	/* GDK "distilled" values */
	guint                 keyval;
	GdkModifierType       modifiers;
} Binding;

static GSList *bindings = NULL;
static guint32 last_event_time = 0;
static gboolean processing_event = FALSE;

static int
FinallyGetModifiersForKeycode(register XkbDescPtr     xkb,
			      KeyCode                 key,
			      uint                    group,
			      uint                    level)
{
	XkbKeyTypeRec *type;
	int col, nKeyGroups;
	unsigned effectiveGroup;
	KeySym *syms;
	int k;
	uint state = 0;
	state &= ~(1 << 13 | 1 << 14);
	state |= group << 13;

	nKeyGroups= XkbKeyNumGroups(xkb,key);
	if ((!XkbKeycodeInRange(xkb,key))||(nKeyGroups==0)) {
		return MODIFIERS_ERROR;
	}

	syms = XkbKeySymsPtr(xkb,key);

	/* Taken from GDK's MyEnhancedXkbTranslateKeyCode */
	/* find the offset of the effective group */
	col = 0;
	effectiveGroup= XkbGroupForCoreState(state);
	if ( effectiveGroup>=nKeyGroups ) {
		unsigned groupInfo= XkbKeyGroupInfo(xkb,key);
		switch (XkbOutOfRangeGroupAction(groupInfo)) {
			default:
				effectiveGroup %= nKeyGroups;
				break;
			case XkbClampIntoRange:
				effectiveGroup = nKeyGroups-1;
				break;
			case XkbRedirectIntoRange:
				effectiveGroup = XkbOutOfRangeGroupNumber(groupInfo);
				if (effectiveGroup>=nKeyGroups)
					effectiveGroup= 0;
				break;
		}
	}
	col= effectiveGroup*XkbKeyGroupsWidth(xkb,key);
	type = XkbKeyKeyType(xkb,key,effectiveGroup);
	for (k = 0; k < type->map_count; k++) {
		if (type->map[k].active && type->map[k].level == level) {
			TRACE (g_print("%d, Mod mask: %x Real: %x Virt: %x\n", k, type->map[k].mods.mask, type->map[k].mods.real_mods, type->map[k].mods.vmods));
			if (type->preserve) {
				return type->map[k].mods.mask & ~type->preserve[k].mask;
			} else {
				return type->map[k].mods.mask;
			}
		}
	}
	return MODIFIERS_NONE;
}


static gboolean
grab_ungrab_with_ignorable_modifiers (GdkWindow *rootwin,
				      uint       keyval,
				      uint       modifiers,
				      gboolean   grab)
{
	/* Ignornable modifiers */
	guint mod_masks [] = {
		0, /* modifier only */
		GDK_MOD2_MASK,
		GDK_LOCK_MASK,
		GDK_MOD2_MASK | GDK_LOCK_MASK,
	};
	int i, k;
	GdkKeymapKey *keys;
	gint n_keys;
	GdkModifierType add_modifiers;
	Display *disp;
	XkbDescPtr xmap;
	gboolean success = FALSE;

	disp = GDK_WINDOW_XDISPLAY(rootwin);
	xmap = XkbGetMap(disp, XkbAllClientInfoMask, XkbUseCoreKbd);

	gdk_keymap_get_entries_for_keyval(NULL,
					  keyval,
					  &keys,
					  &n_keys);

	if (n_keys == 0)
		return FALSE;


	for (k = 0; k < n_keys; k++) {
		TRACE (g_print("Keycode %d  group %d level %d\n",
			keys[k].keycode, keys[k].group, keys[k].level));
		/* NOTE: We only bind for the first group,
		 * so regardless of current keyboard layout, it will
		 * grab the key from the default Layout.
		 */
		if (keys[k].group != WE_ONLY_USE_ONE_GROUP) {
			continue;
		}

		add_modifiers = FinallyGetModifiersForKeycode(xmap, 
					keys[k].keycode,
					keys[k].group,
					keys[k].level);
		TRACE (g_print("Grabbing keycode: %d, lev: %d, grp: %d\n",
			keys[k].keycode, keys[k].level, keys[k].group));
		TRACE (g_print("Modifiers: 0x%x\n",
                      add_modifiers | modifiers));
		TRACE (g_print("consuming modifiers %x\n",
			add_modifiers));
		for (i = 0; i < G_N_ELEMENTS (mod_masks); i++) {
			gdk_error_trap_push ();

			if (grab) {
				XGrabKey (GDK_WINDOW_XDISPLAY (rootwin), 
					  keys[k].keycode, 
					  modifiers | add_modifiers | mod_masks [i], 
					  GDK_WINDOW_XWINDOW (rootwin), 
					  False, 
					  GrabModeAsync,
					  GrabModeAsync);
			} else {
				XUngrabKey (GDK_WINDOW_XDISPLAY (rootwin),
					    keys[k].keycode,
					    modifiers | add_modifiers | mod_masks [i], 
					    GDK_WINDOW_XWINDOW (rootwin));
			}
			gdk_flush();
			if (gdk_error_trap_pop()) {
				TRACE (
				g_warning("Failed bind for %d,  i: %d\n",
					keys[k].keycode, i)
				);
			} else {
				success = TRUE;
				TRACE (
					g_print("Success bind  %d, i: %d\n",
						keys[k].keycode, i)
				);
			}
		}
	}
	g_free(keys);
	XkbFreeClientMap(xmap, 0, TRUE);

	return success;
}

static gboolean
keyvalues_equal(guint kv1, guint kv2)
{
	return kv1 == kv2;
}

/* Compare modifier set equality,
 * while accepting overloaded modifiers (MOD1 and META together)
 */
static gboolean
modifiers_equal(GdkModifierType mf1, GdkModifierType mf2)
{
	GdkModifierType ignored = 0;

	/* Accept MOD1 + META as MOD1 */
	if (mf1 & mf2 & GDK_MOD1_MASK) {
		ignored |= GDK_META_MASK;
	}
	/* Accept SUPER + HYPER as SUPER */
	if (mf1 & mf2 & GDK_SUPER_MASK) {
		ignored |= GDK_HYPER_MASK;
	}
	if ((mf1 & ~ignored) == (mf2 & ~ignored)) {
		return TRUE;
	}
        return FALSE;
}

static gboolean 
do_grab_key (Binding *binding)
{
	gboolean success;
	GdkKeymap *keymap = gdk_keymap_get_default ();
	GdkWindow *rootwin = gdk_get_default_root_window ();


	GdkModifierType modifiers;
	guint keysym = 0;

	if (keymap == NULL || rootwin == NULL)
		return FALSE;

	gtk_accelerator_parse(binding->keystring, &keysym, &modifiers);
	if (keysym == 0)
		return FALSE;

	TRACE (g_print ("Got accel %d, 0x%x\n", keysym, modifiers));

	binding->keyval = keysym;
	binding->modifiers = modifiers;

        /* Map virtual modifiers to non-virtual modifiers */
        gdk_keymap_map_virtual_modifiers(keymap, &modifiers);

	success = grab_ungrab_with_ignorable_modifiers (rootwin,
					keysym,
					modifiers,
					TRUE /* grab */);

	if (!success) {
	   g_warning ("Binding '%s' failed!\n", binding->keystring);
	}

	return success;
}

static gboolean 
do_ungrab_key (Binding *binding)
{
	GdkKeymap *keymap = gdk_keymap_get_default ();
	GdkWindow *rootwin = gdk_get_default_root_window ();
	GdkModifierType modifiers;

	TRACE (g_print ("Removing grab for '%s'\n", binding->keystring));

	if (keymap == NULL || rootwin == NULL)
		return FALSE;

	/* Map virtual modifiers to non-virtual modifiers */
	modifiers = binding->modifiers;
	gdk_keymap_map_virtual_modifiers(keymap, &modifiers);

	grab_ungrab_with_ignorable_modifiers (rootwin,
					      binding->keyval,
					      modifiers,
					      FALSE /* ungrab */);

	return TRUE;
}

static GdkFilterReturn
filter_func (GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
	GdkFilterReturn return_val = GDK_FILTER_CONTINUE;
	XEvent *xevent = (XEvent *) gdk_xevent;
	guint event_mods;
	GSList *iter;
	GdkModifierType consumed, modifiers;
	guint keyval;
	GdkKeymap *keymap = gdk_keymap_get_default();
	guint mod_mask = gtk_accelerator_get_default_mod_mask();
	GdkEventKey *keyevent;

	TRACE (g_print ("Got Event! %d, %d\n", xevent->type, event->type));

	keyevent = (GdkEventKey *)event;

	switch (xevent->type) {
	case KeyPress:
                modifiers = xevent->xkey.state;

		TRACE (g_print ("Got KeyPress keycode: %d, modifiers: 0x%x\n", 
				xevent->xkey.keycode, 
				xevent->xkey.state));

		gdk_keymap_translate_keyboard_state(
				keymap,
				xevent->xkey.keycode,
                                modifiers,
				/* See top comment why we don't use this here:
				   XkbGroupForCoreState (xevent->xkey.state)
				 */
				WE_ONLY_USE_ONE_GROUP,
				&keyval, NULL, NULL, &consumed);
                TRACE (g_print("Consumed: 0x%x\n", consumed));
                modifiers &= ~consumed;
                /* Map non-virtual to virtual modifiers */
                gdk_keymap_add_virtual_modifiers(keymap, &modifiers);

		/* 
		 * Set the last event time for use when showing
		 * windows to avoid anti-focus-stealing code.
		 */
		processing_event = TRUE;
		last_event_time = xevent->xkey.time;


		event_mods = modifiers & mod_mask;

		TRACE (g_print ("Translated keyval: %d, modifiers: 0x%x\n", 
				keyval, 
				event_mods));


		for (iter = bindings; iter != NULL; iter = iter->next) {
			Binding *binding = (Binding *) iter->data;

			if (keyvalues_equal(binding->keyval, keyval) &&
			    modifiers_equal(binding->modifiers, event_mods)) {
				TRACE (g_print ("Calling handler for '%s'...\n", 
						binding->keystring));

				(binding->handler) (binding->keystring, 
						    binding->user_data);
			}
		}

		processing_event = FALSE;
		break;
	case KeyRelease:
		TRACE (g_print ("Got KeyRelease! \n"));
		break;
	}

	return return_val;
}

static void 
keymap_changed (GdkKeymap *map)
{
	GSList *iter;

	TRACE (g_print ("Keymap changed! Regrabbing keys..."));

	for (iter = bindings; iter != NULL; iter = iter->next) {
		Binding *binding = (Binding *) iter->data;
		do_ungrab_key (binding);
	}

	for (iter = bindings; iter != NULL; iter = iter->next) {
		Binding *binding = (Binding *) iter->data;
		do_grab_key (binding);
	}
}

void 
keybinder_init (void)
{
	GdkKeymap *keymap = gdk_keymap_get_default ();
	GdkWindow *rootwin = gdk_get_default_root_window ();

	gdk_window_add_filter (rootwin, 
			       filter_func, 
			       NULL);

	g_signal_connect (keymap, 
			  "keys_changed",
			  G_CALLBACK (keymap_changed),
			  NULL);
}

gboolean
keybinder_bind (const char           *keystring,
		               BindkeyHandler  handler,
		       gpointer              user_data)
{
	Binding *binding;
	gboolean success;

	binding = g_new0 (Binding, 1);
	binding->keystring = g_strdup (keystring);
	binding->handler = handler;
	binding->user_data = user_data;

	/* Sets the binding's keycode and modifiers */
	success = do_grab_key (binding);

	if (success) {
		bindings = g_slist_prepend (bindings, binding);
	} else {
		g_free (binding->keystring);
		g_free (binding);
	}
	return success;
}

void
keybinder_unbind (const char           *keystring, 
			   BindkeyHandler  handler)
{
	GSList *iter;

	for (iter = bindings; iter != NULL; iter = iter->next) {
		Binding *binding = (Binding *) iter->data;

		if (strcmp (keystring, binding->keystring) != 0 ||
		    handler != binding->handler) 
			continue;

		do_ungrab_key (binding);

		bindings = g_slist_remove (bindings, binding);

		g_free (binding->keystring);
		g_free (binding);
		break;
	}
}

guint32
keybinder_get_current_event_time (void)
{
	if (processing_event) 
		return last_event_time;
	else
		return GDK_CURRENT_TIME;
}
