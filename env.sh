libjvmpaths=(
  "/usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server" # OpenJDK 1.7
  "/usr/lib/jvm/java-1.7.0-openjdk/jre/lib/amd64/server" # another OpenJDK 1.7
  "/usr/lib/jvm/jdk1.7.0_65/jre/lib/amd64/server" # Oracle JDK 1.7
  "/Library/Java/JavaVirtualMachines/jdk1.7.0_71.jdk/Contents/Home/jre/lib/server" # MacOS
)
#clientjarpath=$HOME/tachyon/lib/tachyon-client-0.6.4-jar-with-dependencies.jar
clientjarpath=$HOME/alluxio/core/client/target/alluxio-core-client-1.2.0-jar-with-dependencies.jar

found=0
for path in "${libjvmpaths[@]}"
do
  if [ -d $path ]; then
    found=1
    echo "found JVM path at $path, add to LD_LIBRARY_PATH"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$path
    break
  fi
done
if [ $found -eq 0 ]; then
  echo "cannot find JVM path, LD_LIBRARY_PATH not updated"
fi

if [ -f $clientjarpath ]; then
  echo "found alluxio client jar, add to CLASSPATH"
  export CLASSPATH=$CLASSPATH:$clientjarpath
else
  echo "cannot find alluxio client jar, CLASSPATH not updated"
fi

