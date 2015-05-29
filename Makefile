
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

OBJS := Tachyon.o Util.o JNIHelper.o
DEPS := $(OBJS:.o=.d)

all: $(TARGETS)

libtachyon.so: $(OBJS)
	$(CXX) -o $@ -shared -fPIC $(CXXFLAGS) $^

-include $(DEPS)

%.o :%.cc
	$(CXX) -MMD -c -fPIC $(CXXFLAGS) $< -o $@

tachyontest: TachyonTest.o libtachyon.so
	$(CXX) $(LDFLAGS) $< -o $@

TachyonTest.o: TachyonTest.cc Tachyon.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o *.d $(TARGETS)

.PHONY: all clean
