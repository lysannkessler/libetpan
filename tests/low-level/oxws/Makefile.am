include $(top_srcdir)/rules.mk

AM_CFLAGS = -DLIBETPAN_TEST_MODE

exampledir=${datadir}/@PACKAGE@/tests/low-level
example_PROGRAMS = test_oxws

test_oxws_SOURCES = main.c \
  oxws.c autodiscover.c
test_oxws_CFLAGS = $(WERROR) \
  -I$(top_builddir)/include \
  $(CUNIT_CFLAGS) \
  $(AM_CFLAGS)
test_oxws_LDFLAGS = $(CUNIT_LIBS)
test_oxws_LDADD = $(top_srcdir)/src/libetpan.la \
  libtest_oxws_support.la

# test_support depends on libcurl and libxml
noinst_LTLIBRARIES = libtest_oxws_support.la
libtest_oxws_support_la_SOURCES = test_support.c
libtest_oxws_support_la_CFLAGS = $(WERROR) \
  -I$(top_builddir)/include \
  $(LIBCURL_CPPFLAGS) $(LIBXML2_CFLAGS) \
  $(AM_CFLAGS)
libtest_oxws_support_la_LDFLAGS = \
  $(LIBCURL) $(LIBXML2_LIBS)
