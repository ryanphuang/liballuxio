# liballuxio

C/C++ API for Alluxio (formerly Tachyon)

# Requirement:

  * JDK 1.7+
  * Autoconf, Automake, and Libtool (for maintainer)
    - On Ubuntu, `sudo apt-get install autoconf automake libtool`.
    - On Mac OS X, use package manager like `brew` (e.g., `brew install autoconf automake libtool`)

# Compatibility:

The library currently supports Alluxio 1.2.0.

# Compilation:

If you are a maintainer, you need to first invoke the `boostrap` script to generate the necessary
`configure` script.

Then invoke `configure` following the GNU conventions, which does some basic testing,
tries to setup the JNI CFLAGS and LDFLAGS automatically, and then generate a `Makefile`.

It is recommended that you build the project in a separate directory. For example,

```
cd liballuxio
./boostrap  # if you are a maintainer
mkdir build
mkdir bin
cd build
../configure --prefix=$(dirname $(pwd))/bin
make
make install
```

Under `bin/lib` you should see both a static library `liballuxio.a` and a dynamic
library `liballuxio.so` generated. The header file is in `bin/include`.

In your Alluxio client C/C++ code, include the `Alluxio.h` header to use the available
APIs. Then link the liballuxio library to your object files to compile an executable.

# Usage:

To run the client executable, make sure that:

1. `libjvm.so` is in the `LD_LIBRARY_PATH`
2. Alluxio Java client jar is in the `CLASSPATH` or modify the CLASSPATH constants at the top of src/JNIHelper.h.

Example settings of the environment variables are:

1. `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server`.
2. `export CLASSPATH=$CLASSPATH:$HOME/alluxio/core/client/target/alluxio-core-client-1.2.0-jar-with-dependencies.jar`.

**Lazy way**: `source env.sh`. The `env.sh` is a handy example script that you can invoke 
before running the client executable. Modify the script (especially `clientjarpath`) 
if necessary.

# Sample Client:
`alluxiotest` is a sample client executable that test the implemented C/C++ Alluxio APIs.

Example run: `src/alluxiotest localhost 19998`

If successful, the client will output:

```
$ src/alluxiotest localhost 19998

log4j:WARN No appenders could be found for logger (alluxio.logger.type).
log4j:WARN Please initialize the log4j system properly.
log4j:WARN See http://logging.apache.org/log4j/1.2/faq.html#noconfig for more info.

TEST - CREATE FILE: SUCCESS - Created alluxio file: /hello.txt

TEST - DELETE FILE: SUCCESS - Deleted path /hello.txt

TEST - CREATE FILE WITH OPTIONS: SUCCESS - Created alluxio file: /hello.txt

TEST - WRITE FILE: SUCCESS - Wrote "hello, alluxio!!" to file

TEST - OPEN FILE: SUCCESS - Opened file: /hello.txt

TEST - READ FILE: SUCCESS - Content of the created file:hello, alluxio!!

TEST - LS COMMAND: SUCCESS - ls command found the path /hello.txt

TEST - DELETE FILE: SUCCESS - Deleted path /hello.txt

TEST - CREATE DIRECTORY: SUCCESS - Created alluxio dir /alluxiotest

TEST - DIRECTORY EXISTS: SUCCESS - Alluxio dir /alluxiotest exists

TEST - LS COMMAND: SUCCESS - ls command found the path /alluxiotest

TEST - DELETE FILE: SUCCESS - Deleted path /alluxiotest
```
