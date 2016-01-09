SUBDIRS = data src plugins po

AM_CFLAGS = -std=c99 -Wall -Werror -Werror-implicit-function-declaration -Werror=format-security \
	-Wformat -Wformat-security -Wundef -Wstrict-prototypes -Wno-conversion -Wunreachable-code \
	-fno-common -fstack-protector

EXTRA_DIST = \
	AUTHORS \
	LICENSE \
	README.md

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	config.* \
	py-compile

-include $(top_srcdir)/git.mk

