#!/bin/sh

if test ! -d libetpan.xcodeproj ; then
  echo "You must run this script from libetpan/build-mac" 2>&1
  exit 1
fi

logfile="`pwd`/update.log"

cd ..

echo "running autogen.sh"
./autogen.sh >> "$logfile" 2>&1
if [[ "$?" != "0" ]]; then
  echo "autogen.sh failed" 2>&1
  exit 1
fi

echo "running configure"
./configure  "$@" >> "$logfile" 2>&1
if [[ "$?" != "0" ]]; then
  echo "configure failed" 2>&1
  exit 1
fi

# build config files
make stamp-prepare-target >> "$logfile" 2>&1
make libetpan-config.h >> "$logfile" 2>&1
cd build-mac
mkdir -p include/libetpan >> "$logfile" 2>&1
cp -r ../include/libetpan/ include/libetpan/
cp ../config.h include
cp ../libetpan-config.h include

# build dependencies
sh ./prepare-ios.sh
sh ./prepare-macosx.sh
