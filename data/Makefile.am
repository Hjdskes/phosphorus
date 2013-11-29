SUBDIRS = icons

desktopdir = $(datadir)/applications
desktop_DATA = phosphorus.desktop

UPDATE_DESKTOP = update-desktop-database $(datadir)/applications || :

install-data-hook:
        $(UPDATE_DESKTOP)
uninstall-hook:
        $(UPDATE_DESKTOP)

EXTRA_DIST = icon-theme-installer
