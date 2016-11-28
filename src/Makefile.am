lib_LTLIBRARIES = liballuxio.la
liballuxio_la_SOURCES = Alluxio.cc JNIHelper.cc Util.cc Util.h Alluxio.h
liballuxio_la_CPPFLAGS = $(JNI_INCLUDES)
liballuxio_la_LIBADD = $(JNI_LDFLAGS)


include_liballuxiodir = $(includedir)
include_liballuxio_HEADERS = Alluxio.h JNIHelper.h Util.h

bin_PROGRAMS = alluxiotest

alluxiotest_SOURCES = AlluxioTest.cc Util.cc Util.h Alluxio.h
alluxiotest_CPPFLAGS = $(JNI_INCLUDES)
alluxiotest_LDADD = liballuxio.la -lpthread




