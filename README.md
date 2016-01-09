Phosphorus
=========

**A simple wallpaper browser and setter, in the spirit of Nitrogen**

Phosphorus is a simple wallpaper browser and setter. Consider it a(n unofficial) GTK+ 3 port of Nitrogen.

Plugins
-------

Phosphorus, at its core, is agnostic to how the selected image is applied as wallpaper. There are
many different Desktop Environments that all have their own way of applying wallpapers. Soon, with
Wayland becoming stable, every lightweight compositor will have its own implementation as well. It
is therefore not viable for Phosphorus to implement all these different methods in its core.

The decision has been made to leverage plugins for these tasks. The idea is that every Desktop
Environment or every Wayland compositor has its own plugin to use its own backend. These plugins
can be written either in C or in Python 3 and can be installed in your own home directory to easily
extend Phosphorus' functionality. If you write a plugin that you think is usable by others as well,
please make a pull request so it can be reviewed for inclusion!

As an example (and to make Phosphorus somewhat usable by default), two plugins are provided: one to
set the wallpaper for GNOME Shell, the other to set the wallpaper using standard XLib for
standalone X window managers. To write your own plugin, you can look at these plugins as examples
or [read PhPlugins' documentation](https://github.com/Unia/phosphorus/blob/master/src/plugin/ph-plugin.c).
Don't forget that your plugin also needs a `.plugin` file describing it to libpeas!

Installation
------------

The dependencies required to compile and run Phosphorus, are: GTK+ 3, GDK-PixBuf and libpeas.

Once those dependencies are installed, just run these commands to build and install Phosphorus:

    $ ./autogen.sh
    $ make
    # make clean install

Translations
-----------

You can help translate Phosphorus to your language! To do so, simply follow these steps:

	$ cd po
	$ intltool-update --pot
	$ mv Phosphorus.pot <language code>.po

Where `<language code>` is the code of your language (e.g. `nl` for Dutch, `fr` for French, `en_GB` for British English...).
Edit the [LINGUAS](https://github.com/Unia/phosphorus/blob/master/po/LINGUAS) file and add your language code. Please keep the list sorted alphabetically.
Lastly, open the `.po` file you just generated and translate all the strings. Don't forget to fill in the information in the header!

When a translation needs updating, execute the following commands:

	$ cd po
	$ intltool-update --pot
	$ intltool-update --dist --gettext-package=Phosphorus --output-file=<language code>_new.po <language code>

Then make the necessary changes and overwrite the old `.po` file:

	$ mv <language code>_new.po <language code>.po

Bugs
----

For any bug or request [fill an issue](https://github.com/Unia/phosphorus/issues) on [GitHub][github].

License
-------

Please see [LICENSE](https://github.com/Unia/phosphorus/blob/master/LICENSE) on [GitHub][github].

**Copyright © 2013 - 2016** Jente Hidskes &lt;hjdskes@gmail.com&gt;

  [github]: https://github.com/Unia/phosphorus

