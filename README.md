Phosphorus
=========

**A simple wallpaper browser and setter, in the spirit of Nitrogen**

Phosphorus is a simple wallpaper browser and setter. Consider it a(n unofficial) GTK3 port of Nitrogen.

Installation
------------

There are two dependencies required to compile and run Phosphorus: GdkPixbuf and GTK3.

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

**Copyright © 2013 - 2015** Jente Hidskes &lt;hjdskes@gmail.com&gt;

  [github]: https://github.com/Unia/phosphorus

