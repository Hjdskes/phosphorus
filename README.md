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

    $ autoreconf -i
    $ ./configure
    $ make
    # make install
    (Optionally) $ make clean

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
