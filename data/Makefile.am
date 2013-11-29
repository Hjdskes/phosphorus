SUBDIRS = icons

desktopdir = $(datadir)/applications

desktop_in_files = phosphorus.desktop.in
desktop_DATA = $(phosphorus:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

UPDATE_DESKTOP = update-desktop-database $(datadir)/applications || :

install-data-hook:
        $(UPDATE_DESKTOP)
uninstall-hook:
        $(UPDATE_DESKTOP)

EXTRA_DIST = icon-theme-installer
