lib_LTLIBRARIES = libtachyon.la
libtachyon_la_SOURCES = Tachyon.cc JNIHelper.cc Util.cc Util.h Tachyon.h
libtachyon_la_CPPFLAGS = $(JNI_INCLUDES)
libtachyon_la_LIBADD = $(JNI_LDFLAGS)

include_libtachyondir = $(includedir)
include_libtachyon_HEADERS = Tachyon.h JNIHelper.h Util.h

bin_PROGRAMS = tachyontest

tachyontest_SOURCES = TachyonTest.cc Util.cc Util.h Tachyon.h
tachyontest_CPPFLAGS = $(JNI_INCLUDES)
tachyontest_LDADD = libtachyon.la
