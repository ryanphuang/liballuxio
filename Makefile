
###-BEGIN: platform dependent settings-###

ifeq ($(OS), Windows_NT)
  $(error Windows is not supported)
else
  UNAME := $(shell uname -s)
  ifeq ($(UNAME), Linux)
    PLATFORM = linux
  else ifeq ($(UNAME), Darwin)
    PLATFORM = darwin 
  else
    $(error OS version not supported)
  endif
  ifeq ($(shell uname -m), x86_64)
    ifeq ($(PLATFORM), linux)
      ## Typical Linux Java settings
      OS_ARCH = amd64
      JVM_ARCH = 64
      JAVA_HOME = /usr/lib/jvm/java-7-openjdk-amd64
    else
      ## Typical OS X Java settings
      OS_ARCH = 
      JVM_ARCH = 64
      JAVA_HOME = $(shell /usr/libexec/java_home)
    endif
  else
    ifeq ($(PLATFORM), linux)
      ## Typical Linux Java settings
      OS_ARCH = i386 
      JVM_ARCH = 32
      JAVA_HOME = /usr/lib/jvm/java-7-openjdk-i386
    else
      ## Typical OS X Java settings
      OS_ARCH = 
      JVM_ARCH = 32
      JAVA_HOME = $(shell /usr/libexec/java_home)
    endif
  endif
endif

###-END: platform dependent settings-###

JAVA_INCLUDES := -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/$(PLATFORM)
JAVA_LDS := -L$(JAVA_HOME)/jre/lib/$(OS_ARCH)/server -ljvm

CC ?= gcc
CXX ?= g++
AR ?= ar

CFLAGS := -g -Wall -O0 $(JAVA_INCLUDES) -m$(JVM_ARCH) 
CXXFLAGS := -g -Wall -O0 $(JAVA_INCLUDES) -m$(JVM_ARCH) 
LDFLAGS := $(JAVA_LDS)

LIB_NAME := libtachyon
DYLIB_NAME := $(LIB_NAME).so
SLIB_NAME := $(LIB_NAME).a
TEST_NAME := tachyontest

DYLIB_FLAGS := -shared -fPIC 
SLIB_FLAGS := rcs

TARGETS := $(DYLIB_NAME) $(SLIB_NAME) $(TEST_NAME)

OBJS := Tachyon.o Util.o JNIHelper.o
TEST_OBJS := TachyonTest.o
DEPS := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

all: $(TARGETS)

$(DYLIB_NAME): $(OBJS)
	$(CXX) -o $@ $(DYLIB_FLAGS) $(CXXFLAGS) $(JAVA_LDS) $^

$(SLIB_NAME): $(OBJS)
	$(AR) $(SLIB_FLAGS) $@ $^

-include $(DEPS)

%.o :%.cc
	$(CXX) -MMD -c -fPIC $(CXXFLAGS) $< -o $@

$(TEST_NAME): $(TEST_OBJS) $(DYLIB_NAME)
	$(CXX) $^ -o $@ $(LDFLAGS) 

clean:
	rm -f *.o *.d $(TARGETS)

.PHONY: all clean
