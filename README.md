# liballuxio
C/C++ API for Alluxio (formerly Tachyon)

# Requirement:
  * JDK 1.7+
  * Autoconf, Automake, and Libtool (for maintainer)
    - On Ubuntu, `sudo apt-get install autoconf automake libtool`.

# Usage:

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

To run the client executable, you need to first make sure the `libjvm.so` is in the
`LD_LIBRARY_PATH`: e.g., `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server`.
Then make sure the Alluxio Java client jar is in the `CLASSPATH`: e.g.,

`export CLASSPATH=$CLASSPATH:$HOME/alluxio/core/client/target/alluxio-core-client-1.0.1-jar-with-dependencies.jar`.

`alluxiotest` is a sample client executable that test the implemented C/C++ Alluxio APIs.

For an actual usage example of the C++ client, refer to the
[load generator](https://github.com/stormspirit/imembench/blob/master/loadgenerator/tachyondriver.cc) we
implemented in the [imembench](https://github.com/stormspirit/imembench) project based on this client.
