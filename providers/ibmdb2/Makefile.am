providerdir=$(libdir)/libgda-$(GDA_ABI_MAJOR_VERSION).$(GDA_ABI_MINOR_VERSION)/providers
provider_LTLIBRARIES = libgda-ibmdb2.la

AM_CPPFLAGS = \
	$(IBMDB2_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgda \
	-I$(top_builddir) \
	$(LIBGDA_CFLAGS)

libgda_ibmdb2_la_SOURCES = \
	gda-ibmdb2-provider.c \
	gda-ibmdb2-provider.h \
	gda-ibmdb2-recordset.c \
	gda-ibmdb2-recordset.h \
	gda-ibmdb2-types.c \
	gda-ibmdb2-types.h \
	gda-ibmdb2.h \
	libmain.c \
	utils.c

libgda_ibmdb2_la_LDFLAGS = -export-dynamic -module -avoid-version $(NO_UNDEFINED)
libgda_ibmdb2_la_LIBADD = \
	$(top_builddir)/libgda/libgda-4.0.la \
	$(LIBGDA_LIBS) \
	$(IBMDB2_LIBS)


xmldir   = $(datadir)/libgda-4.0
xml_in_files = ibmdb2_specs_dsn.xml.in

@INTLTOOL_XML_RULE@

xml_DATA = $(xml_in_files:.xml.in=.xml)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgda-ibmdb2-4.0.pc

EXTRA_DIST = $(xml_in_files) libgda-ibmdb2-4.0.pc.in
DISTCLEANFILES = $(xml_DATA)


