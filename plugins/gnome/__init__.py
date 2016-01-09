"""
A plugin providing support to Phosphorus to set the wallaper using GNOME Shell's backend.
GNOME Shell provides several configurable settings, all of which in due time should be
supported. At the moment, however, only a picture wallpaper can be set with this plugin.

Supported keys: picture-uri
Unsupported keys: primary-color, secondary-color, color-shading-type, picture-options
"""

import sys

import gi
gi.require_version('GObject', '2.0')
gi.require_version('Gio', '2.0')
gi.require_version('Phosphorus', '1.0')

from gi.repository import GObject, Gio, Phosphorus

class GnomeWallpaperSetter(GObject.Object, Phosphorus.Plugin):
    """
    GnomeWallpaperSetter is the class implementing PhPlugin. It therefore implements the
    do_set_background method (see below). Upon instantiation, a connection to GNOME Shell's
    "org.gnome.desktop.background" GSettings object is made. GNOME Shell automatically picks up on
    any changes made to this object, which we gladly make use of in do_set_background.
    """
    __gtype_name__ = 'GnomeWallpaperSetter'

    def __init__(self):
        GObject.Object.__init__(self)
        self._settings = Gio.Settings.new("org.gnome.desktop.background")

    def do_set_background(self, filepath: str):
        """
        This method changes values in the GSettings object, which GNOME Shell will pick up on
        automatically.

        Args:
            filepath: a string representation of the file path of the selected image.
        """
        # TODO: implement support for these in Phosphorus, and add widget to make these configurable
        #KEY_PRIMARY_COLOR "primary-color"
        #KEY_SECONDARY_COLOR "secondary-color"
        #KEY_COLOR_SHADING_TYPE "color-shading-type"
        #KEY_BACKGROUND_STYLE "picture-options"
        success = self._settings.set_string("picture-uri", filepath)
        if not success:
            # FIXME: make translatable
            print("Could not apply wallpaper", file=sys.stderr)

