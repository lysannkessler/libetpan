#!/bin/sh


script_dir="`pwd`"
current_dir="$script_dir"

build_dir="$current_dir/build/libcurl-ios"
mac_dir="$script_dir/.."
final_dir="$mac_dir/libcurl-ios"

log_file="$current_dir/build/libcurl-build-`date +'%Y%m%d%H%M%S'`.log"

mkdir -p "$build_dir"
echo "Building libcurl" > $log_file

if [ -e $final_dir/lib/libcurl.a ]; then
    echo "already there."
    exit 0;
fi

cd $build_dir

sh $script_dir/build-libcurl-ios.sh 2>&1 >> $log_file

ERR_CODE=$?

if [ "$ERR_CODE" -ne 0 -o ! -d lib -o ! -d include ]; then
    echo "build failed, see $log_file"
    exit $EXIT
fi

mkdir -p "$final_dir"
ditto lib "$final_dir/lib"
ditto include "$final_dir/include"

cd $script_dir

exit 0
