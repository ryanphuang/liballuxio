# libtachyon
C/C++ API for Tachyon

# Usage:

To build, change the platform-dependent settings in `Makefile` such as `OS_ARCH`
and `JAVA_HOME`, and then type `make`. You should be able to produce `libtachyon.so`
and `tachyontest`.

In your Tachyon client C/C++ code, include the `Tachyon.h` header to use the available
APIs. Then link `libtachyon.so` to your object files to compile an executable.

To run the client executable, you need to first make sure the `libjvm.so` is in the
`LD_LIBRARY_PATH`: e.g., `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server`.
Then make sure the Tachyon Java client jar is in the `CLASSPATH`: e.g., 
`export CLASSPATH=$CLASSPATH:$HOME/tachyon/lib/tachyon-client-0.6.4-jar-with-dependencies.jar`.

`tachyontest` is a sample client executable that test the implemented C/C++ Tachyon APIs.

For an actual usage example of the C++ client, refer to the [load generator](https://github.com/stormspirit/imembench/blob/master/loadgenerator/tachyondriver.cc) we implemented in the [imembench](https://github.com/stormspirit/imembench) project based on this client.



