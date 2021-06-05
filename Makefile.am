AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(includedir) -I$(ONLINE_MAIN)/include -I$(OFFLINE_MAIN)/include -I$(ROOTSYS)/include

lib_LTLIBRARIES = liblappdmon.la

liblappdmon_la_SOURCES = \
  pmon.cc \
  LAPPDMon.cc

nodist_liblappdmon_la_SOURCES = \
  pmonDict.C

include_HEADERS = \
  pmon.h

noinst_HEADERS = \
  LAPPDMon.h \
  pmonLinkDef.h

liblappdmon_la_LIBADD = \
  -L$(libdir) \
  -L$(ONLINE_MAIN)/lib \
  -L$(OFFLINE_MAIN)/lib \
  -lcaencalib \
  -lEvent

# Rule for generating CINT dictionaries from class headers.
pmonDict.C: pmon.h pmonLinkDef.h
	rootcint -f pmonDict.C -c $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $^

pcmdir = $(libdir)
nobase_dist_pcm_DATA = \
  pmonDict_rdict.pcm


CLEANFILES = pmonDict.C pmonDict.h

