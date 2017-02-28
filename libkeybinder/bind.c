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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "keybinder.h"

/* Uncomment the next line to print a debug trace. */
/* #define DEBUG */

#ifdef DEBUG
#  define TRACE(x) x
#else
#  define TRACE(x) do {} while (FALSE);
#endif

#define MODIFIERS_ERROR ((GdkModifierType)(-1))
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


struct Binding {
	KeybinderHandler      handler;
	void                 *user_data;
	char                 *keystring;
	GDestroyNotify        notify;
	/* GDK "distilled" values */
	guint                 keyval;
	GdkModifierType       modifiers;
};

static GSList *bindings = NULL;
static guint32 last_event_time = 0;
static gboolean processing_event = FALSE;
static gboolean detected_xkb_extension = FALSE;
static gboolean use_xkb_extension = FALSE;

/* Return the modifier mask that needs to be pressed to produce key in the
 * given group (keyboard layout) and level ("shift level").
 */
static GdkModifierType
FinallyGetModifiersForKeycode (XkbDescPtr xkb,
                               KeyCode    key,
                               guint      group,
                               guint      level)
{
	int nKeyGroups;
	int effectiveGroup;
	XkbKeyTypeRec *type;
	int k;

	nKeyGroups = XkbKeyNumGroups(xkb, key);
	if ((!XkbKeycodeInRange(xkb, key)) || (nKeyGroups == 0)) {
		return MODIFIERS_ERROR;
	}

	/* Taken from GDK's MyEnhancedXkbTranslateKeyCode */
	/* find the offset of the effective group */
	effectiveGroup = group;
	if (effectiveGroup >= nKeyGroups) {
		unsigned groupInfo = XkbKeyGroupInfo(xkb,key);
		switch (XkbOutOfRangeGroupAction(groupInfo)) {
			default:
				effectiveGroup %= nKeyGroups;
				break;
			case XkbClampIntoRange:
				effectiveGroup = nKeyGroups-1;
				break;
			case XkbRedirectIntoRange:
				effectiveGroup = XkbOutOfRangeGroupNumber(groupInfo);
				if (effectiveGroup >= nKeyGroups)
					effectiveGroup = 0;
				break;
		}
	}
	type = XkbKeyKeyType(xkb, key, effectiveGroup);
	for (k = 0; k < type->map_count; k++) {
		if (type->map[k].active && type->map[k].level == level) {
			if (type->preserve) {
				return (type->map[k].mods.mask &
				        ~type->preserve[k].mask);
			} else {
				return type->map[k].mods.mask;
			}
		}
	}
	return MODIFIERS_NONE;
}

/* Grab or ungrab the keycode+modifiers combination, first plainly, and then
 * including each ignorable modifier in turn.
 */
static gboolean
grab_ungrab_with_ignorable_modifiers (GdkWindow *rootwin,
                                      guint      keycode,
                                      guint      modifiers,
                                      gboolean   grab)
{
	guint i;
	gboolean success = FALSE;

	/* Ignorable modifiers */
	guint mod_masks [] = {
		0, /* modifier only */
		GDK_MOD2_MASK,
		GDK_LOCK_MASK,
		GDK_MOD2_MASK | GDK_LOCK_MASK,
	};

	gdk_error_trap_push ();

	for (i = 0; i < G_N_ELEMENTS (mod_masks); i++) {
		if (grab) {
			XGrabKey (GDK_WINDOW_XDISPLAY (rootwin),
			          keycode,
			          modifiers | mod_masks [i],
			          GDK_WINDOW_XID (rootwin),
			          False,
			          GrabModeAsync,
			          GrabModeAsync);
		} else {
			XUngrabKey (GDK_WINDOW_XDISPLAY (rootwin),
			            keycode,
			            modifiers | mod_masks [i],
			            GDK_WINDOW_XID (rootwin));
		}
	}
	gdk_flush();
	if (gdk_error_trap_pop()) {
		TRACE (g_print ("Failed grab/ungrab!\n"));
		if (grab) {
			/* On error, immediately release keys again */
			grab_ungrab_with_ignorable_modifiers(rootwin,
			                                     keycode,
			                                     modifiers,
			                                     FALSE);
		}
	} else {
		success = TRUE;
	}
	return success;
}

/* Grab or ungrab then keyval and modifiers combination, grabbing all key
 * combinations yielding the same key values.
 * Includes ignorable modifiers using grab_ungrab_with_ignorable_modifiers.
 */
static gboolean
grab_ungrab (GdkWindow *rootwin,
             guint      keyval,
             guint      modifiers,
             gboolean   grab)
{
	int k;
	GdkKeymap *map;
	GdkKeymapKey *keys;
	gint n_keys;
	GdkModifierType add_modifiers = 0;
	XkbDescPtr xmap = NULL;
	gboolean success = FALSE;

	if (!keybinder_supported()) {
		return FALSE;
	}

	if (use_xkb_extension) {
		xmap = XkbGetMap(GDK_WINDOW_XDISPLAY(rootwin),
		                 XkbAllClientInfoMask,
		                 XkbUseCoreKbd);
	}

	map = gdk_keymap_get_default();
	gdk_keymap_get_entries_for_keyval(map, keyval, &keys, &n_keys);

	if (n_keys == 0)
		return FALSE;

	for (k = 0; k < n_keys; k++) {
		/* NOTE: We only bind for the first group,
		 * so regardless of current keyboard layout, it will
		 * grab the key from the default Layout.
		 */
		if (keys[k].group != WE_ONLY_USE_ONE_GROUP) {
			continue;
		}


		TRACE (g_print("grab/ungrab keycode: %d, lev: %d, grp: %d, ",
			keys[k].keycode, keys[k].level, keys[k].group));
		if (use_xkb_extension) {
			add_modifiers = FinallyGetModifiersForKeycode(xmap,
		                                              keys[k].keycode,
		                                              keys[k].group,
		                                              keys[k].level);
		} else if (keys[k].level > 0) {
			/* skip shifted/modified keys in non-xkb mode
			 * this might mean the key can't be bound at all
			 */
			continue;
		}

		if (add_modifiers == MODIFIERS_ERROR) {
			continue;
		}
		TRACE (g_print("modifiers: 0x%x (consumed: 0x%x)\n",
		               add_modifiers | modifiers, add_modifiers));
		if (grab_ungrab_with_ignorable_modifiers(rootwin,
		                                         keys[k].keycode,
		                                         add_modifiers | modifiers,
		                                         grab)) {

			success = TRUE;
		} else {
			/* When grabbing, break on error */
			if (grab && !success) {
				break;
			}
		}

	}
	g_free(keys);
	if (xmap) {
		XkbFreeKeyboard(xmap, 0, TRUE);
	}

	return success;
}

static gboolean
keyvalues_equal (guint kv1, guint kv2)
{
	return kv1 == kv2;
}

/* Compare modifier set equality,
 * while accepting overloaded modifiers (MOD1 and META together)
 */
static gboolean
modifiers_equal (GdkModifierType mf1, GdkModifierType mf2)
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
do_grab_key (struct Binding *binding)
{
	gboolean success;
	GdkWindow *rootwin = gdk_get_default_root_window ();
	GdkKeymap *keymap = gdk_keymap_get_default ();


	GdkModifierType modifiers;
	guint keysym = 0;

	if (keymap == NULL || rootwin == NULL)
		return FALSE;

	gtk_accelerator_parse(binding->keystring, &keysym, &modifiers);

	if (keysym == 0)
		return FALSE;

	binding->keyval = keysym;
	binding->modifiers = modifiers;
	TRACE (g_print ("Grabbing keyval: %d, vmodifiers: 0x%x, name: %s\n",
	                keysym, modifiers, binding->keystring));

	/* Map virtual modifiers to non-virtual modifiers */
	gdk_keymap_map_virtual_modifiers(keymap, &modifiers);

	if (modifiers == binding->modifiers &&
	    (GDK_SUPER_MASK | GDK_HYPER_MASK | GDK_META_MASK) & modifiers) {
		g_warning ("Failed to map virtual modifiers");
		return FALSE;
	}

	success = grab_ungrab (rootwin, keysym, modifiers, TRUE /* grab */);

	if (!success) {
	   g_warning ("Binding '%s' failed!", binding->keystring);
	}

	return success;
}

static gboolean
do_ungrab_key (struct Binding *binding)
{
	GdkKeymap *keymap = gdk_keymap_get_default ();
	GdkWindow *rootwin = gdk_get_default_root_window ();
	GdkModifierType modifiers;

	if (keymap == NULL || rootwin == NULL)
		return FALSE;

	TRACE (g_print ("Ungrabbing keyval: %d, vmodifiers: 0x%x, name: %s\n",
	                binding->keyval, binding->modifiers, binding->keystring));

	/* Map virtual modifiers to non-virtual modifiers */
	modifiers = binding->modifiers;
	gdk_keymap_map_virtual_modifiers(keymap, &modifiers);

	grab_ungrab (rootwin, binding->keyval, modifiers, FALSE /* ungrab */);
	return TRUE;
}

static GdkFilterReturn
filter_func (GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
	XEvent *xevent = (XEvent *) gdk_xevent;
	GdkKeymap *keymap = gdk_keymap_get_default();
	guint keyval;
	GdkModifierType consumed, modifiers;
	guint mod_mask = gtk_accelerator_get_default_mod_mask();
	GSList *iter;

	(void) event;
	(void) data;

	switch (xevent->type) {
	case KeyPress:
		modifiers = xevent->xkey.state;

		TRACE (g_print ("Got KeyPress keycode: %d, modifiers: 0x%x\n", 
				xevent->xkey.keycode, 
				xevent->xkey.state));

		if (use_xkb_extension) {
			gdk_keymap_translate_keyboard_state(
				keymap,
				xevent->xkey.keycode,
				modifiers,
				/* See top comment why we don't use this here:
				   XkbGroupForCoreState (xevent->xkey.state)
				 */
				WE_ONLY_USE_ONE_GROUP,
				&keyval, NULL, NULL, &consumed);
		} else {
			consumed = 0;
			keyval = XLookupKeysym(&xevent->xkey, 0);
		}

		/* Map non-virtual to virtual modifiers */
		modifiers &= ~consumed;
		gdk_keymap_add_virtual_modifiers(keymap, &modifiers);
		modifiers &= mod_mask;

		TRACE (g_print ("Translated keyval: %d, vmodifiers: 0x%x, name: %s\n",
		                keyval, modifiers,
		                gtk_accelerator_name(keyval, modifiers)));

		/*
		 * Set the last event time for use when showing
		 * windows to avoid anti-focus-stealing code.
		 */
		processing_event = TRUE;
		last_event_time = xevent->xkey.time;

		iter = bindings;
		while (iter != NULL) {
			/* NOTE: ``iter`` might be removed from the list
			 * in the callback.
			 */
			struct Binding *binding = iter->data;
			iter = iter->next;

			if (keyvalues_equal(binding->keyval, keyval) &&
			    modifiers_equal(binding->modifiers, modifiers)) {
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

	return GDK_FILTER_CONTINUE;
}

static void
keymap_changed (GdkKeymap *map)
{
	GSList *iter;

	(void) map;

	TRACE (g_print ("Keymap changed! Regrabbing keys..."));

	for (iter = bindings; iter != NULL; iter = iter->next) {
		struct Binding *binding = iter->data;
		do_ungrab_key (binding);
	}

	for (iter = bindings; iter != NULL; iter = iter->next) {
		struct Binding *binding = iter->data;
		do_grab_key (binding);
	}
}

/**
 * keybinder_init:
 *
 * Initialize the keybinder library.
 *
 * This function must be called after initializing GTK, before calling any
 * other function in the library. Can only be called once.
 */
void
keybinder_init ()
{
	GdkKeymap *keymap = gdk_keymap_get_default ();
	GdkWindow *rootwin = gdk_get_default_root_window ();
	Display *disp;
	int xkb_opcode;
	int xkb_event_base;
	int xkb_error_base;
	int majver = XkbMajorVersion;
	int minver = XkbMinorVersion;

	if (!keybinder_supported()) {
		return;
	}

	if (!(disp = XOpenDisplay(NULL))) {
		g_warning("keybinder_init: Unable to open display");
		return;
	}

	detected_xkb_extension = XkbQueryExtension(disp,
	                                           &xkb_opcode,
	                                           &xkb_event_base,
	                                           &xkb_error_base,
	                                           &majver, &minver);

	use_xkb_extension = detected_xkb_extension;
	TRACE(g_print("XKB: %d, version: %d, %d\n", use_xkb_extension, majver, minver));

	gdk_window_add_filter (rootwin, filter_func, NULL);

	/* Workaround: Make sure modmap is up to date
	 * There is possibly a bug in GTK+ where virtual modifiers are not
	 * mapped because the modmap is not updated. The following function
	 * updates it.
	 */
	(void) gdk_keymap_have_bidi_layouts(keymap);


	g_signal_connect (keymap, 
			  "keys_changed",
			  G_CALLBACK (keymap_changed),
			  NULL);
}

/**
 * keybinder_set_use_cooked_accelerators:
 * @use_cooked: if %FALSE disable cooked accelerators
 *
 * "Cooked" accelerators use symbols produced by using modifiers such
 * as shift or altgr, for example if "!" is produced by "Shift+1".
 *
 * If cooked accelerators are enabled, use "&lt;Ctrl&gt;exclam" to bind
 * "Ctrl+!" If disabled, use "&lt;Ctrl&gt;&lt;Shift&gt;1" to bind
 * "Ctrl+Shift+1". These two examples are not equal on all keymaps.
 *
 * The cooked accelerator keyvalue and modifiers are provided by the
 * function gdk_keymap_translate_keyboard_state()
 *
 * Cooked accelerators are useful if you receive keystrokes from GTK to bind,
 * but raw accelerators can be useful if you or the user inputs accelerators as
 * text.
 *
 * Default: Enabled. Should be set before binding anything.
 */
void
keybinder_set_use_cooked_accelerators (gboolean use_cooked)
{
	use_xkb_extension = use_cooked && detected_xkb_extension;
}

/**
 * keybinder_bind: (skip)
 * @keystring: an accelerator description (gtk_accelerator_parse() format)
 * @handler:   callback function
 * @user_data: data to pass to @handler
 *
 * Grab a key combination globally and register a callback to be called each
 * time the key combination is pressed.
 *
 * This function is excluded from introspected bindings and is replaced by
 * keybinder_bind_full.
 *
 * Returns: %TRUE if the accelerator could be grabbed
 */
gboolean
keybinder_bind (const char *keystring,
                KeybinderHandler handler,
                void *user_data)
{
	return keybinder_bind_full(keystring, handler, user_data, NULL);
}

/**
 * keybinder_bind_full: (rename-to keybinder_bind)
 * @keystring: an accelerator description (gtk_accelerator_parse() format)
 * @handler:   (scope notified):        callback function
 * @user_data: (closure) (allow-none):  data to pass to @handler
 * @notify:    (allow-none):  called when @handler is unregistered
 *
 * Grab a key combination globally and register a callback to be called each
 * time the key combination is pressed.
 *
 * Since: 0.3.0
 *
 * Returns: %TRUE if the accelerator could be grabbed
 */
gboolean
keybinder_bind_full (const char *keystring,
                     KeybinderHandler handler,
                     void *user_data,
                     GDestroyNotify notify)
{
	struct Binding *binding;
	gboolean success;

	binding = g_new0 (struct Binding, 1);
	binding->keystring = g_strdup (keystring);
	binding->handler = handler;
	binding->user_data = user_data;
	binding->notify = notify;

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

/**
 * keybinder_unbind: (skip)
 * @keystring: an accelerator description (gtk_accelerator_parse() format)
 * @handler:   callback function
 *
 * Unregister a previously bound callback for this keystring.
 *
 * NOTE: multiple callbacks per keystring are not properly supported. You
 * might as well use keybinder_unbind_all().
 *
 * This function is excluded from introspected bindings and is replaced by
 * keybinder_unbind_all().
 */
void
keybinder_unbind (const char *keystring, KeybinderHandler handler)
{
	GSList *iter;

	for (iter = bindings; iter != NULL; iter = iter->next) {
		struct Binding *binding = iter->data;

		if (strcmp (keystring, binding->keystring) != 0 ||
		    handler != binding->handler) 
			continue;

		do_ungrab_key (binding);
		bindings = g_slist_remove (bindings, binding);

		TRACE (g_print("unbind, notify: %p\n", binding->notify));
		if (binding->notify) {
			binding->notify(binding->user_data);
		}
		g_free (binding->keystring);
		g_free (binding);
		break;
	}
}

/**
 * keybinder_unbind_all: (rename-to keybinder_unbind)
 * @keystring: an accelerator description (gtk_accelerator_parse() format)
 *
 * Unregister all previously bound callbacks for this keystring.
 *
 * Since: 0.3.0
 */
void keybinder_unbind_all (const char *keystring)
{
	GSList *iter = bindings;

	for (iter = bindings; iter != NULL; iter = iter->next) {
		struct Binding *binding = iter->data;

		if (strcmp (keystring, binding->keystring) != 0)
			continue;

		do_ungrab_key (binding);
		bindings = g_slist_remove (bindings, binding);

		TRACE (g_print("unbind_all, notify: %p\n", binding->notify));
		if (binding->notify) {
			binding->notify(binding->user_data);
		}
		g_free (binding->keystring);
		g_free (binding);

		/* re-start scan from head of new list */
		iter = bindings;
		if (!iter)
			break;
	}
}

/**
 * keybinder_get_current_event_time:
 *
 * Returns: the current event timestamp
 */
guint32
keybinder_get_current_event_time (void)
{
	if (processing_event)
		return last_event_time;
	else
		return GDK_CURRENT_TIME;
}

/**
 * keybinder_supported:
 *
 * Returns: TRUE if keybindings are supported
 */
gboolean
keybinder_supported (void)
{
	return GDK_IS_X11_DISPLAY(gdk_display_get_default());
}
