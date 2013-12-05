Phosphorus
=========

**A simple wallpaper browser and setter**

Phosphorus is a simple wallpaper browser and setter. Consider it a GTK3 clone of Nitrogen.
Currently, it has less features and it not yet released as stable. 
This means that you probably do not want to use this on a production system yet
(as far as setting a background there is important, but you know what I mean).

Installation
------------

There are two dependencies required to compile and run Phosphorus: GdkPixbuf and, of course, GTK3.
On Debian, these are `libgdk-pixbuf2.0-dev` and `libgtk-3-dev`.
On Arch Linux, these are `gdk-pixbuf2` and `gtk3`.

Once those dependencies are installed, just run these commands to build and install Phosphorus:

    $ ./autogen
    $ ./configure
    $ make
    # make install
    (Optionally) $ make clean

Translations
-----------

You can help translate Phosphorus in your language!
To do so, simply follow these steps:

	$ cd po
	$ intltool-update --pot
	$ mv phosphorus.pot <language code>.po

Where `<language code>` is, obviously, the code of your language (e.g. `nl` for Dutch, `fr` for French, `en` for English...)
Edit the `LINGUAS` file and add your language code. Please keep the list alphabetically.
Lastly, open the .po file you just generated and translate all the strings. Don't forget to fill in the information in the header!

When a translation needs updating, firstly create the `phosphorus.pot` file as explained above, then run the following: 

	$ intltool-update --dist --gettext-package=phosphorus --output-file=<language code>2.po <language code>

Then, make the necessary changes and overwrite the old .po file:

	$ mv <language code>2.po <language code>.po

ToDo
----

 * create/use existing cache for thumbs
 * support multi-monitors / Xinerama
 * support more cmdline parameters a là Nitrogen
 * reload all images upon removing/adding a directory?
 * support for inotify like Nitrogen?
 * support for sorting images like Nitrogen?
 * pass program version from autotools to code instead of defining it

Bugs
----

For any bug or request [fill an issue][bug] on [GitHub][ghp].

  [bug]: https://github.com/Unia/phosphorus/issues
  [ghp]: https://github.com/Unia/phosphorus

License
-------
**Phosphorus** is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

**Phosphorus** is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

**Copyright © 2013** Jente Hidskes <jthidskes@outlook.com>
