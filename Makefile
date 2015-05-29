
OS_ARCH = amd64
PLATFORM = linux
JAVA_HOME = /usr/lib/jvm/java-7-openjdk-amd64

JAVA_INCLUDES := -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/$(PLATFORM)
JAVA_LDS := -L$(JAVA_HOME)/jre/lib/$(OS_ARCH)/server -ljvm

CC ?= gcc
CXX ?= g++

CFLAGS := -g -Wall -O0 $(JAVA_INCLUDES)
CXXFLAGS := -g -Wall -O0 $(JAVA_INCLUDES)
LDFLAGS := $(JAVA_LDS)

TARGETS := libtachyon.so tachyontest

all: $(TARGETS)

libtachyon.so: tachyon.c util.c jni_helper.c tachyon.h util.h jni_helper.h
	$(CXX) -o $@ -shared -fPIC $(CXXFLAGS) $<

tachyontest: tachyontest.o libtachyon.so
	$(CXX) $(LDFLAGS) $< -o $@

tachyontest.o: tachyontest.c tachyon.h
	$(CXX) $(CXXFLAGS) -c tachyontest.c

clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean
