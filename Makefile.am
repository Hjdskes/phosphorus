SUBDIRS = data src plugins po

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

-include $(top_srcdir)/common.mk
-include $(top_srcdir)/git.mk

