# Makefile for libprocess. 

ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = foreign

SUBDIRS = include

lib_LTLIBRARIES = libprocess.la

libprocess_la_SOURCES =		\
  src/config.hpp		\
  src/decoder.hpp		\
  src/encoder.hpp		\
  src/gate.hpp			\
  src/latch.cpp			\
  src/pid.cpp			\
  src/process.cpp		\
  src/statistics.cpp		\
  src/synchronized.hpp

libprocess_la_CPPFLAGS =		\
  -I$(srcdir)/include			\
  -I/usr/include/libev			\
  -I$(srcdir)/$(STOUT)/include		\
  -I$(BOOST)				\
  -I$(GLOG)/src				\
  -I$(LIBEV)				\
  -I$(RY_HTTP_PARSER)			\
  $(AM_CPPFLAGS)

 libprocess_la_LIBADD =			\
  -lglog				\
  -lhttp_parser				\
  -lev

 libprocess_la_LDFLAGS = -version-info 0:0:0
 
 pkgconfigdir = $(libdir)/pkgconfig

 pkgconfig_DATA = libprocess.pc
  
if HAS_GPERFTOOLS
libprocess_la_CPPFLAGS += -I$(GPERFTOOLS)/src
libprocess_la_LIBADD += $(GPERFTOOLS)/libprofiler.la
endif

# Tests.
check_PROGRAMS = tests

tests_SOURCES =							\
  src/tests/decoder_tests.cpp					\
  src/tests/encoder_tests.cpp					\
  src/tests/http_tests.cpp					\
  src/tests/io_tests.cpp					\
  src/tests/main.cpp						\
  src/tests/process_tests.cpp					\
  src/tests/statistics_tests.cpp				\
  src/tests/time_tests.cpp 					\
  /usr/src/gmock/gmock-all.cc

tests_CPPFLAGS =			\
  -I$(top_srcdir)/src			\
  -I/usr/src/gmock			\
  -I$(GTEST)/include			\
  -I$(GMOCK)/include			\
  $(libprocess_la_CPPFLAGS)

tests_LDADD =				\
  -lpthread				\
  -lprocess				\
  -lglog				\
  -lhttp_parser				\
  -lev

TESTS = tests

# TODO(benh): Fix shared builds (tests need libglog, libev, etc).

# Using LT_OUTPUT in configure.ac creates config.lt that doesn't get
# cleaned up by distclean-libtool. See this bug patch (which doesn't
# appear to be in all versions of libtool.m4):
# http://lists.gnu.org/archive/html/automake-commit/2008-11/msg00015.html.
distclean-local:
	-rm -f config.lt
