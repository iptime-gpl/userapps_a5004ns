ACLOCAL_AMFLAGS = -I shave

SUBDIRS = libbb libopkg src tests utils man


HOST_CPU=@host_cpu@
BUILD_CPU=@build_cpu@
OPKGLIBDIR=@opkglibdir@ 
ALL_CFLAGS=-g -O -Wall -DHOST_CPU_STR=\"@host_cpu@\" -DBUILD_CPU=@build_cpu@ -DLIBDIR=\"@libdir@\" -DOPKGLIBDIR=\"@opkglibdir@\" -DDATADIR=\"@datadir@\"

PATHFINDER_CFLAGS = @PATHFINDER_CFLAGS@
PATHFINDER_LIBS   = @PATHFINDER_LIBS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libopkg.pc



interceptdir = $(datadir)/opkg/intercept
intercept_DATA = intercept/ldconfig intercept/depmod intercept/update-modules

install-data-hook:
	chmod +x $(DESTDIR)$(datadir)/opkg/intercept/*

EXTRA_DIST = $(intercept_DATA) tests/regress CONTRIBUTORS ChangeLog.ipkg

MAINTAINERCLEANFILES= \
	configure \
	Makefile.in \
	config.guess \
	config.sub \
	ltmain.sh \
	.Makefile.am.swp \
	aclocal.m4

package: all-recursive
	STRIPPROG=$(STRIP) INSTALL=$$PWD/install-sh  binary-arch

# Ensure libopkg/config.h doesn't slip into a distribution archive.
# TODO: Fix headers and remove config.h from libopkg_include_HEADERS so that
# this hack isn't needed!
opkg-distdir: distdir
	rm -f $(distdir)/libopkg/config.h

opkg-dist: opkg-distdir
	tar c $(distdir) | gzip -9 > $(distdir).tar.gz
