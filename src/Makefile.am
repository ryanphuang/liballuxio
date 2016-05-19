lib_LTLIBRARIES = libtachyon.la liballuxio.la
libtachyon_la_SOURCES = Tachyon.cc JNIHelper.cc Util.cc Util.h Tachyon.h
libtachyon_la_CPPFLAGS = $(JNI_INCLUDES)
libtachyon_la_LIBADD = $(JNI_LDFLAGS)
liballuxio_la_SOURCES = Alluxio.cc JNIHelper.cc Util.cc Util.h Alluxio.h
liballuxio_la_CPPFLAGS = $(JNI_INCLUDES)
liballuxio_la_LIBADD = $(JNI_LDFLAGS)


include_libtachyondir = $(includedir)
include_libtachyon_HEADERS = Tachyon.h JNIHelper.h Util.h
include_liballuxiodir = $(includedir)
include_liballuxio_HEADERS = Alluxio.h JNIHelper.h Util.h

bin_PROGRAMS = tachyontest alluxiotest

tachyontest_SOURCES = TachyonTest.cc Util.cc Util.h Tachyon.h
tachyontest_CPPFLAGS = $(JNI_INCLUDES)
tachyontest_LDADD = libtachyon.la
alluxiotest_SOURCES = AlluxioTest.cc Util.cc Util.h Alluxio.h
alluxiotest_CPPFLAGS = $(JNI_INCLUDES)
alluxiotest_LDADD = liballuxio.la




