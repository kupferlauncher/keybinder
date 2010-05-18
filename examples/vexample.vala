/* vexample.vala
 * Created in 2010 by Ulrik Sverdrup <ulrik.sverdrup@gmail.com>
 *
 * This work is placed in the public domain.
 */

const string EXAMPLE_KEY = "<Ctrl>A";

public void callback (string keystring, void *udata) {
	string s = (string) udata;
	print("Handling key %s. User data: %p, %s\n", keystring, udata, s);
	print("Event time: %u\n", Keybinder.get_current_event_time());
	Keybinder.unbind(keystring, callback);
	Gtk.main_quit();
}

int main(string[] args) {
	Gtk.init(ref args);
	Keybinder.init();
	Keybinder.bind(EXAMPLE_KEY, callback, "user data");
	print("Press %s to activate keybinding and quit\n", EXAMPLE_KEY);
	Gtk.main();
	return 0;
}
